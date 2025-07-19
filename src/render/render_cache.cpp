
#include "render/render_cache.h"
#include "render/render_target.h"
#include "system/debug.h"
#include "render/draw_command_buffer_manager.h"

drgRenderCache::drgRenderCache(unsigned int listID)
{
	m_CommandBufferListID = listID;
	for (int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget)
	{
		m_RenderTargetsBound[currentTarget] = NULL;
	}

	m_DepthTargetBound = NULL;
}

drgRenderCache::~drgRenderCache()
{
}

void drgRenderCache::SetCommandBufferListID(unsigned int listID)
{
	m_CommandBufferListID = listID;
}


void drgRenderCache::SetupCache()
{
	// Setup blend defaults
	m_AlphaBlendEnabled = false;
	m_ColorBlendEnabled = false;
	m_SourceColorBlend = DRG_BLEND_FUNC_ONE;
	m_SourceAlphaBlend = DRG_BLEND_FUNC_ONE;
	m_DestAlphaBlend = DRG_BLEND_FUNC_ZERO;
	m_DestColorBlend = DRG_BLEND_FUNC_ZERO;
	m_ColorOperation = DRG_BLEND_OP_MULT;
	m_AlphaOperation = DRG_BLEND_OP_MULT;

	// Setup depth/stencil defaults
	m_DepthTestEnabled = false;
	m_DepthWriteEnabled = true;
	m_StencilTestEnabled = false;
	m_StencilWriteEnabled = false;
	m_DepthCompareFunc = DRG_COMPARE_LESS;
	m_StencilCompareFunc = DRG_COMPARE_ALWAYS;
	m_StencilRef = 0;
	m_StencilMask = 0xFFFFFFFF;

	// Setup Clip/Viewport Values
	m_ScissorEnabled = false;	// This is a weird one. If we don't do this, font won't scissor correctly...
	m_ClipTop = m_ClipBot = m_ClipLeft = m_ClipRight = 0;
	m_ViewportTop = m_ViewportBottom = m_ViewportLeft = m_ViewportRight = 0.0f;

	// Setup Rasterizer values
	m_CullMode = DRG_CULL_BACK;
	m_FillMode = DRG_FILL_POLYGON;
	m_FrontFace = DRG_WINDING_COUNTER_CLOCKWISE;
}

