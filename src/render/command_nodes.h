#ifndef __DRG_DRAW_LIST_NODES__
#define __DRG_DRAW_LIST_NODES__

#include "render/material.h"
#include "system/debug.h"

class drgTexture;
class drgMaterial;
class drgVertBuffer;
class drgModel;
class drgRenderTarget;
class drgDrawCommandBuffer;

enum DRG_BLEND_FUNCTION
{
	DRG_BLEND_FUNC_SOURCE,
	DRG_BLEND_FUNC_INV_SOURCE,
	DRG_BLEND_FUNC_DEST,
	DRG_BLEND_FUNC_INV_DEST,
	DRG_BLEND_FUNC_ZERO,
	DRG_BLEND_FUNC_ONE,
};

enum DRG_BLEND_OPERATION
{
	DRG_BLEND_OP_ADD,
	DRG_BLEND_OP_SUB,
	DRG_BLEND_OP_MULT,
};

enum DRG_COMPARE_FUNC
{
	DRG_COMPARE_LESS,
	DRG_COMPARE_LEQUAL,
	DRG_COMPARE_GREATER,
	DRG_COMPARE_GEQUAL,
	DRG_COMPARE_MIN,
	DRG_COMPARE_MAX,
	DRG_COMPARE_EQUAL,
	DRG_COMPARE_NOTEQUAL,
	DRG_COMPARE_ALWAYS,
	DRG_COMPARE_NEVER,
};

enum DRG_CULL_MODE
{
	DRG_CULL_NONE,
	DRG_CULL_BACK,
	DRG_CULL_FRONT
};

enum DRG_WINDING_ORDER
{
	DRG_WINDING_CLOCKWISE,
	DRG_WINDING_COUNTER_CLOCKWISE
};

enum DRG_FILL_MODE
{
	DRG_FILL_POLYGON,
	DRG_FILL_WIREFRAME
};

// The type of node
enum DRG_DRAW_COMMAND_TYPE
{
	DRG_DRAW_COMMAND_NONE,
	DRG_DRAW_COMMAND_CLEAR_TARGET,
	DRG_DRAW_COMMAND_SWAP_BUFFERS,
	DRG_DRAW_COMMAND_MESH,
	DRG_DRAW_COMMAND_MODEL,
	DRG_DRAW_COMMAND_UNIFORM,
	DRG_DRAW_COMMAND_SET_PASS_MODE,
	DRG_DRAW_COMMAND_DYNAMIC,
	DRG_DRAW_COMMAND_DYNAMIC_DRAW_VERT_LIST,
	DRG_DRAW_COMMAND_DYNAMIC_DRAW_VERT_LIST_TEX,
	DRG_DRAW_COMMAND_DYNAMIC_DRAW_LINE,
	DRG_DRAW_COMMAND_ENABLE_SCISSOR,
	DRG_DRAW_COMMAND_SCISSOR,
	DRG_DRAW_COMMAND_VIEWPORT,
	DRG_DRAW_COMMAND_COLOR_BLEND,
	DRG_DRAW_COMMAND_ALPHA_BLEND,
	DRG_DRAW_COMMAND_ENABLE_COLOR_BLEND,
	DRG_DRAW_COMMAND_COLOR_BLEND_SOURCE,
	DRG_DRAW_COMMAND_COLOR_BLEND_DEST,
	DRG_DRAW_COMMAND_COLOR_BLEND_OPERATION,
	DRG_DRAW_COMMAND_ENABLE_ALPHA_BLEND,
	DRG_DRAW_COMMAND_ALPHA_BLEND_SOURCE,
	DRG_DRAW_COMMAND_ALPHA_BLEND_DEST,
	DRG_DRAW_COMMAND_ALPHA_BLEND_OPERATION,
	DRG_DRAW_COMMAND_DEPTH_STATE,
	DRG_DRAW_COMMAND_STENCIL_STATE,
	DRG_DRAW_COMMAND_ENABLE_DEPTH_TEST,
	DRG_DRAW_COMMAND_ENABLE_DEPTH_WRITE,
	DRG_DRAW_COMMAND_DEPTH_COMPARE_FUNC,
	DRG_DRAW_COMMAND_ENABLE_STENCIL_TEST,
	DRG_DRAW_COMMAND_ENABLE_STENCIL_WRITE,
	DRG_DRAW_COMMAND_STENCIL_COMPARE_FUNC,
	DRG_DRAW_COMMAND_STENCIL_REF,
	DRG_DRAW_COMMAND_STENCIL_MASK,
	DRG_DRAW_COMMAND_CULL_MODE,
	DRG_DRAW_COMMAND_FRONT_FACE,
	DRG_DRAW_COMMAND_FILL_MODE,
	DRG_DRAW_COMMAND_MAKE_CURRENT,
	DRG_DRAW_COMMAND_BIND_RENDER_TARGET,
	DRG_DRAW_COMMAND_UNBIND_RENDER_TARGET,
	DRG_DRAW_COMMAND_UPDATE_SURFACES,
	DRG_DRAW_COMMAND_RUN_COMMAND_LIST,
	DRG_DRAW_COMMAND_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////
// drgMeshNode: This is the primary draw node, used to simply draw an object with a material
////////////////////////////////////////////////////////////////////////////////////////////

struct drgMeshNode
{
	drgMaterial *m_Material;
	drgVertBuffer *m_VertexBuffer;
	int m_StartPrimitive;
	int m_NumPrimitives;
	int m_SetupBuffer;
	int m_UseEffect;

