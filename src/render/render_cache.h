#ifndef __DRG_RENDER_CACHE__
#define __DRG_RENDER_CACHE__

#include "render/context.h"
#include "render/draw_command_buffer.h"

#define DRG_MAX_RENDER_TARGETS 4
#define DRG_MAX_TEXTURE_SAMPLERS 16

// Class Declaration
// This class is responsible for managing all information that can be cached, and dispatch new commands as necessary
class drgRenderCache
{
public: // Methods
	drgRenderCache(unsigned int commandBufferListID);
	~drgRenderCache();

	void SetCommandBufferListID(unsigned int listID);
	void SetupCache();
	void CopyCache(drgRenderCache &cache);

	// Render States
	void SetScissorState(bool enable, short top, short bottom, short left, short right);

	void EnableScissor(bool enable);
	inline bool IsScissorEnabled()
	{
		return m_ScissorEnabled;
	}

	void SetClipRect(short top, short bottom, short left, short right);
	inline void GetClipRect(short *top, short *bot, short *left, short *right)
	{
		(*top) = m_ClipTop;
		(*bot) = m_ClipBot;
		(*left) = m_ClipLeft;
		(*right) = m_ClipRight;
	}

	void SetViewport(float top, float bottom, float left, float right);
	inline void GetViewport(float &top, float &bottom, float &left, float &right)
	{
		left = m_ViewportLeft;
		right = m_ViewportRight;
		top = m_ViewportTop;
		bottom = m_ViewportBottom;
	}

	void SetCullMode(DRG_CULL_MODE mode);
	inline DRG_CULL_MODE GetCullMode()
	{
		return m_CullMode;
	}

	void SetFrontFace(DRG_WINDING_ORDER order);
	inline DRG_WINDING_ORDER GetFrontFace()
	{
		return m_FrontFace;
	}

	void SetFillMode(DRG_FILL_MODE mode);
	inline DRG_FILL_MODE GetFillMode()
	{
		return m_FillMode;
	}

	//
	// Blend Functions
	//

	void SetSourceColorBlend(DRG_BLEND_FUNCTION func);
	inline DRG_BLEND_FUNCTION GetSourceColorBlend()
	{
		return m_SourceColorBlend;
	}

	void SetSourceAlphaBlend(DRG_BLEND_FUNCTION func);
	inline DRG_BLEND_FUNCTION GetSourceAlphaBlend()
	{
		return m_SourceAlphaBlend;
	}

	void SetDestColorBlend(DRG_BLEND_FUNCTION func);
	inline DRG_BLEND_FUNCTION GetDestColorBlend()
	{
		return m_DestColorBlend;
	}

	void SetDestAlphaBlend(DRG_BLEND_FUNCTION func);
	inline DRG_BLEND_FUNCTION GetDestAlphaBlend()
	{
		return m_DestAlphaBlend;
	}

	void SetColorBlendOperation(DRG_BLEND_OPERATION operation);
	inline DRG_BLEND_OPERATION GetColorBlendOperation()
	{
		return m_ColorOperation;
	}

	void SetAlphaBlendOperation(DRG_BLEND_OPERATION operation);
	inline DRG_BLEND_OPERATION GetAlphaBlendOperation()
	{
		return m_AlphaOperation;
	}

	void EnableColorBlend(bool enable);
	inline bool IsColorBlendEnabled()
	{
		return m_ColorBlendEnabled;
	}

	void EnableAlphaBlend(bool enable);
	inline bool IsAlphaBlendEnabled()
	{
		return m_AlphaBlendEnabled;
	}

	void SetColorBlend(bool enable,
					   DRG_BLEND_FUNCTION sourceBlend,
					   DRG_BLEND_FUNCTION destBlend,
					   DRG_BLEND_OPERATION operation);

	void SetAlphaBlend(bool enable,
					   DRG_BLEND_FUNCTION alphaBlend,
					   DRG_BLEND_FUNCTION destBlend,
					   DRG_BLEND_OPERATION operation);

	//
	// Depth/Stencil Functions
	//

	void SetDepthCompareFunc(DRG_COMPARE_FUNC func);
	inline DRG_COMPARE_FUNC GetDepthCompareFunc()
	{
		return m_DepthCompareFunc;
	}

	void EnableDepthTesting(bool enable);
	inline bool IsDepthTestingEnabled()
	{
		return m_DepthTestEnabled;
	}

	void EnableDepthWriting(bool enable);
	inline bool IsDepthWritingEnabled()
	{
		return m_DepthWriteEnabled;
	}

	void SetStencilCompareFunc(DRG_COMPARE_FUNC func);
	inline DRG_COMPARE_FUNC GetCompareFunc()
	{
		return m_StencilCompareFunc;
	}

	void SetStencilRef(int ref);
	inline int GetStencilRef()
	{
		return m_StencilRef;
	}

	void SetStencilMask(int mask);
	inline int GetStencilMask()
	{
		return m_StencilMask;
	}

	void EnableStencilTesting(bool enable);
	inline bool IsStencilTestingEnabled()
	{
		return m_StencilTestEnabled;
	}

	void EnableStencilWriting(bool enable);
	inline bool IsStencilWritingEnabled()
	{
		return m_StencilWriteEnabled;
	}

	void SetDepthState(bool enableDepthTest,
					   bool enableDepthWrite,
					   DRG_COMPARE_FUNC func);

	void SetStencilState(bool enableStencilTest,
						 bool enableStencilWrite,
						 DRG_COMPARE_FUNC func,
						 int ref,
						 int mask);

	void SetRenderTarget(drgRenderTarget *target, int index);
	void SetDepthTarget(drgRenderTarget *target);

protected: // Members
	unsigned int m_CommandBufferListID;

	//
	// Render States
	//

	short m_ClipTop;
	short m_ClipBot;
	short m_ClipLeft;
	short m_ClipRight;
	bool m_ScissorEnabled;

	float m_ViewportTop;
	float m_ViewportBottom;
	float m_ViewportLeft;
	float m_ViewportRight;

	DRG_BLEND_FUNCTION m_SourceColorBlend;
	DRG_BLEND_FUNCTION m_SourceAlphaBlend;
	DRG_BLEND_FUNCTION m_DestColorBlend;
	DRG_BLEND_FUNCTION m_DestAlphaBlend;
	DRG_BLEND_OPERATION m_ColorOperation;
	DRG_BLEND_OPERATION m_AlphaOperation;
	bool m_ColorBlendEnabled;
	bool m_AlphaBlendEnabled;

	DRG_COMPARE_FUNC m_DepthCompareFunc;
	DRG_COMPARE_FUNC m_StencilCompareFunc;
	bool m_DepthTestEnabled;
	bool m_DepthWriteEnabled;
	bool m_StencilTestEnabled;
	bool m_StencilWriteEnabled;
	int m_StencilRef;
	int m_StencilMask;

	DRG_FILL_MODE m_FillMode;
	DRG_CULL_MODE m_CullMode;
	DRG_WINDING_ORDER m_FrontFace;

	drgRenderTarget *m_DepthTargetBound;
	drgRenderTarget *m_RenderTargetsBound[DRG_MAX_RENDER_TARGETS];

	//
	// Friends
	//

	friend class drgRenderContext;
	friend class drgRenderContext_OGL;
};

#endif