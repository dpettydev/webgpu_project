
#include "render/draw_command_buffer_manager.h"
#include "render/dynamic_draw_command_buffer.h"
#include "render/render_cache.h"
#include "core/memmgr.h"

drgDrawCommandBuffer *drgDrawCommandBufferManager::m_CommandBuffers[DRG_MAX_COMMAND_BUFFERS];
drgRenderCache *drgDrawCommandBufferManager::m_CommandCaches[DRG_MAX_COMMAND_BUFFERS];

void drgDrawCommandBufferManager::Init()
{
	for (int currentCommandBuffer = 0; currentCommandBuffer < DRG_MAX_COMMAND_BUFFERS; ++currentCommandBuffer)
	{
		m_CommandBuffers[currentCommandBuffer] = NULL;
	}
}

void drgDrawCommandBufferManager::CreateCommandBuffer(int bufferID, drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type, bool createCache)
{
	assert(bufferID >= 0 && bufferID < DRG_MAX_COMMAND_BUFFERS);
	assert(m_CommandBuffers[bufferID] == NULL && "Tried to create an existing command buffer");

	if (createCache)
	{
		MEM_INFO_SET_NEW;
		m_CommandCaches[bufferID] = new drgRenderCache(bufferID);
		m_CommandCaches[bufferID]->SetupCache();
	}

	switch (type)
	{
	case DRG_COMMAND_BUFFER_DEFAULT:
		MEM_INFO_SET_NEW;
		m_CommandBuffers[bufferID] = new drgDrawCommandBuffer(context, type, m_CommandCaches[bufferID]);
		m_CommandBuffers[bufferID]->Init();
		break;

	case DRG_COMMAND_BUFFER_DYNAMIC:
		MEM_INFO_SET_NEW;
		m_CommandBuffers[bufferID] = new drgDynamicDrawCommandBuffer(context, type, m_CommandCaches[bufferID]);
		m_CommandBuffers[bufferID]->Init();
		break;

	default:
		drgPrintWarning("Tried to create an invalid command buffer type");
		return;
	}

	m_CommandBuffers[bufferID]->m_ListID = bufferID;
}

unsigned int drgDrawCommandBufferManager::CreateCommandBuffer(drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type, bool createCache)
{
	unsigned int listID = GetNextUnusedListID();
	if (listID == DRG_INVALID_COMMAND_ID)
	{
		return DRG_INVALID_COMMAND_ID;
	}

	CreateCommandBuffer(listID, context, type, createCache);
	return listID;
}

void drgDrawCommandBufferManager::RemoveCommandBuffer(int bufferID)
{
	assert(bufferID >= 0 && bufferID < DRG_MAX_COMMAND_BUFFERS);

	if (m_CommandCaches[bufferID] != NULL)
	{
		delete m_CommandCaches[bufferID];
		m_CommandCaches[bufferID] = NULL;
	}

	if (m_CommandBuffers[bufferID] != NULL)
	{
		delete m_CommandBuffers[bufferID];
		m_CommandBuffers[bufferID] = NULL;
	}
}

unsigned int drgDrawCommandBufferManager::GetNextUnusedListID()
{
	for (int currentListID = 0; currentListID < DRG_MAX_COMMAND_BUFFERS; ++currentListID)
	{
		if (m_CommandBuffers[currentListID] == NULL)
		{
			return currentListID;
		}
	}

	drgPrintWarning("No unused list IDs");
	return DRG_INVALID_COMMAND_ID;
}

void drgDrawCommandBufferManager::ClearTarget(unsigned clearFlags, drgColor clearColor, float depth, unsigned int stencil, int bufferID)
{
	drgClearTargetNode node(clearFlags, clearColor, depth, stencil);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_CLEAR_TARGET, &node, sizeof(drgClearTargetNode));
}

void drgDrawCommandBufferManager::SwapBuffers(GUI_Window *window, int bufferID)
{
	drgSwapBuffersNode node(window);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_SWAP_BUFFERS, &node, sizeof(drgSwapBuffersNode));
}