	drgMeshNode(drgMaterial *material,
				drgVertBuffer *buffer,
				int startPrim,
				int numPrims,
				bool setup,
				bool useEffect)
	{
		m_Material = material;
		m_VertexBuffer = buffer;
		m_StartPrimitive = startPrim;
		m_NumPrimitives = numPrims;
		m_SetupBuffer = setup ? 1 : 0;
		m_UseEffect = useEffect ? 1 : 0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
// drgModelNode: This will draw a model
////////////////////////////////////////////////////////////////////////////////////////////

struct drgModelNode
{
	drgModel *m_Model;
	drgBitArray *m_Chunks;
	drgModelNode(drgModel *model, drgBitArray *chunks)
	{
		assert(model);
		m_Model = model;
		m_Chunks = chunks;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
// drgModelNode: This will draw a model
////////////////////////////////////////////////////////////////////////////////////////////

struct drgPassModeNode
{
	drgMaterial::DRG_MATERIAL_PASS_MODE m_PassMode;
	drgPassModeNode(drgMaterial::DRG_MATERIAL_PASS_MODE type)
	{
		m_PassMode = type;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgDynamicTexturedVertNode: This can only be used by a dynamic draw list. It will read verts from the buffer and draw them
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgDynamicTexturedVertNode
{
	int m_StartOffset;	   // The offset into the vertex stream where my verts begin
	int m_NumVertices;	   // The number of vertices in the stream
	drgTexture *m_Texture; // The texture to use for the vertices.

	drgDynamicTexturedVertNode(int startOffset, int numVerts, drgTexture *texture)
	{
		m_StartOffset = startOffset;
		m_NumVertices = numVerts;
		m_Texture = texture;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgDynamicLineNode
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgDynamicLineNode
{
	int m_StartOffset;
	int m_NumVerts;

	drgDynamicLineNode(int startOffset, int numVerts)
	{
		m_StartOffset = startOffset;
		m_NumVerts = numVerts;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgDynamicFlushNode: Will cause the list to flush all commands
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgDynamicFlushNode
{
	int m_Offset;

	drgDynamicFlushNode(int offset)
	{
		m_Offset = offset;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgClearTargetNode: Will clear a specified render target
////////////////////////////////////////////////////////////////////////////////////////////////

enum DRG_CLEAR_FLAG
{
	DRG_CLEAR_COLOR = 0x00000001,
	DRG_CLEAR_DEPTH = 0x00000002,
	DRG_CLEAR_STENCIL = 0x00000004,
};

#define DRG_CLEAR_ALL (DRG_CLEAR_COLOR | DRG_CLEAR_DEPTH | DRG_CLEAR_STENCIL)

struct drgClearTargetNode
{
	unsigned int m_ClearFlags;
	unsigned int m_StencilClear;
	float m_DepthClear;
	drgColor m_ClearColor;

	drgClearTargetNode(int clearFlags,
					   drgColor color,
					   float depth = 1.0f,
					   unsigned int stencil = 0x00000001)
	{
		m_ClearFlags = clearFlags;
		m_ClearColor = color;
		m_DepthClear = depth;
		m_StencilClear = stencil;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgSwapBuffersNode: Will swap the back buffer to the front buffer
////////////////////////////////////////////////////////////////////////////////////////////////

class GUI_Window;
struct drgSwapBuffersNode
{
	GUI_Window *m_Window;

	drgSwapBuffersNode(GUI_Window *window)
	{
		assert(window);
		m_Window = window;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgMakeCurrentNode: Will make the specified context current
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgMakeCurrentNode
{
	void *m_PlatformRenderContext;
	void *m_PlatformDeviceContext;

	drgMakeCurrentNode(void *renderContext, void *deviceContext)
	{
		m_PlatformRenderContext = renderContext;
		m_PlatformDeviceContext = deviceContext;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgBindRenderTarget: Will bind the render target
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgBindRenderTarget
{
	drgRenderTarget *m_Target;
	int m_Index;

	drgBindRenderTarget(drgRenderTarget *target, int index)
	{
		m_Target = target;
		m_Index = index;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgBindRenderTargetTexture: Will bind the render target texture
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgBindRenderTargetTexture
{
	drgRenderTarget *m_Target;
	int m_Index;

	drgBindRenderTargetTexture(drgRenderTarget *target, int index)
	{
		m_Target = target;
		m_Index = index;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgUnbindRenderTarget: Will unbind the render target
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgUnbindRenderTarget
{
	drgRenderTarget *m_Target;

	drgUnbindRenderTarget(drgRenderTarget *target)
	{
		m_Target = target;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgUnbindRenderTargetTexture: Will unbind the render target texture
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgUnbindRenderTargetTexture
{
	drgRenderTarget *m_Target;

	drgUnbindRenderTargetTexture(drgRenderTarget *target)
	{
		m_Target = target;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// drgRunCommandList
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgRunCommandBuffer
{
	drgDrawCommandBuffer *m_CommandList;
	unsigned int m_Flush;

	drgRunCommandBuffer(drgDrawCommandBuffer *list, bool flush)
	{
		assert(list);
		m_CommandList = list;
		m_Flush = flush ? 1 : 0;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// drgUniformNode: Thes nodes are used to set a new uniform
//////////////////////////////////////////////////////////////////////////////////////////////////

enum DRG_DRAW_COMMAND_UNIFORM_TYPE
{
	DRG_UNIFORM_VIEW_PROJECTION,
	DRG_UNIFORM_AMBIENT_COLOR,
	DRG_UNIFORM_WORLD,
	DRG_UNIFORM_SHADOW_MATRIX,
	DRG_UNIFORM_BONE_MATRIX,
	DRG_UNIFORM_SPECULAR,
	DRG_UNIFORM_DIFFUSE,
	DRG_UNIFORM_TEXCOORD_OFFSET,
	DRG_UNIFORM_VIEW_POSITION,
	DRG_UNIFORM_VIEW_DIRECTION,
};

// Uniform Structures: There are going to be a lot of these...
struct drgMatrixUniform
{
	drgMat44 m_Matrix;
	drgMatrixUniform(drgMat44 *inMat)
	{
		m_Matrix = *inMat;
	}
};

struct drgColorUniform
{
	drgColor m_Color;

	drgColorUniform(unsigned int inColor) : m_Color(inColor)
	{
	}

	drgColorUniform(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
		: m_Color(r, g, b, a)
	{
	}

	drgColorUniform(float r, float g, float b, float a)
		: m_Color((unsigned char)(r * 255.0f),
				  (unsigned char)(r * 255.0f),
				  (unsigned char)(r * 255.0f),
				  (unsigned char)(r * 255.0f))
	{
	}
};

struct drgVector3Uniform
{
	drgVec3 m_Vector;

	drgVector3Uniform(drgVec3 *vec)
	{
		m_Vector = *vec;
	}

	drgVector3Uniform(float x, float y, float z)
		: m_Vector(x, y, z)
	{
	}
};

struct drgVector4Uniform
{
	drgVec4 m_Vector;

	drgVector4Uniform(drgVec4 *vec)
	{
		m_Vector = *vec;
	}

	drgVector4Uniform(float x, float y, float z, float w)
		: m_Vector(x, y, z, w)
	{
	}
};

struct drgBoneMatrixUniform
{
	drgMat44 m_Matrix;
	int m_BoneIndex;
	drgBoneMatrixUniform(drgMat44 *matrix, int index)
	{
		m_Matrix = *matrix;
		m_BoneIndex = index;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Blend States
////////////////////////////////////////////////////////////////////////////////////////////////////

struct drgColorBlendNode
{
	DRG_BLEND_FUNCTION m_SourceColor;
	DRG_BLEND_FUNCTION m_DestColor;
	DRG_BLEND_OPERATION m_BlendEquation;
	int m_Enable;

	drgColorBlendNode(bool enable,
					  DRG_BLEND_FUNCTION sourceColor = DRG_BLEND_FUNC_ONE,
					  DRG_BLEND_FUNCTION destColor = DRG_BLEND_FUNC_ONE,
					  DRG_BLEND_OPERATION equation = DRG_BLEND_OP_MULT)
	{
		m_SourceColor = sourceColor;
		m_DestColor = destColor;
		m_BlendEquation = equation;
		m_Enable = enable == true ? 1 : 0;
	}
};

struct drgSourceColorBlendNode
{
	DRG_BLEND_FUNCTION m_SourceColorBlend;
	drgSourceColorBlendNode(DRG_BLEND_FUNCTION color)
	{
		m_SourceColorBlend = color;
	}
};

struct drgDestColorBlendNode
{
	DRG_BLEND_FUNCTION m_DestColorBlend;
	drgDestColorBlendNode(DRG_BLEND_FUNCTION color)
	{
		m_DestColorBlend = color;
	}
};

struct drgColorBlendOperationNode
{
	DRG_BLEND_OPERATION m_Operation;
	drgColorBlendOperationNode(DRG_BLEND_OPERATION op)
	{
		m_Operation = op;
	}
};

struct drgEnableColorBlendNode
{
	int m_Enable;
	drgEnableColorBlendNode(bool enable)
	{
		m_Enable = enable == true ? 1 : 0;
	}
};

struct drgAlphaBlendNode
{
	DRG_BLEND_FUNCTION m_SourceAlpha;
	DRG_BLEND_FUNCTION m_DestAlpha;
	DRG_BLEND_OPERATION m_BlendEquation;
	int m_Enable;

	drgAlphaBlendNode(bool enable,
					  DRG_BLEND_FUNCTION sourceAlpha = DRG_BLEND_FUNC_ONE,
					  DRG_BLEND_FUNCTION destAlpha = DRG_BLEND_FUNC_INV_SOURCE,
					  DRG_BLEND_OPERATION equation = DRG_BLEND_OP_MULT)
	{
		m_SourceAlpha = sourceAlpha;
		m_DestAlpha = destAlpha;
		m_BlendEquation = equation;
		m_Enable = enable == true ? 1 : 0;
	}
};

struct drgSourceAlphaBlendNode
{
	DRG_BLEND_FUNCTION m_SourceAlphaBlend;
	drgSourceAlphaBlendNode(DRG_BLEND_FUNCTION func)
	{
		m_SourceAlphaBlend = func;
	}
};

struct drgDestAlphaBlendNode
{
	DRG_BLEND_FUNCTION m_DestAlphaBlend;
	drgDestAlphaBlendNode(DRG_BLEND_FUNCTION func)
	{
		m_DestAlphaBlend = func;
	}
};

struct drgAlphaBlendOperationNode
{
	DRG_BLEND_OPERATION m_Operation;
	drgAlphaBlendOperationNode(DRG_BLEND_OPERATION op)
	{
		m_Operation = op;
	}
};

struct drgEnableAlphaBlendNode
{
	int m_Enable;
	drgEnableAlphaBlendNode(bool enable)
	{
		m_Enable = enable == true ? 1 : 0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Depth/Stencil States
////////////////////////////////////////////////////////////////////////////////////////////////////

struct drgDepthStateNode
{
	DRG_COMPARE_FUNC m_CompareFunc;
	int m_EnableDepthTest;
	int m_EnableDepthWrite;

	drgDepthStateNode(bool enableDepthTest,
					  bool enableDepthWrite,
					  DRG_COMPARE_FUNC compareFunc = DRG_COMPARE_LEQUAL)
	{
		m_EnableDepthTest = enableDepthTest == true ? 1 : 0;
		m_EnableDepthWrite = enableDepthWrite == true ? 1 : 0;
		m_CompareFunc = compareFunc;
	}
};

struct drgDepthCompareFuncNode
{
	DRG_COMPARE_FUNC m_CompareFunc;
	drgDepthCompareFuncNode(DRG_COMPARE_FUNC func)
	{
		m_CompareFunc = func;
	}
};

struct drgDepthTestEnableNode
{
	int m_Enable;
	drgDepthTestEnableNode(bool enable)
	{
		m_Enable = enable == true ? 1 : 0;
	}
};

struct drgDepthWriteEnableNode
{
	int m_Enable;
	drgDepthWriteEnableNode(bool enable)
	{
		m_Enable = enable == true ? 1 : 0;
	}
};

struct drgStencilStateNode
{
	DRG_COMPARE_FUNC m_CompareFunc;
	int m_EnableStencilTest;
	int m_EnableStencilWrite;
	int m_StencilRef;
	int m_StencilMask;

	drgStencilStateNode(bool enableStencilTest,
						bool enableStencilWrite,
						DRG_COMPARE_FUNC compareFunc,
						int stencilRef,
						int stencilMask)
	{
		m_EnableStencilTest = enableStencilTest == true ? 1 : 0;
		m_EnableStencilWrite = enableStencilWrite == true ? 1 : 0;
		m_CompareFunc = compareFunc;
		m_StencilRef = stencilRef;
		m_StencilMask = stencilMask;
	}
};

struct drgStencilCompareFuncNode
{
	DRG_COMPARE_FUNC m_CompareFunc;
	drgStencilCompareFuncNode(DRG_COMPARE_FUNC func)
	{
		m_CompareFunc = func;
	}
};

struct drgStencilRefNode
{
	int m_StencilRef;
	drgStencilRefNode(int ref)
	{
		m_StencilRef = ref;
	}
};

struct drgStencilMaskNode
{
	int m_StencilMask;
	drgStencilMaskNode(int mask)
	{
		m_StencilMask = mask;
	}
};

struct drgStencilTestEnableNode
{
	int m_Enable;
	drgStencilTestEnableNode(bool enable)
	{
		m_Enable = enable == true ? 1 : 0;
	}
};

struct drgStencilWriteEnableNode
{
	int m_Enable;
	drgStencilWriteEnableNode(bool enable)
	{
		m_Enable = enable == true ? 1 : 0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous States
////////////////////////////////////////////////////////////////////////////////////////////////////

struct drgCullNode
{
	DRG_CULL_MODE m_CullMode;
	drgCullNode(DRG_CULL_MODE mode)
	{
		m_CullMode = mode;
	}
};

struct drgFrontFaceNode
{
	DRG_WINDING_ORDER m_WindingOrder;
	drgFrontFaceNode(DRG_WINDING_ORDER order)
	{
		m_WindingOrder = order;
	}
};

struct drgFillNode
{
	DRG_FILL_MODE m_FillMode;
	drgFillNode(DRG_FILL_MODE mode)
	{
		m_FillMode = mode;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Scissor
////////////////////////////////////////////////////////////////////////////////////////////////

struct drgEnableScissorNode
{
	int m_EnableScissor;
	drgEnableScissorNode(bool enable)
	{
		m_EnableScissor = enable == true ? 1 : 0;
	}
};

struct drgScissorNode
{
	short m_Top;
	short m_Left;
	short m_Height;
	short m_Width;

	drgScissorNode(short top, short left, short height, short width)
	{
		m_Top = top;
		m_Left = left;
		m_Height = height;
		m_Width = width;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// drgViewportNode: This will be set to set a new viewport
//////////////////////////////////////////////////////////////////////////////////////////////////

struct drgViewportNode
{
	short m_Top;
	short m_Bottom;
	short m_Left;
	short m_Right;

	drgViewportNode(short top, short bottom, short left, short right)
	{
		m_Top = top;
		m_Bottom = bottom;
		m_Left = left;
		m_Right = right;
	}
};

#endif