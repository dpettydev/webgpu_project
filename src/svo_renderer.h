#ifndef DRAGO_AABB_RENDERER_H
#define DRAGO_AABB_RENDERER_H

#include "util/math_def.h"
#include "render/camera.h"
#include <time.h>

#include "extern/lz4/lz4.h"

#ifndef FLT_MAX
#define FLT_MAX __FLT_MAX__
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// drgAABB: Represents an axis-aligned bounding box
struct drgAABB
{
    drgVec3 min_val; // Minimum corner
    unsigned int pad1;
    drgVec3 max_val; // Maximum corner
    unsigned int pad2;
};

struct drgSVOBuilder
{
    drgAABB aabb;
    unsigned int index;
    unsigned int is_leaf;
    drgVec3 color;
    drgVec3 normal; // Added for precomputed normals
    drgSVOBuilder *children[8];
};

struct drgSVONode
{
    drgAABB aabb;
    drgVec3 color;
    float alpha;
    drgVec3 normal; // Added for precomputed normals
    unsigned int is_leaf;
    unsigned int child_idx;
    unsigned int has_child;
    unsigned int children[8];
};

struct drgHit
{
    bool hit;
    drgVec3 color;
    float t;
    drgVec3 hit_point; // Added for shading
    drgVec3 normal;    // Added for shading
};

drgVec3 aabb_center(drgAABB aabb)
{
    return (drgVec3){
        (aabb.min_val.x + aabb.max_val.x) * 0.5f,
        (aabb.min_val.y + aabb.max_val.y) * 0.5f,
        (aabb.min_val.z + aabb.max_val.z) * 0.5f};
}

drgMat44 drgMat44_Translation(drgVec3 t)
{
    drgMat44 m;
    m.m[0][3] = t.x;
    m.m[1][3] = t.y;
    m.m[2][3] = t.z;
    return m;
}

drgMat44 drgMat4_Multiply(drgMat44 m1, drgMat44 m2)
{
    drgMat44 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                result.m[i][j] += m1.m[i][k] * m2.m[k][j];
            }
        }
    }
    return result;
}

drgVec3 drgMat4_TransformVec3(drgMat44 m, drgVec3 v)
{
    float w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3];
    if (fabs(w) < 1e-6f)
        w = 1.0f;
    drgVec3 result = {
        (m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3]) / w,
        (m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3]) / w,
        (m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3]) / w};
    return result;
}

drgMat44 drgMat4_LookAt(drgVec3 eye, drgVec3 target, drgVec3 up)
{
    drgVec3 f = drgVec3::Normalize(drgVec3::Sub(target, eye));
    drgVec3 s = drgVec3::Normalize(drgVec3::Cross(f, up));
    drgVec3 u = drgVec3::Cross(s, f);

    drgMat44 m;
    m.m[0][0] = s.x;
    m.m[0][1] = s.y;
    m.m[0][2] = s.z;
    m.m[1][0] = u.x;
    m.m[1][1] = u.y;
    m.m[1][2] = u.z;
    m.m[2][0] = -f.x;
    m.m[2][1] = -f.y;
    m.m[2][2] = -f.z;
    m.m[0][3] = -drgVec3::Dot(s, eye);
    m.m[1][3] = -drgVec3::Dot(u, eye);
    m.m[2][3] = drgVec3::Dot(f, eye);
    return m;
}

drgMat44 drgMat4_Perspective(float fovy, float aspect, float _near, float _far)
{
    float f = 1.0f / tanf(fovy * 0.5f * 3.14159265358979323846f / 180.0f);
    float nf = _near - _far;

    drgMat44 m;
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (_far + _near) / nf;
    m.m[2][3] = (2.0f * _far * _near) / nf;
    m.m[3][2] = -1.0f;
    return m;
}

drgMat44 drgMat4_Inverse(drgMat44 m)
{
    drgMat44 inv;
    if (m.m[0][0] == 1 && m.m[1][1] == 1 && m.m[2][2] == 1 && m.m[3][3] == 1 &&
        m.m[0][1] == 0 && m.m[0][2] == 0 && m.m[1][0] == 0 && m.m[1][2] == 0 &&
        m.m[2][0] == 0 && m.m[2][1] == 0 && m.m[3][0] == 0 && m.m[3][1] == 0 && m.m[3][2] == 0)
    {
        inv.Identity();
        inv.m[0][3] = -m.m[0][3];
        inv.m[1][3] = -m.m[1][3];
        inv.m[2][3] = -m.m[2][3];
        return inv;
    }

    float det = 0;
    for (int i = 0; i < 4; i++)
    {
        det += m.m[0][i] * (m.m[1][(i + 1) % 4] * m.m[2][(i + 2) % 4] * m.m[3][(i + 3) % 4] -
                            m.m[1][(i + 2) % 4] * m.m[2][(i + 1) % 4] * m.m[3][(i + 3) % 4]);
    }
    if (fabs(det) < 1e-6f)
        det = 1.0f;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            float minor = 1.0f;
            for (int k = 0; k < 4; k++)
            {
                if (k != i)
                {
                    for (int l = 0; l < 4; l++)
                    {
                        if (l != j)
                        {
                            minor *= m.m[k][l];
                        }
                    }
                }
            }
            inv.m[j][i] = ((i + j) % 2 ? -1 : 1) * minor / det;
        }
    }
    return inv;
}

