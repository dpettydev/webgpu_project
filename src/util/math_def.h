#ifndef DRAGO_MATH_GLOBAL_H
#define DRAGO_MATH_GLOBAL_H

#include <math.h>

#define DRG_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define DRG_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define DRG_ABS(a) (((a) < 0) ? -(a) : (a))
#define DRG_ROUND(a) ((a) > 0 ? ((int)((a) + .5f)) : ((int)((a) - .5f)))

#define DRG_HIGH32(d) ((UINT32)((UINT64)(d) >> 32))
#define DRG_LOW32(d) ((UINT32)((UINT64)(d) & 0xffffffff))

#define drgSqrtF(x) sqrtf(x)
#define drgInvSqrtF(x) (1.0f / drgSqrtF(x))

#ifdef _ftol
#undef _ftol
#endif
#define _ftol(x) Float2Int(x)
// #define floor(x) DRG_FLOOR(x)
// #define DRG_FLOOR(x) Floor2Int(x)
#define DRG_FLOOR(x) floor(x)
// #define DRG_FLOOR_DOUBLE(x) FloorDouble(x)
#define DRG_FLOOR_DOUBLE(x) floor(x)
// #define ceil(x) DRG_CEIL(x)
// #define DRG_CEIL(x) Ceil2Int(x)
#define DRG_CEIL(x) ceil(x)

#define DRG_CLAMP(in, lo, hi) (((lo) > (in)) ? (lo) : ((((hi) < (in)) ? (hi) : (in))))
#define DRG_CLAMP_NORM(in) (((0.0f) > (in)) ? (0.0f) : ((((1.0f) < (in)) ? (1.0f) : (in))))
#define DRG_SIGN(a) (((a) < 0) ? -1 : 1)
#define DRG_SQR(a) ((a) * (a))
#define DRG_ROUNDINT(a, n) (((drgUInt32)(a) + (n) - 1) & ~((n) - 1))

#define DRG_UPPER32(a) ((drgUInt32)((a) >> 32))
#define DRG_LOWER32(a) ((drgUInt32)((a) & 0xFFFFFFFF))
#define DRG_UPPER16(a) ((drgUInt16)((a) >> 16))
#define DRG_LOWER16(a) ((drgUInt16)((a) & 0xFFFF))
#define DRG_UPPER8(a) ((drgUInt8)((a) >> 8))
#define DRG_LOWER8(a) ((drgUInt8)((a) & 0xFF))

#define DRG_PI (3.1415926535f)
#define DRG_PI_HALF (1.5707963267f)
#define DRG_PI_DOUBLE (6.283185307f)
#define DRG_BIGFLOAT (float(1.0e30))
#define DRG_EPSILON (0.000001f)
#define DRG_DELTA (0.0001f)

#define DRG_IN2M (1.f / 39.37f)
#define DRG_M2IN (39.37f)
#define DRG_LBS2KG (0.45359237)
#define DRG_FT2IN_SQR(x) ((x * 12.0f) * (x * 12.0f))

#define DRG_DEGREES_1 (182)
#define DRG_DEGREES_5 (910)
#define DRG_DEGREES_15 (2730)
#define DRG_DEGREES_22 (4005)
#define DRG_DEGREES_45 (8192)
#define DRG_DEGREES_90 (16384)
#define DRG_DEGREES_135 (24576)
#define DRG_DEGREES_180 (32767)
#define DRG_DEGREES_225 (40959)
#define DRG_DEGREES_270 (49151)
#define DRG_DEGREES_315 (57343)
#define DRG_DEGREES_360 (65535)
#define DRG_RAD_TO_SHORT(x) ((short)((x) * (DRG_DEGREES_180 / DRG_PI)))
#define DRG_DEG_TO_SHORT(x) ((short)((x) * (DRG_DEGREES_360 / 360.0f)))
#define DRG_SHORT_TO_RAD(x) ((((float)x) * (DRG_PI / DRG_DEGREES_180)))
#define DRG_SHORT_TO_DEG(x) ((((float)x) * (360.0f / DRG_DEGREES_360)))
#define DRG_DEGREES_TO_RADIANS(x) ((DRG_PI / 180.0f) * x)