void drgDrawCommandBufferManager::DrawMesh(drgVertBuffer *vertBuffer, drgMaterial *material, int startPrimitive, int numPrimitives, bool setup, bool useEffect, int bufferID)
{
	drgMeshNode node(material, vertBuffer, startPrimitive, numPrimitives, setup, useEffect);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_MESH, &node, sizeof(drgMeshNode));
}

void drgDrawCommandBufferManager::DrawModel(drgModel *model, int bufferID)
{
	// drgModelNode node(model);

	// drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	// assert("Failed to find the command buffer specified");

	// buffer->AddNode(DRG_DRAW_COMMAND_MODEL, &node, sizeof(drgModelNode));
}

// void drgDrawCommandBufferManager::DrawScene(GUI_SceneViewer *scene, int bufferID)
// {
// 	// drgSceneNode node(scene);

// 	// drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
// 	// assert("Failed to find the command buffer specified");

// 	// buffer->AddNode(DRG_DRAW_COMMAND_SCENE, &node, sizeof(drgSceneNode));
// }

void drgDrawCommandBufferManager::MakeCurrent(void *renderContext, void *deviceContext, int bufferID)
{
	drgMakeCurrentNode node(renderContext, deviceContext);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_MAKE_CURRENT, &node, sizeof(drgMakeCurrentNode));
}

void drgDrawCommandBufferManager::SetUniform(void *uniformStructure, unsigned int uniformSize, DRG_DRAW_COMMAND_UNIFORM_TYPE type, int bufferID)
{
	// First we get the size the uniform should be.
	unsigned int dataSize = 0;
	switch (type)
	{
	case DRG_UNIFORM_AMBIENT_COLOR:
	case DRG_UNIFORM_DIFFUSE:
	case DRG_UNIFORM_SPECULAR:
	{
		dataSize = sizeof(drgColorUniform);
		break;
	}

	case DRG_UNIFORM_VIEW_DIRECTION:
	case DRG_UNIFORM_VIEW_POSITION:
	{
		dataSize = sizeof(drgVector3Uniform);
		break;
	}

	case DRG_UNIFORM_VIEW_PROJECTION:
	case DRG_UNIFORM_WORLD:
	{
		dataSize = sizeof(drgMatrixUniform);
		break;
	}

	case DRG_UNIFORM_BONE_MATRIX:
	{
		dataSize = sizeof(drgBoneMatrixUniform);
		break;
	}

	default:
		assert("Tried to set an unknown uniform type");
	}

	// Adding in a special parameter to verify they are sending in the right data.
	assert(dataSize == uniformSize);

	// Then we add the node.
	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddUniformNode(DRG_DRAW_COMMAND_UNIFORM, type, uniformStructure, uniformSize);
}

void drgDrawCommandBufferManager::EnableScissor(bool enable, int bufferID)
{
	drgEnableScissorNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_SCISSOR, &node, sizeof(drgEnableScissorNode));
}

void drgDrawCommandBufferManager::SetScissorState(bool enable, short top, short bottom, short left, short right, int bufferID)
{
	// drgScissorStateNode node(enable, top, bottom, left, right);

	// drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	// assert("Failed to find the command buffer specified");

	// buffer->AddNode(DRG_DRAW_COMMAND_SCISSOR_STATE, &node, sizeof(drgScissorStateNode));
}

void drgDrawCommandBufferManager::SetClipRect(short top, short bottom, short left, short right, int bufferID)
{
	// drgClipRectNode clipRectNode(top, bottom, left, right);

	// drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	// assert("Failed to find the command buffer specified");

	// buffer->AddNode(DRG_DRAW_COMMAND_CLIP_RECT, &clipRectNode, sizeof(drgClipRectNode));
}

void drgDrawCommandBufferManager::SetViewport(short top, short bottom, short left, short right, int bufferID)
{
	drgViewportNode node(top, bottom, left, right);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_VIEWPORT, &node, sizeof(drgViewportNode));
}

void drgDrawCommandBufferManager::SetColorBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op, int bufferID)
{
	drgColorBlendNode node(enable, source, dest, op);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_COLOR_BLEND, &node, sizeof(drgColorBlendNode));
}