void drgRenderCache::CopyCache(drgRenderCache &cache)
{
	m_AlphaBlendEnabled = cache.m_AlphaBlendEnabled;
	m_AlphaOperation = cache.m_AlphaOperation;
	m_SourceAlphaBlend = cache.m_SourceAlphaBlend;
	m_DestAlphaBlend = cache.m_DestAlphaBlend;

	m_ColorBlendEnabled = cache.m_ColorBlendEnabled;
	m_ColorOperation = cache.m_ColorOperation;
	m_SourceColorBlend = cache.m_SourceColorBlend;
	m_DestColorBlend = cache.m_DestColorBlend;

	m_DepthTestEnabled = cache.m_DepthTestEnabled;
	m_DepthWriteEnabled = cache.m_DepthWriteEnabled;
	m_DepthCompareFunc = cache.m_DepthCompareFunc;

	m_StencilTestEnabled = cache.m_StencilTestEnabled;
	m_StencilWriteEnabled = cache.m_StencilWriteEnabled;
	m_StencilCompareFunc = cache.m_StencilCompareFunc;
	m_StencilRef = cache.m_StencilRef;
	m_StencilMask = cache.m_StencilMask;

	m_FrontFace = cache.m_FrontFace;
	m_FillMode = cache.m_FillMode;
	m_CullMode = cache.m_CullMode;

	m_ScissorEnabled = cache.m_ScissorEnabled;
	m_ClipTop = cache.m_ClipTop;
	m_ClipBot = cache.m_ClipBot;
	m_ClipLeft = cache.m_ClipLeft;
	m_ClipRight = cache.m_ClipRight;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Render States
///////////////////////////////////////////////////////////////////////////////////////////////////

void drgRenderCache::EnableScissor(bool enable)
{
	if (m_ScissorEnabled != enable)
	{
		m_ScissorEnabled = enable;
		drgDrawCommandBufferManager::EnableScissor(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::SetScissorState(bool enable, short top, short bottom, short left, short right)
{
	bool enableChanged = m_ScissorEnabled != enable;
	bool topChanged = m_ClipTop != top;
	bool botChanged = m_ClipBot != bottom;
	bool leftChanged = m_ClipLeft != left;
	bool rightChanged = m_ClipRight != right;
	bool allChanged = enableChanged || (topChanged && botChanged && leftChanged && rightChanged);

	if (allChanged)
	{
		m_ScissorEnabled = enable;
		m_ClipTop = top;
		m_ClipBot = bottom;
		m_ClipLeft = left;
		m_ClipRight = right;

		drgDrawCommandBufferManager::SetScissorState(enable, top, bottom, left, right, m_CommandBufferListID);
	}
	else
	{
		EnableScissor(enable);
		SetClipRect(top, bottom, left, right);
	}
}

void drgRenderCache::SetClipRect(short top, short bottom, short left, short right)
{
	//
	// This is a bit hacky, but we're going to solve a potential crash in OGL.
	// If our right or bottom is less than our left or top, then we need to reverse it.
	//

	if (bottom < top)
	{
		top = bottom;
		bottom = top + 1;
	}

	if (right < left)
	{
		left = right;
		right = left + 1;
	}

	bool sendCommand = true; //(top != m_ClipTop || bottom != m_ClipBot || left != m_ClipLeft || right != m_ClipRight );
	if (sendCommand)
	{
		m_ClipTop = top;
		m_ClipLeft = left;
		m_ClipBot = bottom;
		m_ClipRight = right;

		drgDrawCommandBufferManager::SetClipRect(top, bottom, left, right, m_CommandBufferListID);
	}
}

void drgRenderCache::SetViewport(float top, float bottom, float left, float right)
{
	bool sendCommand = (top != m_ViewportTop || bottom != m_ViewportBottom || left != m_ViewportLeft || right != m_ViewportRight);

	if (sendCommand)
	{
		m_ViewportTop = top;
		m_ViewportBottom = bottom;
		m_ViewportLeft = left;
		m_ViewportRight = right;

		drgDrawCommandBufferManager::SetViewport((short)top, (short)bottom, (short)left, (short)right, m_CommandBufferListID);
	}
}

void drgRenderCache::SetColorBlend(bool enable, DRG_BLEND_FUNCTION sourceBlend, DRG_BLEND_FUNCTION destBlend, DRG_BLEND_OPERATION operation)
{
	bool enableChanged = enable != m_ColorBlendEnabled;
	bool sourceBlendChanged = sourceBlend != m_SourceColorBlend;
	bool destBlendChanged = destBlend != m_DestColorBlend;
	bool operationChanged = operation != m_ColorOperation;
	bool allChanged = enableChanged && sourceBlendChanged && destBlendChanged && operationChanged;

	if (allChanged)
	{
		m_ColorBlendEnabled = enable;
		m_SourceColorBlend = sourceBlend;
		m_DestColorBlend = destBlend;
		m_ColorOperation = operation;

		drgDrawCommandBufferManager::SetColorBlend(enable, sourceBlend, destBlend, operation, m_CommandBufferListID);
	}
	else
	{
		EnableColorBlend(enable);
		SetSourceColorBlend(sourceBlend);
		SetDestColorBlend(destBlend);
		SetColorBlendOperation(operation);
	}
}

void drgRenderCache::SetAlphaBlend(bool enable, DRG_BLEND_FUNCTION sourceBlend, DRG_BLEND_FUNCTION destBlend, DRG_BLEND_OPERATION operation)
{
	bool enableChanged = enable != m_AlphaBlendEnabled;
	bool sourceBlendChanged = sourceBlend != m_SourceAlphaBlend;
	bool destBlendChanged = destBlend != m_DestAlphaBlend;
	bool operationChanged = operation != m_AlphaOperation;
	bool allChanged = enableChanged && sourceBlendChanged && destBlendChanged && operationChanged;

	if (allChanged)
	{
		m_AlphaBlendEnabled = enable;
		m_SourceAlphaBlend = sourceBlend;
		m_DestAlphaBlend = destBlend;
		m_AlphaOperation = operation;

		drgDrawCommandBufferManager::SetAlphaBlend(enable, sourceBlend, destBlend, operation, m_CommandBufferListID);
	}
	else
	{
		EnableAlphaBlend(enable);
		SetSourceAlphaBlend(sourceBlend);
		SetDestAlphaBlend(destBlend);
		SetAlphaBlendOperation(operation);
	}
}

void drgRenderCache::EnableColorBlend(bool enable)
{
	if (enable != m_ColorBlendEnabled)
	{
		m_ColorBlendEnabled = enable;
		drgDrawCommandBufferManager::EnableColorBlend(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::SetSourceColorBlend(DRG_BLEND_FUNCTION func)
{
	if (func != m_SourceColorBlend)
	{
		m_SourceColorBlend = func;
		drgDrawCommandBufferManager::SetSourceColorBlend(func, m_CommandBufferListID);
	}
}

void drgRenderCache::SetDestColorBlend(DRG_BLEND_FUNCTION func)
{
	if (func != m_DestColorBlend)
	{
		m_DestColorBlend = func;
		drgDrawCommandBufferManager::SetDestColorBlend(func, m_CommandBufferListID);
	}
}

void drgRenderCache::SetColorBlendOperation(DRG_BLEND_OPERATION operation)
{
	if (operation != m_ColorOperation)
	{
		m_ColorOperation = operation;
		drgDrawCommandBufferManager::SetColorBlendOperation(operation, m_CommandBufferListID);
	}
}

void drgRenderCache::EnableAlphaBlend(bool enable)
{
	if (enable != m_AlphaBlendEnabled)
	{
		m_AlphaBlendEnabled = enable;
		drgDrawCommandBufferManager::EnableAlphaBlend(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::SetSourceAlphaBlend(DRG_BLEND_FUNCTION func)
{
	if (func != m_SourceAlphaBlend)
	{
		m_SourceAlphaBlend = func;
		drgDrawCommandBufferManager::SetSourceAlphaBlend(func, m_CommandBufferListID);
	}
}

void drgRenderCache::SetDestAlphaBlend(DRG_BLEND_FUNCTION func)
{
	if (func != m_DestAlphaBlend)
	{
		m_DestAlphaBlend = func;
		drgDrawCommandBufferManager::SetDestAlphaBlend(func, m_CommandBufferListID);
	}
}

void drgRenderCache::SetAlphaBlendOperation(DRG_BLEND_OPERATION operation)
{
	if (operation != m_AlphaOperation)
	{
		m_AlphaOperation = operation;
		drgDrawCommandBufferManager::SetAlphaBlendOperation(operation, m_CommandBufferListID);
	}
}

void drgRenderCache::SetDepthState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func)
{
	bool enableTestChanged = enableTest != m_DepthTestEnabled;
	bool enableWriteChanged = enableWrite != m_DepthWriteEnabled;
	bool funcChanged = func != m_DepthCompareFunc;
	bool allChanged = enableTestChanged && enableWriteChanged && funcChanged;

	if (allChanged)
	{
		m_DepthTestEnabled = enableTest;
		m_DepthWriteEnabled = enableWrite;
		m_DepthCompareFunc = func;

		drgDrawCommandBufferManager::SetDepthState(enableTest, enableWrite, func, m_CommandBufferListID);
	}
	else
	{
		EnableDepthTesting(enableTest);
		EnableDepthWriting(enableWrite);
		SetDepthCompareFunc(func);
	}
}

void drgRenderCache::SetStencilState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask)
{
	bool enableTestChanged = enableTest != m_StencilTestEnabled;
	bool enableWriteChanged = enableWrite != m_StencilWriteEnabled;
	bool funcChanged = func != m_StencilCompareFunc;
	bool refChanged = ref != m_StencilRef;
	bool maskChanged = ref != m_StencilMask;
	bool allChanged = enableTestChanged && enableWriteChanged && funcChanged && refChanged && maskChanged;

	if (allChanged)
	{
		m_StencilTestEnabled = enableTest;
		m_StencilWriteEnabled = enableWrite;
		m_StencilCompareFunc = func;
		m_StencilRef = ref;
		m_StencilMask = mask;

		drgDrawCommandBufferManager::SetStencilState(enableTest, enableWrite, func, ref, mask, m_CommandBufferListID);
	}
	else
	{
		EnableStencilTesting(enableTest);
		EnableStencilWriting(enableWrite);
		SetStencilCompareFunc(func);
	}
}

void drgRenderCache::EnableDepthTesting(bool enable)
{
	if (enable != m_DepthTestEnabled)
	{
		m_DepthTestEnabled = enable;
		drgDrawCommandBufferManager::EnableDepthTest(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::EnableDepthWriting(bool enable)
{
	if (enable != m_DepthWriteEnabled)
	{
		m_DepthWriteEnabled = enable;
		drgDrawCommandBufferManager::EnableDepthWrite(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::SetDepthCompareFunc(DRG_COMPARE_FUNC func)
{
	if (func != m_DepthCompareFunc)
	{
		m_DepthCompareFunc = func;
		drgDrawCommandBufferManager::SetDepthCompareFunc(func, m_CommandBufferListID);
	}
}

void drgRenderCache::EnableStencilTesting(bool enable)
{
	if (enable != m_StencilTestEnabled)
	{
		m_StencilTestEnabled = enable;
		drgDrawCommandBufferManager::EnableStencilTest(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::EnableStencilWriting(bool enable)
{
	if (enable != m_StencilWriteEnabled)
	{
		m_StencilWriteEnabled = enable;
		drgDrawCommandBufferManager::EnableStencilWrite(enable, m_CommandBufferListID);
	}
}

void drgRenderCache::SetStencilCompareFunc(DRG_COMPARE_FUNC func)
{
	if (func != m_StencilCompareFunc)
	{
		m_StencilCompareFunc = func;
		drgDrawCommandBufferManager::SetStencilCompareFunc(func, m_CommandBufferListID);
	}
}

void drgRenderCache::SetStencilRef(int ref)
{
	if (ref != m_StencilRef)
	{
		m_StencilRef = ref;
		drgDrawCommandBufferManager::SetStencilRef(ref, m_CommandBufferListID);
	}
}

void drgRenderCache::SetStencilMask(int mask)
{
	if (mask != m_StencilMask)
	{
		m_StencilMask = mask;
		drgDrawCommandBufferManager::SetStencilMask(mask, m_CommandBufferListID);
	}
}

void drgRenderCache::SetFrontFace(DRG_WINDING_ORDER order)
{
	if (order != m_FrontFace)
	{
		m_FrontFace = order;
		drgDrawCommandBufferManager::SetFrontFace(m_FrontFace, m_CommandBufferListID);
	}
}

void drgRenderCache::SetCullMode(DRG_CULL_MODE mode)
{
	if (mode != m_CullMode)
	{
		m_CullMode = mode;
		drgDrawCommandBufferManager::SetCullMode(mode, m_CommandBufferListID);
	}
}

void drgRenderCache::SetFillMode(DRG_FILL_MODE mode)
{
	if (mode != m_FillMode)
	{
		m_FillMode = mode;
		drgDrawCommandBufferManager::SetFillMode(mode, m_CommandBufferListID);
	}
}

void drgRenderCache::SetRenderTarget(drgRenderTarget *target, int index)
{
	if (target)
	{
		assert(target->IsDepthStencil() == false);
	}

	assert(index >= 0 && index < DRG_MAX_RENDER_TARGETS);

	if (target != m_RenderTargetsBound[index])
	{
		if (m_RenderTargetsBound[index])
		{
			drgDrawCommandBufferManager::UnbindRenderTarget(m_RenderTargetsBound[index], m_CommandBufferListID);
		}

		m_RenderTargetsBound[index] = target;
		drgDrawCommandBufferManager::BindRenderTarget(target, index, m_CommandBufferListID);
	}
}

void drgRenderCache::SetDepthTarget(drgRenderTarget *target)
{
	if (target)
	{
		assert(target->IsDepthStencil());
	}

	if (target != m_DepthTargetBound)
	{
		if (m_DepthTargetBound)
		{
			drgDrawCommandBufferManager::UnbindRenderTarget(m_DepthTargetBound, m_CommandBufferListID);
		}

		m_DepthTargetBound = target;
		drgDrawCommandBufferManager::BindDepthTarget(target, m_CommandBufferListID);
	}
}