#define DRG_MAX_SHORT (32767)
#define DRG_MIN_SHORT (-32768)
#define DRG_MAX_USHORT (65535)
#define DRG_MIN_INT (-2147483648)
#define DRG_MAX_INT (2147483647)
#define DRG_MAX_UINT (4294967295)

#define DRG_DIFF(a, b) (DRG_ABS(a - b))

#define DRG_DBL_EPSILON      2.2204460492503131e-016

#ifndef FLT_MAX
#define FLT_MAX (3.40282e+038f)
#endif

inline int Float2Int(float a)
{
    return (int)a;
}

inline short Float2Short(float a)
{
    return (short)a;
}

inline int Floor2Int(float a)
{
    return (int)floor(a);
}

inline double FloorDouble(double x)
{
    return floor(x);
}

inline int Ceil2Int(float a)
{
    return (int)ceil(a);
}

////////////////////////////////////////////////////////////////
// Inline methods
////////////////////////////////////////////////////////////////

inline float drgRadToDeg(float rad)
{
    rad *= 57.295779513082320876798154814105f;
    while (rad >= 360.0f)
        rad -= 360.0f;
    while (rad < 0.0f)
        rad += 360.0f;
    while (rad >= 360.0f)
        rad -= 360.0f;
    return rad;
}

inline short drgRadToShort(float rad)
{
    rad = (float)(int)(rad * 20860.438391054721659227594865235f);
    while (rad >= 32768.0f)
        rad -= 65535.0f;
    while (rad < -32768.0f)
        rad += 65535.0f;
    while (rad >= 32768.0f)
        rad -= 65535.0f;
    return (short)rad;
}

template <class C>
inline void DRG_SWAP(C &a, C &b)
{
    C temp = a;
    a = b;
    b = temp;
};

inline bool drgIsZero(float a)
{
    return (DRG_ABS(a) < DRG_EPSILON);
}

inline int RoundUpToPowerOfTwo(int x)
{
    int s2;
    for (s2 = 1; s2 < x;)
        s2 <<= 1;
    return s2;
}

inline int IsPowerOfTwo(int x)
{
    return (RoundUpToPowerOfTwo(x) == x);
}

inline void drgFlipEndian16(void *val)
{
    unsigned char *flipbuf = (unsigned char *)val;
    unsigned char tempval = flipbuf[0];
    flipbuf[0] = flipbuf[1];
    flipbuf[1] = tempval;
}

inline void drgFlipEndian32(void *val)
{
    unsigned char *flipbuf = (unsigned char *)val;
    unsigned char tempval = flipbuf[0];
    flipbuf[0] = flipbuf[3];
    flipbuf[3] = tempval;
    tempval = flipbuf[1];
    flipbuf[1] = flipbuf[2];
    flipbuf[2] = tempval;
}

inline void drgFlipEndian64(void *val)
{
    unsigned char *flipbuf = (unsigned char *)val;
    unsigned char tempval = flipbuf[0];
    flipbuf[0] = flipbuf[7];
    flipbuf[7] = tempval;
    tempval = flipbuf[1];
    flipbuf[1] = flipbuf[6];
    flipbuf[6] = tempval;
    tempval = flipbuf[2];
    flipbuf[2] = flipbuf[5];
    flipbuf[5] = tempval;
    tempval = flipbuf[3];
    flipbuf[3] = flipbuf[4];
    flipbuf[4] = tempval;
}