void drgDrawCommandBufferManager::SetAlphaBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op, int bufferID)
{
	drgAlphaBlendNode node(enable, source, dest, op);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ALPHA_BLEND, &node, sizeof(drgAlphaBlendNode));
}

void drgDrawCommandBufferManager::EnableColorBlend(bool enable, int bufferID)
{
	drgEnableColorBlendNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_COLOR_BLEND, &node, sizeof(drgEnableColorBlendNode));
}

void drgDrawCommandBufferManager::SetSourceColorBlend(DRG_BLEND_FUNCTION func, int bufferID)
{
	drgSourceColorBlendNode node(func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_COLOR_BLEND_SOURCE, &node, sizeof(drgSourceColorBlendNode));
}

void drgDrawCommandBufferManager::SetDestColorBlend(DRG_BLEND_FUNCTION func, int bufferID)
{
	drgDestColorBlendNode node(func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_COLOR_BLEND_DEST, &node, sizeof(drgDestColorBlendNode));
}

void drgDrawCommandBufferManager::SetColorBlendOperation(DRG_BLEND_OPERATION op, int bufferID)
{
	drgColorBlendOperationNode node(op);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_COLOR_BLEND_OPERATION, &node, sizeof(drgColorBlendOperationNode));
}

void drgDrawCommandBufferManager::EnableAlphaBlend(bool enable, int bufferID)
{
	drgEnableAlphaBlendNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_ALPHA_BLEND, &node, sizeof(drgEnableAlphaBlendNode));
}

void drgDrawCommandBufferManager::SetSourceAlphaBlend(DRG_BLEND_FUNCTION func, int bufferID)
{
	drgSourceAlphaBlendNode node(func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ALPHA_BLEND_SOURCE, &node, sizeof(drgSourceAlphaBlendNode));
}

void drgDrawCommandBufferManager::SetDestAlphaBlend(DRG_BLEND_FUNCTION func, int bufferID)
{
	drgDestAlphaBlendNode node(func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ALPHA_BLEND_DEST, &node, sizeof(drgDestAlphaBlendNode));
}

void drgDrawCommandBufferManager::SetAlphaBlendOperation(DRG_BLEND_OPERATION op, int bufferID)
{
	drgAlphaBlendOperationNode node(op);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ALPHA_BLEND_OPERATION, &node, sizeof(drgAlphaBlendOperationNode));
}

void drgDrawCommandBufferManager::SetDepthState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int bufferID)
{
	drgDepthStateNode node(enableTest, enableWrite, func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_DEPTH_STATE, &node, sizeof(drgDepthStateNode));
}

void drgDrawCommandBufferManager::SetStencilState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask, int bufferID)
{
	drgStencilStateNode node(enableTest, enableWrite, func, ref, mask);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_STENCIL_STATE, &node, sizeof(drgStencilStateNode));
}

void drgDrawCommandBufferManager::EnableDepthTest(bool enable, int bufferID)
{
	drgDepthTestEnableNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_DEPTH_TEST, &node, sizeof(drgDepthTestEnableNode));
}

void drgDrawCommandBufferManager::EnableDepthWrite(bool enable, int bufferID)
{
	drgDepthWriteEnableNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_DEPTH_WRITE, &node, sizeof(drgDepthWriteEnableNode));
}

void drgDrawCommandBufferManager::SetDepthCompareFunc(DRG_COMPARE_FUNC func, int bufferID)
{
	drgDepthCompareFuncNode node(func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_DEPTH_COMPARE_FUNC, &node, sizeof(drgDepthCompareFuncNode));
}

void drgDrawCommandBufferManager::EnableStencilTest(bool enable, int bufferID)
{
	drgStencilTestEnableNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_STENCIL_TEST, &node, sizeof(drgStencilTestEnableNode));
}

void drgDrawCommandBufferManager::EnableStencilWrite(bool enable, int bufferID)
{
	drgStencilWriteEnableNode node(enable);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_ENABLE_STENCIL_WRITE, &node, sizeof(drgStencilWriteEnableNode));
}