int clamp(int x, int min, int max)
{
    return x < min ? min : (x > max ? max : x);
}

float rnorm()
{
    return (float)rand() / (float)RAND_MAX;
}

// Random number generator (LCG)
static unsigned int rng_state = 1;
void rng_seed(unsigned int seed)
{
    rng_state = seed;
}
float rng_float()
{
    rng_state = 1664525 * rng_state + 1013904223;
    return (float)(rng_state >> 8) / (1U << 24); // [0, 1)
}

#define drgInt8 char
#define drgInt32 int
#define drgInt64 long long

#define drgUInt8 unsigned char
#define drgUInt32 unsigned int
#define drgUInt64 unsigned long long

#if 1

drgUInt32 _octree_total = 0;
drgUInt32 *_octree = NULL;

drgUInt32 _octree_test_total = 23;
drgUInt32 _octree_test[32] = {
    // Root node (unchanged)
    0x40f0f, // 4 children, offset to index 1

    // Child nodes (unchanged)
    0x10ff00, // Child 0: offset to leaves at index 5
    0x2c5500, // Child 1: offset to leaves at index 11
    0x383300, // Child 2: offset to leaves at index 14
    0x441100, // Child 3: offset to leaves at index 17

    // Leaf nodes for Child 0 (indices 5–10, randomized colors)
    0x00FF5733, // RGB: (255, 87, 51)
    0x00C700FF, // RGB: (199, 0, 255)
    0x0033FF99, // RGB: (51, 255, 153)
    0x00FF9900, // RGB: (255, 153, 0)
    0x0066CCFF, // RGB: (102, 204, 255)
    0x00AA00BB, // RGB: (170, 0, 187)

    // Leaf nodes for Child 1 (indices 11–13, randomized colors)
    0x0011EE55, // RGB: (17, 238, 85)
    0x00FF22DD, // RGB: (255, 34, 221)
    0x00887722, // RGB: (136, 119, 34)

    // Leaf nodes for Child 2 (indices 14–16, randomized colors)
    0x0044AAFF, // RGB: (68, 170, 255)
    0x00BB3366, // RGB: (187, 51, 102)
    0x0022FF88, // RGB: (34, 255, 136)

    // Leaf nodes for Child 3 (indices 17–22, randomized colors)
    0x00DD1155, // RGB: (221, 17, 85)
    0x0077CC33, // RGB: (119, 204, 51)
    0x00FF88BB, // RGB: (255, 136, 187)
    0x0033AA22, // RGB: (51, 170, 34)
    0x0099FF66, // RGB: (153, 255, 102)
    0x00EE4477  // RGB: (238, 68, 119)
};

#else

drgUInt32 _octree_total = 9;
drgUInt32 _octree[9] = {
    0x4ff00,
    0x0044AAFF, // RGB: (68, 170, 255)
    0x00BB3366, // RGB: (187, 51, 102)
    0x0022FF88, // RGB: (34, 255, 136)
    0x00FF5733, // RGB: (255, 87, 51)
    0x00C700FF, // RGB: (199, 0, 255)
    0x0033FF99, // RGB: (51, 255, 153)
    0x00FF9900, // RGB: (255, 153, 0)
    0x0066CCFF, // RGB: (102, 204, 255)
};

#endif

static const drgInt32 MaxScale = 23;