inline void drgFlipEndianM33(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < (3 * 3); ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianM34(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < (4 * 3); ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianM44(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < (4 * 4); ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianP4(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < 4; ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianP3(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < 3; ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianP2(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < 2; ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianQuat(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < 4; ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline void drgFlipEndianBBox(void *val)
{
    float *fltary = (float *)val;
    for (int i = 0; i < 6; ++i)
        drgFlipEndian32(&(fltary[i]));
}

inline short drgFloatNormToShort(float val)
{
    return (short)(val * 32767.0f);
}

inline float drgFloatNormFromShort(short val)
{
    return ((float)val) / 32767.0f;
}

inline short drgFloatUVToShort(float val)
{
    return (short)(val * 32767.0f);
}

inline float drgFloatUVFromShort(short val)
{
    return ((float)val) / 32767.0f;
}

//	New icos function  - returns smooth -1.0f to 1.0f result
//	Better, faster, stronger
static inline float icos_smooth(int angle)
{
    angle = angle << 16 >> 16;
    float x = angle * 1.0f * (((float)DRG_PI) / 65536.f);
    float x2 = x * x;
    float x3 = x * x2;
    float x5 = x3 * x2;
    float x7 = x5 * x2;
    float s = x - (.166665434f * x3) + (.832632648e-2f * x5) - (.188351023e-3f * x7);
    return ((s * s) * 2.0f) - 1.0f;
}

// TODO:figure out why this works some places
//  This function returns values between -3.0f and 1.0f
//  isin an icos methods
static inline float icos(int angle)
{
    angle = angle << 16 >> 16;

    float sign = 1.0f;
    if ((angle ^ (angle << 1)) & 0x8000)
    {
        angle = angle << 17 >> 17;
        sign = -1.0f;
    }

    float x = angle * 1.0f * (((float)DRG_PI) / 65536.f);
    float x2 = x * x;
    float x3 = x * x2;
    float x5 = x3 * x2;
    float x7 = x5 * x2;
    float s = x - .166665434f * x3 + .832632648e-2f * x5 - .188351023e-3f * x7;
    return sign * (1.0f - 2.0f * s * s);
}

static inline float isin(int angle)
{
    return icos(angle - 16384);
}

static inline short iatan2(float y, float x)
{
    return short(atan2f(y, x) * (32768.f / ((float)DRG_PI)));
}

static inline short iacos(float val)
{
    return short(acos(val) * (32768.f / ((float)DRG_PI)));
}

static inline short iasin(float val)
{
    return short(asin(val) * (32768.f / ((float)DRG_PI)));
}

static inline short idif(short from, short to)
{
    int delta = (((int)to) - ((int)from));
    if (delta >= DRG_DEGREES_180)
        delta -= DRG_DEGREES_360;
    if (delta <= -DRG_DEGREES_180)
        delta += DRG_DEGREES_360;
    return (short)delta;
}

static inline short iclamp(short val1, short val2, short angle)
{
    int retval = val2;
    short delta = idif(val1, val2);
    if (delta >= angle)
        retval = val1 + angle;
    if (delta <= -angle)
        retval = val1 - angle;
    if (retval >= DRG_DEGREES_180)
        retval -= DRG_DEGREES_360;
    if (retval <= -DRG_DEGREES_180)
        retval += DRG_DEGREES_360;
    return (short)retval;
}

class drgVec2
{
public:
    union
    {
        struct
        {
            float x, y;
        };
        float a[2];
    };

    drgVec2(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}

    inline void Set(float x_, float y_)
    {
        x = x_;
        y = y_;
    }

    inline void Add(const drgVec2 &other)
    {
        x += other.x;
        y += other.y;
    }

    inline void Sub(const drgVec2 &other)
    {
        x -= other.x;
        y -= other.y;
    }

    inline void Scale(float s)
    {
        x *= s;
        y *= s;
    }

    inline float Length() const
    {
        return sqrt(x * x + y * y);
    }

    inline drgVec2 Abs() const
    {
        return drgVec2(fabsf(x), fabsf(y));
    }

    inline drgVec2 operator*(const float other) const
    {
        return drgVec2(x * other, y * other);
    }

    inline drgVec2 operator*=(const float other)
    {
        x *= other;
        y *= other;
        return drgVec2(x, y);
    }

    inline drgVec2 operator-(const drgVec2 &other) const
    {
        return drgVec2(x - other.x, y - other.y);
    }

    inline drgVec2 operator-=(const drgVec2 &other)
    {
        x -= other.x;
        y -= other.y;
        return drgVec2(x, y);
    }

    inline drgVec2 operator+(const drgVec2 &other) const
    {
        return drgVec2(x + other.x, y + other.y);
    }

    inline drgVec2 operator+=(const drgVec2 &other)
    {
        x += other.x;
        y += other.y;
        return drgVec2(x, y);
    }

    inline drgVec2 operator/(const drgVec2 &other) const
    {
        return drgVec2(x / other.x, y / other.y);
    }

    inline drgVec2 operator-() const
    {
        return drgVec2(-x, -y);
    }

    inline float operator[](int index) const
    {
        return (&x)[index];
    }

    static inline drgVec2 Add(const drgVec2 a, const drgVec2 b)
    {
        return (drgVec2){a.x + b.x, a.y + b.y};
    }

    static inline drgVec2 Sub(const drgVec2 a, const drgVec2 b)
    {
        return (drgVec2){a.x - b.x, a.y - b.y};
    }

    static inline drgVec2 Scale(float s, drgVec2 vec)
    {
        return (drgVec2){vec.x * s, vec.y * s};
    }

    static inline drgVec2 Scale(drgVec2 vec, float s)
    {
        return (drgVec2){vec.x * s, vec.y * s};
    }

    static inline float Length(const drgVec2 vec)
    {
        return sqrt(vec.x * vec.x + vec.y * vec.y);
    }

    void Print(const char *name = NULL) const;
};

class drgVec3
{
public:
    union
    {
        struct
        {
            float x, y, z;
        };
        float a[3];
    };

    drgVec3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}
    drgVec3(const drgVec3 &vec) : x(vec.x), y(vec.y), z(vec.z) {}
    drgVec3(const drgVec2 &vec) : x(vec.x), y(vec.y), z(0.0f) {}

    inline void Set(float x_, float y_, float z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }

    inline void Add(const drgVec3 &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    inline void Sub(const drgVec3 &other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    inline void Scale(float s)
    {
        x *= s;
        y *= s;
        z *= s;
    }

    inline float Dot(const drgVec3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    inline void Cross(const drgVec3 &other)
    {
        float tx = y * other.z - z * other.y;
        float ty = z * other.x - x * other.z;
        z = x * other.y - y * other.x;
        x = tx;
        y = ty;
    }

    inline float Length() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    inline float LengthSqr() const
    {
        return (x * x + y * y + z * z);
    }

    inline void Normalize()
    {
        float len = Length();
        if (len > 0.0f)
            Scale(1.0f / len);
    }

    inline drgVec3 Abs() const
    {
        return drgVec3(fabsf(x), fabsf(y), fabsf(z));
    }

    inline void Min(drgVec3 *p1, drgVec3 *p2)
    {
        x = DRG_MIN(p1->x, p2->x);
        y = DRG_MIN(p1->y, p2->y);
        z = DRG_MIN(p1->z, p2->z);
    }

    inline void Max(drgVec3 *p1, drgVec3 *p2)
    {
        x = DRG_MAX(p1->x, p2->x);
        y = DRG_MAX(p1->y, p2->y);
        z = DRG_MAX(p1->z, p2->z);
    }

    inline bool IsZero()
    {
        return (x == 0.0f && y == 0.0f && z == 0.0f);
    }

    inline drgVec3 operator*(const float other) const
    {
        return drgVec3(x * other, y * other, z * other);
    }

    inline drgVec3 operator*=(const float other)
    {
        x *= other;
        y *= other;
        z *= other;
        return drgVec3(x, y, z);
    }

    inline drgVec3 operator-(const drgVec3 &other) const
    {
        return drgVec3(x - other.x, y - other.y, z - other.z);
    }

    inline drgVec3 operator-=(const drgVec3 &other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return drgVec3(x, y, z);
    }

    inline drgVec3 operator+(const drgVec3 &other) const
    {
        return drgVec3(x + other.x, y + other.y, z + other.z);
    }

    inline drgVec3 operator+=(const drgVec3 &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return drgVec3(x, y, z);
    }

    inline drgVec3 operator/(const drgVec3 &other) const
    {
        return drgVec3(x / other.x, y / other.y, z / other.z);
    }

    inline drgVec3 operator-() const
    {
        return drgVec3(-x, -y, -z);
    }

    inline float operator[](int index) const
    {
        return (&x)[index];
    }

    static inline drgVec3 Normalize(drgVec3 vec)
    {
        float len = sqrt(Dot(vec, vec));
        return (drgVec3){vec.x / len, vec.y / len, vec.z / len};
    }

    static inline drgVec3 Add(const drgVec3 a, const drgVec3 b)
    {
        return (drgVec3){a.x + b.x, a.y + b.y, a.z + b.z};
    }

    static inline drgVec3 Sub(const drgVec3 a, const drgVec3 b)
    {
        return (drgVec3){a.x - b.x, a.y - b.y, a.z - b.z};
    }

    static inline drgVec3 Scale(float s, drgVec3 vec)
    {
        return (drgVec3){vec.x * s, vec.y * s, vec.z * s};
    }

    static inline drgVec3 Scale(drgVec3 vec, float s)
    {
        return (drgVec3){vec.x * s, vec.y * s, vec.z * s};
    }

    static inline float Dot(const drgVec3 a, const drgVec3 b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static inline drgVec3 Cross(const drgVec3 a, const drgVec3 b)
    {
        return (drgVec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    static inline float Length(const drgVec3 vec)
    {
        return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    }

    void Print(const char *name = NULL) const;
};

class drgVec4
{
public:
    union
    {
        struct
        {
            float x, y, z, w;
        };
        float a[4];
    };

    drgVec4(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f, float w_ = 0.0f) : x(x_), y(y_), z(z_), w(w_) {}

    inline void Set(float x_, float y_, float z_, float w_)
    {
        x = x_;
        y = y_;
        z = z_;
        w = w_;
    }
};

class alignas(16) drgMat44
{
public:
    float m[4][4];

    drgMat44()
    {
        Identity();
    }

    inline void Identity()
    {
        m[0][0] = 1.0f;
        m[0][1] = 0.0f;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = 1.0f;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = 0.0f;
        m[2][2] = 1.0f;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
    }

    inline void Multiply(const drgMat44 &other)
    {
        float result[4][4];
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    result[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m[i][j] = result[i][j];
            }
        }
    }

    inline void Transform(drgVec3 &p)
    {
        float tx = m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3];
        float ty = m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3];
        float tz = m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3];
        p.x = tx;
        p.y = ty;
        p.z = tz;
    }

    inline void TransformVector(drgVec3 &v) const
    {
        float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];
        w = w != 0.0f ? w : 1.0f;
        float tx = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
        float ty = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
        float tz = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
        v.x = tx / w;
        v.y = ty / w;
        v.z = tz / w;
    }

    inline void Perspective(float fov, float aspect, float near_plane, float far_plane)
    {
        if (aspect == 0.0f || near_plane == far_plane)
            return;
        float tan_half_fov = tan(fov * 0.5f);
        float range = near_plane - far_plane;

        m[0][0] = 1.0f / (aspect * tan_half_fov);
        m[0][1] = 0.0f;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = 1.0f / tan_half_fov;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = 0.0f;
        m[2][2] = -(near_plane + far_plane) / range;
        m[2][3] = (2.0f * near_plane * far_plane) / range;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = -1.0f;
        m[3][3] = 0.0f;
    }

    inline void Ortho(float left, float right, float bottom, float top, float near_plane, float far_plane)
    {
        if (right == left || top == bottom || far_plane == near_plane)
            return;
        Identity();
        m[0][0] = 2.0f / (right - left);
        m[1][1] = 2.0f / (top - bottom);
        m[2][2] = -2.0f / (far_plane - near_plane);
        m[0][3] = -(right + left) / (right - left);
        m[1][3] = -(top + bottom) / (top - bottom);
        m[2][3] = -(far_plane + near_plane) / (far_plane - near_plane);
    }

    inline void LookAt(const drgVec3 &eye, const drgVec3 &target, const drgVec3 &up)
    {
        drgVec3 z(eye);
        z.Sub(target);
        z.Normalize();

        drgVec3 x(up);
        x.Cross(z);
        x.Normalize();

        drgVec3 y(z);
        y.Cross(x);

        m[0][0] = x.x;
        m[0][1] = x.y;
        m[0][2] = x.z;
        m[0][3] = -x.Dot(eye);
        m[1][0] = y.x;
        m[1][1] = y.y;
        m[1][2] = y.z;
        m[1][3] = -y.Dot(eye);
        m[2][0] = z.x;
        m[2][1] = z.y;
        m[2][2] = z.z;
        m[2][3] = -z.Dot(eye);
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
    }

    inline drgMat44 Inverse() const
    {
        drgMat44 inv;
        float det = 0.0f;

        // Compute determinant
        det += m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                          m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                          m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
        det -= m[0][1] * (m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                          m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                          m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
        det += m[0][2] * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                          m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                          m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
        det -= m[0][3] * (m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                          m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                          m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));

        if (fabs(det) < 1e-6f)
        {
            inv.Identity(); // Return identity if non-invertible
            return inv;
        }

        // Compute adjugate matrix
        inv.m[0][0] = (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                       m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                       m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])) /
                      det;
        inv.m[0][1] = -(m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                        m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                        m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])) /
                      det;
        inv.m[0][2] = (m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
                       m[0][2] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) +
                       m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1])) /
                      det;
        inv.m[0][3] = -(m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
                        m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) +
                        m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])) /
                      det;

        inv.m[1][0] = -(m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                        m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                        m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])) /
                      det;
        inv.m[1][1] = (m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                       m[0][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                       m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])) /
                      det;
        inv.m[1][2] = -(m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
                        m[0][2] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) +
                        m[0][3] * (m[1][0] * m[3][2] - m[1][2] * m[3][0])) /
                      det;
        inv.m[1][3] = (m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
                       m[0][2] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) +
                       m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])) /
                      det;

        inv.m[2][0] = (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                       m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                       m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) /
                      det;
        inv.m[2][1] = -(m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                        m[0][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                        m[0][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) /
                      det;
        inv.m[2][2] = (m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) -
                       m[0][1] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) +
                       m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0])) /
                      det;
        inv.m[2][3] = -(m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) -
                        m[0][1] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) +
                        m[0][3] * (m[1][0] * m[2][1] - m[1][1] * m[2][0])) /
                      det;

        inv.m[3][0] = -(m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                        m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                        m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) /
                      det;
        inv.m[3][1] = (m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                       m[0][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                       m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) /
                      det;
        inv.m[3][2] = -(m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) -
                        m[0][1] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]) +
                        m[0][2] * (m[1][0] * m[3][1] - m[1][1] * m[3][0])) /
                      det;
        inv.m[3][3] = (m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                       m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                       m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0])) /
                      det;

        return inv;
    }

    inline void operator=(const drgMat44 &other)
    {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = other.m[i][j];
    }

    inline drgMat44 operator*(const drgMat44 &other) const
    {
        drgMat44 ret = *this;
        ret.Multiply(other);
        return ret;
    }

    void Print(const char *name = NULL) const;
};