void drgDrawCommandBufferManager::SetStencilCompareFunc(DRG_COMPARE_FUNC func, int bufferID)
{
	drgStencilCompareFuncNode node(func);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_STENCIL_COMPARE_FUNC, &node, sizeof(drgStencilCompareFuncNode));
}

void drgDrawCommandBufferManager::SetStencilRef(int ref, int bufferID)
{
	drgStencilRefNode node(ref);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_STENCIL_REF, &node, sizeof(drgStencilRefNode));
}

void drgDrawCommandBufferManager::SetStencilMask(int mask, int bufferID)
{
	drgStencilMaskNode node(mask);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_STENCIL_MASK, &node, sizeof(drgStencilMaskNode));
}

void drgDrawCommandBufferManager::SetCullMode(DRG_CULL_MODE mode, int bufferID)
{
	drgCullNode node(mode);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_CULL_MODE, &node, sizeof(drgCullNode));
}

void drgDrawCommandBufferManager::SetFrontFace(DRG_WINDING_ORDER order, int bufferID)
{
	drgFrontFaceNode node(order);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_FRONT_FACE, &node, sizeof(drgFrontFaceNode));
}

void drgDrawCommandBufferManager::SetFillMode(DRG_FILL_MODE mode, int bufferID)
{
	drgFillNode node(mode);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_FILL_MODE, &node, sizeof(drgFillNode));
}

void drgDrawCommandBufferManager::BindRenderTarget(drgRenderTarget *target, int index, int bufferID)
{
	drgBindRenderTarget node(target, index);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_BIND_RENDER_TARGET, &node, sizeof(drgBindRenderTarget));
}

void drgDrawCommandBufferManager::BindDepthTarget(drgRenderTarget *target, int bufferID)
{
	drgBindRenderTarget node(target, -1);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_BIND_RENDER_TARGET, &node, sizeof(drgBindRenderTarget));
}

void drgDrawCommandBufferManager::UnbindRenderTarget(drgRenderTarget *target, int bufferID)
{
	drgUnbindRenderTarget node(target);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_UNBIND_RENDER_TARGET, &node, sizeof(drgUnbindRenderTarget));
}

void drgDrawCommandBufferManager::UpdateSurfaces(int bufferID)
{
	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");

	buffer->AddNode(DRG_DRAW_COMMAND_UPDATE_SURFACES, NULL, 0);
}

void drgDrawCommandBufferManager::RunCommandList(drgDrawCommandBuffer *list, int bufferID, bool flush)
{
	drgRunCommandBuffer node(list, flush);

	drgDrawCommandBuffer *buffer = GetCommandBuffer(bufferID);
	assert("Failed to find the command buffer specified");
	assert(list != buffer && "Can't run a command list from itself.");

	buffer->AddNode(DRG_DRAW_COMMAND_RUN_COMMAND_LIST, &node, sizeof(drgRunCommandBuffer));
}

drgDrawCommandBuffer *drgDrawCommandBufferManager::GetCommandBuffer(int bufferID)
{
	assert(bufferID >= 0 && bufferID < DRG_MAX_COMMAND_BUFFERS);
	return m_CommandBuffers[bufferID];
}

drgRenderCache *drgDrawCommandBufferManager::GetCommandBufferRenderCache(int bufferID)
{
	assert(bufferID >= 0 && bufferID < DRG_MAX_COMMAND_BUFFERS);
	return m_CommandCaches[bufferID];
}

void drgDrawCommandBufferManager::Clear()
{
	for (int currentBuffer = 0; currentBuffer < DRG_MAX_COMMAND_BUFFERS; ++currentBuffer)
	{
		if (m_CommandBuffers[currentBuffer] != NULL)
		{
			m_CommandBuffers[currentBuffer]->Clear();
		}
	}
}

void drgDrawCommandBufferManager::ResetAllRenderCaches()
{
	for (int currentBuffer = 0; currentBuffer < DRG_MAX_COMMAND_BUFFERS; ++currentBuffer)
	{
		if (m_CommandBuffers[currentBuffer] != NULL && m_CommandBuffers[currentBuffer]->GetRenderCache())
		{
			m_CommandBuffers[currentBuffer]->GetRenderCache()->SetupCache();
		}
	}
}