static const drgUInt32 BitCount[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

static inline float uintBitsToFloat(drgUInt32 i)
{
    union
    {
        drgUInt32 i;
        float f;
    } unionHack;
    unionHack.i = i;
    return unionHack.f;
}

static inline drgUInt32 floatBitsToUint(float f)
{
    union
    {
        drgUInt32 i;
        float f;
    } unionHack;
    unionHack.f = f;
    return unionHack.i;
}

inline void decompressMaterial(drgUInt32 normal, drgVec3 &dst, float &shade) 
{
    drgUInt32 sign = (normal & 0x80000000) >> 31;
    drgUInt32 face = (normal & 0x60000000) >> 29;
    drgUInt32 u    = (normal & 0x1FFC0000) >> 18;
    drgUInt32 v    = (normal & 0x0003FF80) >> 7;
    drgUInt32 c    = (normal & 0x0000007F);

    const int mod3[] = {0, 1, 2, 0, 1};
    dst.a[     face     ] = (sign ? -1.0f : 1.0f);
    dst.a[mod3[face + 1]] = u*4.8852e-4f*2.0f - 1.0f;
    dst.a[mod3[face + 2]] = v*4.8852e-4f*2.0f - 1.0f;

    dst.Normalize();
    shade = c*1.0f/127.0f;
}

bool svo_raymarch(const drgVec3 &rayOrigin, const drgVec3 &rayDirection, float rayScale, drgUInt32 &hitColor, float &hitDistance)
{
    struct StackEntry
    {
        drgUInt32 offset;
        float maxT;
    };
    StackEntry rayStack[MaxScale + 1];

    float ox = rayOrigin.x, oy = rayOrigin.y, oz = rayOrigin.z;
    float dx = rayDirection.x, dy = rayDirection.y, dz = rayDirection.z;

    if (fabs(dx) < 1e-4f)
        dx = 1e-4f;
    if (fabs(dy) < 1e-4f)
        dy = 1e-4f;
    if (fabs(dz) < 1e-4f)
        dz = 1e-4f;

    float dTx = 1.0f / -fabs(dx);
    float dTy = 1.0f / -fabs(dy);
    float dTz = 1.0f / -fabs(dz);

    float bTx = dTx * ox;
    float bTy = dTy * oy;
    float bTz = dTz * oz;

    drgUInt8 octantMask = 7;
    if (dx > 0.0f)
        octantMask ^= 1, bTx = 3.0f * dTx - bTx;
    if (dy > 0.0f)
        octantMask ^= 2, bTy = 3.0f * dTy - bTy;
    if (dz > 0.0f)
        octantMask ^= 4, bTz = 3.0f * dTz - bTz;

    float minT = max(2.0f * dTx - bTx, max(2.0f * dTy - bTy, 2.0f * dTz - bTz));
    float maxT = min(dTx - bTx, min(dTy - bTy, dTz - bTz));
    minT = max(minT, 0.0f);

    drgUInt32 current = 0;
    drgUInt32 parent = 0;
    int idx = 0;
    float posX = 1.0f;
    float posY = 1.0f;
    float posZ = 1.0f;
    int scale = MaxScale - 1;

    float scaleExp2 = 0.5f;

    if (1.5f * dTx - bTx > minT)
        idx ^= 1, posX = 1.5f;
    if (1.5f * dTy - bTy > minT)
        idx ^= 2, posY = 1.5f;
    if (1.5f * dTz - bTz > minT)
        idx ^= 4, posZ = 1.5f;

    while (scale < MaxScale)
    {
        if (current == 0)
            current = _octree[parent];

        float cornerTX = posX * dTx - bTx;
        float cornerTY = posY * dTy - bTy;
        float cornerTZ = posZ * dTz - bTz;
        float maxTC = min(cornerTX, min(cornerTY, cornerTZ));

        int childShift = idx ^ octantMask;
        drgUInt32 childMasks = current << childShift;

        if ((childMasks & 0x8000) && minT <= maxT)
        {
            if (maxTC * rayScale >= scaleExp2)
            {
                hitDistance = maxTC;
                return true;
            }

            float maxTV = min(maxT, maxTC);
            float half = scaleExp2 * 0.5f;
            float centerTX = half * dTx + cornerTX;
            float centerTY = half * dTy + cornerTY;
            float centerTZ = half * dTz + cornerTZ;

            if (minT <= maxTV)
            {
                drgUInt32 childOffset = current >> 18;
                if (current & 0x20000)
                {
                    childOffset = (childOffset << 32) | (_octree[parent + 1]);
                }

                if (!(childMasks & 0x80))
                {
                    hitColor = _octree[childOffset + parent + BitCount[((childMasks >> (8 + childShift)) << childShift) & 127]];
                    break;
                }

                rayStack[scale].offset = parent;
                rayStack[scale].maxT = maxT;

                drgUInt32 siblingCount = BitCount[childMasks & 127];
                parent += childOffset + siblingCount;
                if (current & 0x10000)
                {
                    parent += siblingCount;
                }

                idx = 0;
                scale--;
                scaleExp2 = half;

                if (centerTX > minT)
                    idx ^= 1, posX += scaleExp2;
                if (centerTY > minT)
                    idx ^= 2, posY += scaleExp2;
                if (centerTZ > minT)
                    idx ^= 4, posZ += scaleExp2;

                maxT = maxTV;
                current = 0;

                continue;
            }
        }

        int stepMask = 0;
        if (cornerTX <= maxTC)
            stepMask ^= 1, posX -= scaleExp2;
        if (cornerTY <= maxTC)
            stepMask ^= 2, posY -= scaleExp2;
        if (cornerTZ <= maxTC)
            stepMask ^= 4, posZ -= scaleExp2;

        minT = maxTC;
        idx ^= stepMask;

        if ((idx & stepMask) != 0)
        {
            int differingBits = 0;
            if (stepMask & 1)
                differingBits |= floatBitsToUint(posX) ^ floatBitsToUint(posX + scaleExp2);
            if (stepMask & 2)
                differingBits |= floatBitsToUint(posY) ^ floatBitsToUint(posY + scaleExp2);
            if (stepMask & 4)
                differingBits |= floatBitsToUint(posZ) ^ floatBitsToUint(posZ + scaleExp2);
            scale = (floatBitsToUint((float)differingBits) >> 23) - 127;
            scaleExp2 = uintBitsToFloat((scale - MaxScale + 127) << 23);

            parent = rayStack[scale].offset;
            maxT = rayStack[scale].maxT;

            int shX = floatBitsToUint(posX) >> scale;
            int shY = floatBitsToUint(posY) >> scale;
            int shZ = floatBitsToUint(posZ) >> scale;
            posX = uintBitsToFloat(shX << scale);
            posY = uintBitsToFloat(shY << scale);
            posZ = uintBitsToFloat(shZ << scale);
            idx = (shX & 1) | ((shY & 1) << 1) | ((shZ & 1) << 2);

            current = 0;
        }
    }

    if (scale >= MaxScale)
        return false;

    hitDistance = minT;
    return true;
}

bool svo_raymarch_o(const drgVec3 &rayOrigin, const drgVec3 &rayDirection, float rayScale, drgUInt32 &hitColor, float &hitDistance)
{

    // Stack to store parent nodes and their max distances during traversal
    struct StackEntry
    {
        drgUInt32 nodeOffset; // Offset to the parent node in the octree
        float maxDistance;    // Maximum distance for this node's traversal
    };
    StackEntry traversalStack[MaxScale + 1];

    // Extract ray origin and direction components for clarity
    float originX = rayOrigin.x, originY = rayOrigin.y, originZ = rayOrigin.z;
    float dirX = rayDirection.x, dirY = rayDirection.y, dirZ = rayDirection.z;

    // Prevent division by zero by clamping small direction components
    if (fabs(dirX) < 1e-4f)
        dirX = 1e-4f;
    if (fabs(dirY) < 1e-4f)
        dirY = 1e-4f;
    if (fabs(dirZ) < 1e-4f)
        dirZ = 1e-4f;

    // Compute inverse direction components for ray-plane intersection calculations
    float invDirX = 1.0f / -fabs(dirX);
    float invDirY = 1.0f / -fabs(dirY);
    float invDirZ = 1.0f / -fabs(dirZ);

    // Compute ray-plane intersection biases based on origin
    float biasX = invDirX * originX;
    float biasY = invDirY * originY;
    float biasZ = invDirZ * originZ;

    // Determine octant based on ray direction (used to adjust traversal order)
    drgUInt8 octantMask = 7;
    if (dirX > 0.0f)
        octantMask ^= 1, biasX = 3.0f * invDirX - biasX;
    if (dirY > 0.0f)
        octantMask ^= 2, biasY = 3.0f * invDirY - biasY;
    if (dirZ > 0.0f)
        octantMask ^= 4, biasZ = 3.0f * invDirZ - biasZ;

    // Calculate initial min and max distances for the ray within the octree
    float minDistance = max(2.0f * invDirX - biasX, max(2.0f * invDirY - biasY, 2.0f * invDirZ - biasZ));
    float maxDistance = min(invDirX - biasX, min(invDirY - biasY, invDirZ - biasZ));
    minDistance = max(minDistance, 0.0f);

    // Initialize traversal variables
    drgUInt32 currentNode = 0;       // Current node being processed
    drgUInt32 parentNode = 0;        // Offset to the parent node
    int childIndex = 0;              // Index of the current child node
    float voxelPosX = 1.0f;          // Current voxel position in X
    float voxelPosY = 1.0f;          // Current voxel position in Y
    float voxelPosZ = 1.0f;          // Current voxel position in Z
    int currentScale = MaxScale - 1; // Current scale level in the octree
    float scaleSize = 0.5f;          // Size of the voxel at the current scale

    // Adjust initial voxel position based on ray entry point
    if (1.5f * invDirX - biasX > minDistance)
        childIndex ^= 1, voxelPosX = 1.5f;
    if (1.5f * invDirY - biasY > minDistance)
        childIndex ^= 2, voxelPosY = 1.5f;
    if (1.5f * invDirZ - biasZ > minDistance)
        childIndex ^= 4, voxelPosZ = 1.5f;

    // Traverse the octree until a hit is found or the maximum scale is reached
    while (currentScale < MaxScale)
    {
        // Load the current node from the octree if not already loaded
        if (currentNode == 0)
            currentNode = _octree[parentNode];

        // Compute distances to the voxel's boundaries
        float boundaryDistX = voxelPosX * invDirX - biasX;
        float boundaryDistY = voxelPosY * invDirY - biasY;
        float boundaryDistZ = voxelPosZ * invDirZ - biasZ;
        float closestBoundaryDist = min(boundaryDistX, min(boundaryDistY, boundaryDistZ));

        // Compute child node index based on octant and ray direction
        int childShift = childIndex ^ octantMask;
        drgUInt32 childMasks = currentNode << childShift;

        // Check if the current voxel is valid and within the ray's range
        if ((childMasks & 0x8000) && minDistance <= maxDistance)
        {
            // Check if the voxel is small enough to be considered a hit
            if (closestBoundaryDist * rayScale >= scaleSize)
            {
                hitDistance = closestBoundaryDist;
                return true;
            }

            // Compute distances to the voxel's center for child traversal
            float maxChildDist = min(maxDistance, closestBoundaryDist);
            float halfSize = scaleSize * 0.5f;
            float centerDistX = halfSize * invDirX + boundaryDistX;
            float centerDistY = halfSize * invDirY + boundaryDistY;
            float centerDistZ = halfSize * invDirZ + boundaryDistZ;

            // Proceed to child nodes if within range
            if (minDistance <= maxChildDist)
            {
                // Compute the offset to the child nodes
                drgUInt32 childOffset = currentNode >> 18;
                if (currentNode & 0x20000)
                {
                    // fix this to only use 32 bit
                    childOffset = (childOffset << 32) | (_octree[parentNode + 1]);
                }

                // Check if the child node is a leaf (contains color data)
                if (!(childMasks & 0x80))
                {
                    hitColor = _octree[childOffset + parentNode + BitCount[((childMasks >> (8 + childShift)) << childShift) & 127]];
                    break;
                }

                // Push current state to the stack
                traversalStack[currentScale].nodeOffset = parentNode;
                traversalStack[currentScale].maxDistance = maxDistance;

                // Update parent node offset based on child nodes
                drgUInt32 siblingCount = BitCount[childMasks & 127];
                parentNode += childOffset + siblingCount;
                if (currentNode & 0x10000)
                {
                    parentNode += siblingCount;
                }

                // Move to the next level of the octree
                childIndex = 0;
                currentScale--;
                scaleSize = halfSize;

                // Adjust voxel position for the child node
                if (centerDistX > minDistance)
                    childIndex ^= 1, voxelPosX += scaleSize;
                if (centerDistY > minDistance)
                    childIndex ^= 2, voxelPosY += scaleSize;
                if (centerDistZ > minDistance)
                    childIndex ^= 4, voxelPosZ += scaleSize;

                maxDistance = maxChildDist;
                currentNode = 0;

                continue;
            }
        }

        // Determine which voxel boundary was crossed
        int stepMask = 0;
        if (boundaryDistX <= closestBoundaryDist)
            stepMask ^= 1, voxelPosX -= scaleSize;
        if (boundaryDistY <= closestBoundaryDist)
            stepMask ^= 2, voxelPosY -= scaleSize;
        if (boundaryDistZ <= closestBoundaryDist)
            stepMask ^= 4, voxelPosZ -= scaleSize;

        // Update ray position and child index
        minDistance = closestBoundaryDist;
        childIndex ^= stepMask;

        // If exiting the current voxel, move up the octree
        if ((childIndex & stepMask) != 0)
        {
            voxelPosX = 0.5f;
            voxelPosY = 0.5f;
            voxelPosZ = 0.5f;
            scaleSize = 0.5f;
            int differingBits = 0;
            if (stepMask & 1)
            {
                differingBits |= floatBitsToUint(voxelPosX) ^ floatBitsToUint(voxelPosX + scaleSize);
            }
            if (stepMask & 2)
            {
                differingBits |= floatBitsToUint(voxelPosY) ^ floatBitsToUint(voxelPosY + scaleSize);
            }
            if (stepMask & 4)
            {
                differingBits |= floatBitsToUint(voxelPosZ) ^ floatBitsToUint(voxelPosZ + scaleSize);
            }

            // Compute new scale based on the differing bits
            union
            {
                drgUInt32 i;
                float f;
            } unionHack;
            unionHack.f = ((float)differingBits);
            currentScale = unionHack.i;
            currentScale = (currentScale >> 23);
            currentScale = (currentScale - 127);

            // Update scale size for the new level
            scaleSize = uintBitsToFloat((currentScale - MaxScale + 127) << 23);

            // Pop state from the stack
            parentNode = traversalStack[currentScale].nodeOffset;
            maxDistance = traversalStack[currentScale].maxDistance;

            // Adjust voxel position and child index for the new level
            int shiftX = floatBitsToUint(voxelPosX) >> currentScale;
            int shiftY = floatBitsToUint(voxelPosY) >> currentScale;
            int shiftZ = floatBitsToUint(voxelPosZ) >> currentScale;
            voxelPosX = uintBitsToFloat(shiftX << currentScale);
            voxelPosY = uintBitsToFloat(shiftY << currentScale);
            voxelPosZ = uintBitsToFloat(shiftZ << currentScale);
            childIndex = (shiftX & 1) | ((shiftY & 1) << 1) | ((shiftZ & 1) << 2);

            currentNode = 0;
        }
    }

    // If maximum scale is reached, no hit was found
    if (currentScale >= MaxScale)
        return false;

    // Set hit distance and return success
    hitDistance = minDistance;
    return true;
}

drgSVOBuilder *build_recursive(drgAABB aabb, int depth, drgVec3 sphere_center, float sphere_radius, int max_depth, unsigned int *total_nodes)
{
    drgSVOBuilder *node = (drgSVOBuilder *)malloc(sizeof(drgSVOBuilder));
    node->aabb = aabb;
    node->index = (*total_nodes);
    (*total_nodes)++;

    if (depth >= max_depth)
    {
        node->is_leaf = 1;
        drgVec3 center = aabb_center(aabb);
        drgVec3 delta = {
            center.x - sphere_center.x,
            center.y - sphere_center.y,
            center.z - sphere_center.z};
        float distance = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
        if (distance <= sphere_radius)
        {
            // node->leaf.color = (drgVec3){rnorm(), rnorm(), rnorm()}; // Random
            node->color = (drgVec3){0.0f, 0.0f, 1.0f}; // Blue
            node->normal = delta;
            node->normal.Normalize();

            return node;
        }
        else
        {
            (*total_nodes)--;
            free(node);
            return NULL;
        }
    }

    node->is_leaf = 0;
    drgVec3 mid = aabb_center(aabb);
    drgAABB child_aabbs[8] = {
        {{aabb.min_val.x, aabb.min_val.y, aabb.min_val.z}, 0, {mid.x, mid.y, mid.z}, 0},
        {{mid.x, aabb.min_val.y, aabb.min_val.z}, 0, {aabb.max_val.x, mid.y, mid.z}, 0},
        {{aabb.min_val.x, mid.y, aabb.min_val.z}, 0, {mid.x, aabb.max_val.y, mid.z}, 0},
        {{mid.x, mid.y, aabb.min_val.z}, 0, {aabb.max_val.x, aabb.max_val.y, mid.z}, 0},
        {{aabb.min_val.x, aabb.min_val.y, mid.z}, 0, {mid.x, mid.y, aabb.max_val.z}, 0},
        {{mid.x, aabb.min_val.y, mid.z}, 0, {aabb.max_val.x, mid.y, aabb.max_val.z}, 0},
        {{aabb.min_val.x, mid.y, mid.z}, 0, {mid.x, aabb.max_val.y, aabb.max_val.z}, 0},
        {{mid.x, mid.y, mid.z}, 0, {aabb.max_val.x, aabb.max_val.y, aabb.max_val.z}, 0}};

    bool has_children = false;
    for (int i = 0; i < 8; i++)
    {
        drgVec3 child_center = aabb_center(child_aabbs[i]);
        drgVec3 closest = {
            max(child_aabbs[i].min_val.x, min(sphere_center.x, child_aabbs[i].max_val.x)),
            max(child_aabbs[i].min_val.y, min(sphere_center.y, child_aabbs[i].max_val.y)),
            max(child_aabbs[i].min_val.z, min(sphere_center.z, child_aabbs[i].max_val.z))};
        drgVec3 delta = {
            closest.x - sphere_center.x,
            closest.y - sphere_center.y,
            closest.z - sphere_center.z};
        float distance_squared = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
        if (distance_squared <= sphere_radius * sphere_radius)
        {
            node->children[i] = build_recursive(child_aabbs[i], depth + 1, sphere_center, sphere_radius, max_depth, total_nodes);
            if (node->children[i])
                has_children = true;
        }
        else
        {
            node->children[i] = NULL;
        }
    }

    if (!has_children)
    {
        (*total_nodes)--;
        free(node);
        return NULL;
    }
    return node;
}

void populate_svo_nodes(drgSVONode *node_list, drgSVOBuilder *node)
{
    if (!node)
    {
        return;
    }

    node_list[node->index].aabb = node->aabb;
    node_list[node->index].is_leaf = node->is_leaf;
    node_list[node->index].color = node->color;
    node_list[node->index].normal = node->normal;

    if (!node->is_leaf)
    {
        for (int i = 0; i < 8; i++)
        {
            node_list[node->index].children[i] = 0;
            if (node->children[i])
            {
                node_list[node->index].children[i] = node->children[i]->index;
                populate_svo_nodes(node_list, node->children[i]);
            }
        }
    }
}

// Create hardcoded SVO for a sphere
drgSVONode *create_sphere_svo(drgVec3 sphere_center, float sphere_radius, int max_depth, unsigned int *total_nodes)
{
    drgAABB root_aabb = {{-1.0f, -1.0f, -1.0f}, 0, {1.0f, 1.0f, 1.0f}, 0};
    drgSVOBuilder *root = (drgSVOBuilder *)malloc(sizeof(drgSVOBuilder));
    root->aabb = root_aabb;

    (*total_nodes) = 0;
    root = build_recursive(root_aabb, 0, sphere_center, sphere_radius, max_depth, total_nodes);

    unsigned int size = sizeof(drgSVONode) * (*total_nodes);
    drgSVONode *nodes = (drgSVONode *)malloc(size);
    populate_svo_nodes(nodes, root);

    return nodes;
}

static const size_t CompressionBlockSize = 64 * 1024 * 1024;

unsigned int *get_octree_test(unsigned int *total_nodes)
{
    FILE *fp = fopen("C:/Users/dylan/Downloads/sparse-voxel-octrees-master/sparse-voxel-octrees-master/models/XYZRGB-Dragon.oct", "rb");

    if (fp)
    {
        drgVec3 center;
        drgUInt64 _octreeSize = 0;
        fread(&center, sizeof(float), 3, fp);
        fread(&_octreeSize, sizeof(drgUInt64), 1, fp);

        _octree_total = _octreeSize;
        _octree = (drgUInt32 *)malloc(sizeof(drgUInt32) * _octreeSize);

        char *buffer = new char[LZ4_compressBound(CompressionBlockSize)];
        char *dst = (char *)_octree;

        LZ4_streamDecode_t *stream = LZ4_createStreamDecode();
        LZ4_setStreamDecode(stream, dst, 0);

        drgUInt64 compressedSize = 0;
        for (drgUInt64 offset = 0; offset < _octreeSize * sizeof(drgUInt32); offset += CompressionBlockSize)
        {
            drgUInt64 compSize;
            fread(&compSize, sizeof(drgUInt64), 1, fp);
            fread(buffer, sizeof(char), size_t(compSize), fp);

            int outSize = min(_octreeSize * sizeof(drgUInt32) - offset, CompressionBlockSize);
            LZ4_decompress_fast_continue(stream, buffer, dst + offset, outSize);
            compressedSize += compSize + 8;
        }
        LZ4_freeStreamDecode(stream);

        fclose(fp);
    }

    (*total_nodes) = _octree_total;
    return _octree;
}

// drgSVORenderer: Renders an AABB to an 800x600 buffer (4 bytes per pixel)
class drgSVORenderer
{
public:
    static const int WIDTH = 800;
    static const int HEIGHT = 600;
    static const int BUFFER_SIZE = WIDTH * HEIGHT * 4; // RGBA, 4 bytes per pixel

    drgSVORenderer()
    {
        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            buffer[i] = 0;
        }
    }

    bool intersect_aabb(const drgVec3 &rayOrigin, const drgVec3 &rayDirection, drgAABB *aabb, float *t_near, float *t_far)
    {
        *t_near = -INFINITY;
        *t_far = INFINITY;
        for (int i = 0; i < 3; i++)
        {
            float t1 = (aabb->min_val[i] - rayOrigin[i]) / rayDirection[i];
            float t2 = (aabb->max_val[i] - rayOrigin[i]) / rayDirection[i];
            if (t1 > t2)
            {
                float tmp = t1;
                t1 = t2;
                t2 = tmp;
            }
            *t_near = max(*t_near, t1);
            *t_far = min(*t_far, t2);
        }
        return *t_near <= *t_far && *t_far >= 0;
    }

    // Lambertian shading
    drgVec3 shade(drgVec3 voxel_color, drgVec3 normal, drgVec3 light_dir)
    {
        float diffuse = max(0.0f, drgVec3::Dot(normal, light_dir));
        return drgVec3::Scale(diffuse, voxel_color);
    }

    void render_svo(const drgCamera &_camera)
    {
        // Octree transformation (center [0, 1]^3 at (0, 0, 0))
        drgMat44 octree_transform;
        octree_transform.Identity();
        drgMat44 inv_octree_transform = octree_transform.Inverse();

        // Combine view and inverse octree transform
        drgMat44 combined = _camera.view;
        combined.Multiply(inv_octree_transform);
        drgMat44 inv_combined = combined.Inverse();

        // Compute projection matrix
        drgMat44 inv_proj = _camera.projection.Inverse();

        // Transform camera position to octree local space
        drgVec3 local_cam_eye = _camera.position;
        inv_octree_transform.TransformVector(local_cam_eye);

        drgVec3 light_dir = {0.577350f, 0.577350f, 0.577350f}; // (1, 1, 1) / sqrt(3)

        // Render each pixel
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                // NDC coordinates: x, y in [-1, 1], z at near plane
                float ndc_x = 2.0f * ((float)x + 0.5f) / WIDTH - 1.0f;
                float ndc_y = 1.0f - 2.0f * ((float)y + 0.5f) / HEIGHT;
                float ndc_z = 0.0f; // Near plane

                // Unproject to view space
                drgVec3 view_pos = {ndc_x, ndc_y, ndc_z};
                inv_proj.TransformVector(view_pos);

                // Transform to octree local space
                drgVec3 local_pos = view_pos;
                inv_combined.TransformVector(local_pos);

                // Compute ray direction in octree local space
                drgVec3 ray_dir = drgVec3::Normalize(drgVec3::Sub(local_cam_eye, local_pos));

                // Raymarch
                drgUInt32 hit_color = 0;
                float hit_distance;
                bool hit = svo_raymarch(local_cam_eye, ray_dir, 0.00001f, hit_color, hit_distance);

                // Set pixel color
                drgVec3 pixel_color;
                if (hit)
                {
                    drgVec3 n;
                    float c;
                    decompressMaterial(hit_color, n, c);
                    drgVec3 hitc = {c, c, c};
                    pixel_color = shade(hitc, n, light_dir);
                
                    //pixel_color.x = ((float)((hit_color >> 16) & 0xFF)) / 255.0f; // Red
                    //pixel_color.y = ((float)((hit_color >> 8) & 0xFF)) / 255.0f;  // Green
                    //pixel_color.z = ((float)(hit_color & 0xFF)) / 255.0f;         // Blue
                }
                else
                {
                    pixel_color = drgVec3{0.0f, 0.0f, 0.0f}; // Black background
                }

                set_pixel(x, y, pixel_color);
            }
        }
    }

    void render_svo_o(const drgCamera &_camera)
    {
        drgVec3 cam_target = {0.0f, 0.0f, 0.0f};
        // drgVec3 cam_eye = _camera.position;
        drgVec3 cam_eye = {cam_target.x, cam_target.y, _camera.position.z};
        drgVec3 cam_up = {0.0, 1.0, 0.0};
        float aspect_ratio = ((float)WIDTH) / ((float)HEIGHT); // 4/3
        float fov = 90.0f;

        // Compute field of view scale
        float fov_scale = tan(fov * 0.5 * 3.14159265358979323846 / 180.0);

        // Camera basis
        drgVec3 tmp1 = drgVec3::Sub(cam_target, cam_eye);
        drgVec3 forward = drgVec3::Normalize(tmp1);
        drgVec3 right = drgVec3::Normalize(drgVec3::Cross(forward, cam_up));
        cam_up = drgVec3::Cross(right, forward); // Recompute up to ensure orthogonality

        drgVec3 light_dir = {0.577350f, 0.577350f, 0.577350f}; // (1, 1, 1) / sqrt(3)

        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
#if 0 // jitter
                drgVec3 pixel_color = {0, 0, 0};
                int samples = 4; // 2x2 subpixels
                float inv_samples = 1.0f / samples;

                // 2x2 stratified jittering
                for (int sy = 0; sy < 2; sy++)
                {
                    for (int sx = 0; sx < 2; sx++)
                    {
                        // Subpixel coordinates with jitter
                        float u = (x + (sx + rng_float()) * 0.5f) / (WIDTH - 1);
                        float v = (y + (sy + rng_float()) * 0.5f) / (HEIGHT - 1);
                        u = 2.0f * u - 1.0f; // [-1, 1]
                        v = 1.0f - 2.0f * v; // [1, -1]

                        // Generate ray
                        drgRay ray;
                        ray.origin = cam_eye;
                        drgVec3 pixel_dir = drgVec3::Add(
                            drgVec3::Add(forward, drgVec3::Scale(u, right)),
                            drgVec3::Scale(v, cam_up));
                        ray.dir = drgVec3::Normalize(pixel_dir);

                        // Trace and shade
                        drgHit hit = trace_svo(&ray, svo);
                        drgVec3 color = hit.hit ? shade(hit.color, hit.normal, light_dir) : (drgVec3){0, 0, 0};
                        pixel_color = drgVec3::Add(pixel_color, drgVec3::Scale(inv_samples, color));
                    }
                }

                set_pixel(x, y, pixel_color);
#else

                // Normalized device coordinates (NDC): map pixel (x, y) to [-1, 1] range
                float ndc_x = (2.0 * (x + 0.5) / WIDTH - 1.0) * aspect_ratio * fov_scale;
                float ndc_y = (1.0 - 2.0 * (y + 0.5) / HEIGHT) * fov_scale;

                // Ray direction in world space
                drgVec3 ray_dir = drgVec3::Normalize(
                    drgVec3::Add(
                        drgVec3::Add(
                            drgVec3::Scale(forward, 1.0),
                            drgVec3::Scale(right, ndc_x)),
                        drgVec3::Scale(cam_up, ndc_y)));

#if 0 // just show the ray color for sanity check
                tmp1 = (drgVec3){(ray_dir.x + 1)/2, (ray_dir.y + 1)/2, (ray_dir.z + 1)/2};
#if 1 // test intersection on root node for sanity
                float t_near, t_far;
                if (intersect_aabb(&cam_eye, &ray_dir, &svo[0].aabb, &t_near, &t_far))
                {
                    tmp1 = (drgVec3){0.0f, 1.0f, 0.0f};
                }
#endif

#elif 1

                struct ColVal
                {
                    union
                    {
                        unsigned int uic;
                        unsigned char c[4];
                    };
                };

                drgUInt64 vox_data = 0;
                ColVal col;
                float depth = 0.0f;
                if ((x == 0) && (y == 0))
                {
                    ray_dir.Print("UL");
                }
                if ((x == (WIDTH - 1)) && (y == 0))
                {
                    ray_dir.Print("UR");
                }
                if ((x == 0) && (y == (HEIGHT - 1)))
                {
                    ray_dir.Print("BL");
                }
                if ((x == (WIDTH - 1)) && (y == (HEIGHT - 1)))
                {
                    ray_dir.Print("BR");
                }
                bool hit = svo_raymarch(cam_eye, ray_dir, 0.001, col.uic, depth);
                if (hit)
                { // hit
                    tmp1 = drgVec3{((float)col.c[2]) / 255.0f, ((float)col.c[1]) / 255.0f, ((float)col.c[0]) / 255.0f};
                }
                else
                {                                  // miss
                    tmp1 = drgVec3{0.0, 0.0, 0.0}; // Black background
                }
#else
                drgHit hit = trace_svo(&ray, svo);

                if (hit.hit)
                { // hit
                    tmp1 = shade(hit.color, hit.normal, light_dir);
                    // tmp1 = hit.color;
                }
                else
                { // miss
                    // tmp1 = drgVec3{(ray.dir.x + 1) / 2, (ray.dir.y + 1) / 2, (ray.dir.z + 1) / 2};
                    tmp1 = drgVec3{0.0, 0.0, 0.0}; // Black background
                }
#endif
                set_pixel(x, y, tmp1);
#endif
            }
        }
    }

    void render(const drgCamera &_camera)
    {
        rng_seed((unsigned int)time(NULL));
        render_svo(_camera);
    }

    unsigned char *getbuffer()
    {
        return buffer;
    }

    // Save the buffer to a PPM file, ignoring alpha byte
    void save_ppm(const char *filename) const
    {
        FILE *file = fopen(filename, "wb");
        if (!file)
        {
            printf("Error: Could not open file %s\n", filename);
            return;
        }
        fprintf(file, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        // Write only RGB, skipping alpha
        for (int i = 0; i < WIDTH * HEIGHT; ++i)
        {
            fwrite(&buffer[i * 4], sizeof(unsigned char), 3, file); // Write R, G, B
        }
        fclose(file);
    }

private:
    unsigned char buffer[WIDTH * HEIGHT * 4]; // RGBA buffer (4 bytes per pixel)

    // Set pixel color in the buffer (RGBA, 0-1 to 0-255, alpha=255)
    inline void set_pixel(int x, int y, const drgVec3 &color)
    {
        int idx = (y * WIDTH + x) * 4;
        buffer[idx] = (unsigned char)(color.x * 255.0f);     // R
        buffer[idx + 1] = (unsigned char)(color.y * 255.0f); // G
        buffer[idx + 2] = (unsigned char)(color.z * 255.0f); // B
        buffer[idx + 3] = 255;                               // A (opaque)
    }
};

#endif // DRAGO_AABB_RENDERER_H