class drgBBox
{
public:
    drgVec3 minv;
    drgVec3 maxv;

    drgBBox() {}
    drgBBox(drgVec3 min_, drgVec3 max_) : minv(min_), maxv(max_) {}
    drgBBox(short top, short bottom, short left, short right) : minv((float)left, (float)top, 0.0f), maxv((float)right, (float)bottom, 0.0f) {}

    inline void Add(drgBBox *box)
    {
        if (box->minv.x < minv.x)
            minv.x = box->minv.x;
        if (box->minv.y < minv.y)
            minv.y = box->minv.y;
        if (box->minv.z < minv.z)
            minv.z = box->minv.z;
        if (box->maxv.x < minv.x)
            minv.x = box->maxv.x;
        if (box->maxv.y < minv.y)
            minv.y = box->maxv.y;
        if (box->maxv.z < minv.z)
            minv.z = box->maxv.z;

        if (box->minv.x > maxv.x)
            maxv.x = box->minv.x;
        if (box->minv.y > maxv.y)
            maxv.y = box->minv.y;
        if (box->minv.z > maxv.z)
            maxv.z = box->minv.z;
        if (box->maxv.x > maxv.x)
            maxv.x = box->maxv.x;
        if (box->maxv.y > maxv.y)
            maxv.y = box->maxv.y;
        if (box->maxv.z > maxv.z)
            maxv.z = box->maxv.z;
    }

    inline void Add(drgVec3 *point)
    {
        if (point->x < minv.x)
            minv.x = point->x;
        if (point->y < minv.y)
            minv.y = point->y;
        if (point->z < minv.z)
            minv.z = point->z;

        if (point->x > maxv.x)
            maxv.x = point->x;
        if (point->y > maxv.y)
            maxv.y = point->y;
        if (point->z > maxv.z)
            maxv.z = point->z;
    }

    inline void GetRect(short *top, short *bottom, short *left, short *right)
    {
        (*top) = Float2Short(minv.y);
        (*bottom) = Float2Short(maxv.y);
        (*left) = Float2Short(minv.x);
        (*right) = Float2Short(maxv.x);
    }
};

class drgBitArray
{
public:
	drgBitArray(unsigned int size);
	~drgBitArray();

	void SetBit(unsigned int bit);
	void ClearBit(unsigned int bit);
	bool TestBit(unsigned int bit);

private:
	unsigned int m_Size;
	unsigned int* m_Array;
};

#endif // DRAGO_MATH_GLOBAL_H
