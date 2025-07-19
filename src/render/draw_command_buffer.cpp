// Includes
#include "draw_command_buffer.h"
#include "system/debug.h"
#include "core/memmgr.h"
#include "render/context.h"
#include "render/render_cache.h"

// Defines
drgDrawCommandBuffer::drgDrawCommandBuffer(drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type, drgRenderCache *cache)
{
	m_Context = context;
	m_CommandBufferType = type;
	m_ListID = -1;
	m_Cache = cache;

	assert(m_Context);
}

drgDrawCommandBuffer::~drgDrawCommandBuffer()
{
	Destroy();
}

void drgDrawCommandBuffer::Init()
{
	m_CurrentDataOffset = 0;

	for (int currentDrawBuffer = 0; currentDrawBuffer < DRG_NUM_DRAW_BUFFERS; ++currentDrawBuffer)
	{
		m_NumCommands[currentDrawBuffer] = 0;
		m_DataBuffer[currentDrawBuffer] = (unsigned char *)drgMemAlign(128, DRG_MAX_COMMAND_SIZE);
	}
}

void drgDrawCommandBuffer::Destroy()
{
	for (int currentDrawBuffer = 0; currentDrawBuffer < DRG_NUM_DRAW_BUFFERS; ++currentDrawBuffer)
	{
		drgMemFree(m_DataBuffer[currentDrawBuffer]);
	}
}

void drgDrawCommandBuffer::Begin()
{
	m_NumCommands[m_CurrentDrawBuffer] = 0;
	m_CurrentDataOffset = 0;

	assert(m_DataBuffer[m_CurrentDrawBuffer] && "You're attempting to use a command buffer that has not been initialized");
}

void drgDrawCommandBuffer::End()
{
}

void drgDrawCommandBuffer::Draw(bool flush)
{
	// End our list
	if (flush)
	{
		End();
	}

	// Walk the bucket list, and process the commands appropriately
	ProcessCommands();
	if (flush)
	{
		// Swap draw buffers. TODO: Make this a function so we can handle more than two buffers if necessary
		m_CurrentDrawBuffer = !m_CurrentDrawBuffer;
		// Make sure we reset again.
		Begin();
	}
}

void drgDrawCommandBuffer::Clear()
{
	for (int currentDrawBuffer = 0; currentDrawBuffer < DRG_NUM_DRAW_BUFFERS; ++currentDrawBuffer)
	{
		m_NumCommands[currentDrawBuffer] = 0;
	}
	Begin();
}

void drgDrawCommandBuffer::AddNode(DRG_DRAW_COMMAND_TYPE type, void *data, int dataSize)
{
	++m_NumCommands[m_CurrentDrawBuffer];
	assert(m_NumCommands[m_CurrentDrawBuffer] < DRG_MAX_COMMANDS);
	assert(m_CurrentDataOffset < DRG_MAX_COMMAND_SIZE);

	if (m_NumCommands[m_CurrentDrawBuffer] >= DRG_MAX_COMMANDS)
	{
		drgDebug::Error("Tried to add a node that exceeds the maximum amount of commands");
	}

	if (m_CurrentDataOffset >= DRG_MAX_COMMAND_SIZE)
	{
		drgDebug::Error("Command buffer is full!");
	}

	unsigned char *dataStartOffset = m_DataBuffer[m_CurrentDrawBuffer] + m_CurrentDataOffset;
	assert(dataStartOffset);

	// First we add the command type to prefix our data
	drgMemory::MemCopy(dataStartOffset, &type, sizeof(type));
	dataStartOffset += sizeof(DRG_DRAW_COMMAND_TYPE);

	// Next we copy in the actual data
	if (data)
	{
		drgMemory::MemCopy(dataStartOffset, data, dataSize);
	}

	// Update our current offset into the data buffer
	m_CurrentDataOffset += dataSize + sizeof(type);
}

void drgDrawCommandBuffer::AddUniformNode(DRG_DRAW_COMMAND_TYPE type, DRG_DRAW_COMMAND_UNIFORM_TYPE uniformType, void *data, int dataSize)
{
	++m_NumCommands[m_CurrentDrawBuffer];
	assert(m_NumCommands[m_CurrentDrawBuffer] < DRG_MAX_COMMANDS);
	assert(m_CurrentDataOffset < DRG_MAX_COMMAND_SIZE);

	if (m_NumCommands[m_CurrentDrawBuffer] >= DRG_MAX_COMMANDS)
	{
		drgDebug::Error("Tried to add a node that exceeds the maximum amount of commands");
	}

	if (m_CurrentDataOffset >= DRG_MAX_COMMAND_SIZE)
	{
		drgDebug::Error("Command buffer is full!");
	}

	unsigned char *dataStartOffset = m_DataBuffer[m_CurrentDrawBuffer] + m_CurrentDataOffset;

	// First we add the command type to prefix our data
	drgMemory::MemCopy(dataStartOffset, &type, sizeof(type));
	dataStartOffset += sizeof(DRG_DRAW_COMMAND_TYPE);

	// Next we add the uniform type
	drgMemory::MemCopy(dataStartOffset, &uniformType, sizeof(DRG_DRAW_COMMAND_UNIFORM_TYPE));
	dataStartOffset += sizeof(DRG_DRAW_COMMAND_UNIFORM_TYPE);

	// Next we copy in the actual data
	drgMemory::MemCopy(dataStartOffset, data, dataSize);

	// Update our current offset into the data buffer
	m_CurrentDataOffset += dataSize + sizeof(type) + sizeof(uniformType);
}

void drgDrawCommandBuffer::ProcessCommands()
{
	// drgPrintOut( "Processing commands: %i", m_NumCommands[ m_CurrentDrawBuffer ] );
	int currentOffset = 0;
	for (int currentDrawCommand = 0; currentDrawCommand < m_NumCommands[m_CurrentDrawBuffer]; ++currentDrawCommand)
	{
		DRG_DRAW_COMMAND_TYPE command = *(DRG_DRAW_COMMAND_TYPE *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(command);

		DispatchCommand(command, currentOffset);
	}
}

void drgDrawCommandBuffer::DispatchCommand(DRG_DRAW_COMMAND_TYPE command, int &currentOffset)
{
	switch (command)
	{
	case DRG_DRAW_COMMAND_CLEAR_TARGET:
		ProcessClear(currentOffset);
		break;

	case DRG_DRAW_COMMAND_SWAP_BUFFERS:
		ProcessSwapBuffers(currentOffset);
		break;

	case DRG_DRAW_COMMAND_MESH:
		ProcessDrawMesh(currentOffset);
		break;

		// 		case DRG_DRAW_COMMAND_SCENE:
		// 			ProcessDrawScene(currentOffset);
		// 			break;

		// 		case DRG_DRAW_COMMAND_MODEL:
		// 			ProcessDrawModel( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_CLIP_RECT:
		// 			ProcessClipRect( currentOffset );
		// 			break;

	case DRG_DRAW_COMMAND_ENABLE_SCISSOR:
		ProcessEnableScissor(currentOffset);
		break;

		// 		case DRG_DRAW_COMMAND_SCISSOR_STATE:
		// 			ProcessScissorState( currentOffset );
		// 			break;

	case DRG_DRAW_COMMAND_VIEWPORT:
		ProcessViewport(currentOffset);
		break;

	case DRG_DRAW_COMMAND_COLOR_BLEND:
		ProcessColorBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ALPHA_BLEND:
		ProcessAlphaBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ENABLE_COLOR_BLEND:
		ProcessEnableColorBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_COLOR_BLEND_SOURCE:
		ProcessColorSourceBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_COLOR_BLEND_DEST:
		ProcessColorDestBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_COLOR_BLEND_OPERATION:
		ProcessColorOperation(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ENABLE_ALPHA_BLEND:
		ProcessEnableAlphaBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ALPHA_BLEND_SOURCE:
		ProcessAlphaSourceBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ALPHA_BLEND_DEST:
		ProcessAlphaDestBlend(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ALPHA_BLEND_OPERATION:
		ProcessAlphaOperation(currentOffset);
		break;

	case DRG_DRAW_COMMAND_DEPTH_STATE:
		ProcessDepthState(currentOffset);
		break;

	case DRG_DRAW_COMMAND_STENCIL_STATE:
		ProcessStencilState(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ENABLE_DEPTH_TEST:
		ProcessEnableDepthTest(currentOffset);
		break;

	case DRG_DRAW_COMMAND_ENABLE_DEPTH_WRITE:
		ProcessEnableDepthWrite(currentOffset);
		break;

	case DRG_DRAW_COMMAND_DEPTH_COMPARE_FUNC:
		ProcessDepthCompareFunc(currentOffset);
		break;

		// 		case DRG_DRAW_COMMAND_ENABLE_STENCIL_TEST:
		// 			ProcessEnableStencilTest( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_ENABLE_STENCIL_WRITE:
		// 			ProcessEnableStencilWrite( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_STENCIL_COMPARE_FUNC:
		// 			ProcessStencilCompareFunc( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_STENCIL_REF:
		// 			ProcessStencilRef( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_STENCIL_MASK:
		// 			ProcessStencilMask( currentOffset );
		// 			break;

	case DRG_DRAW_COMMAND_CULL_MODE:
		ProcessCullMode(currentOffset);
		break;

	case DRG_DRAW_COMMAND_FRONT_FACE:
		ProcessFrontFace(currentOffset);
		break;

		// 		case DRG_DRAW_COMMAND_FILL_MODE:
		// 			ProcessFillMode( currentOffset );
		// 			break;

	case DRG_DRAW_COMMAND_MAKE_CURRENT:
		ProcessMakeCurrent(currentOffset);
		break;

		// 		case DRG_DRAW_COMMAND_BIND_RENDER_TARGET:
		// 			ProcessBindRenderTarget( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_UNBIND_RENDER_TARGET:
		// 			ProcessUnbindRenderTarget( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_UPDATE_SURFACES:
		// 			ProcessUpdateSurfaces( currentOffset );
		// 			break;

		// 		case DRG_DRAW_COMMAND_RUN_COMMAND_LIST:
		// 			ProcessRunCommandList( currentOffset );
		// 			break;

	case DRG_DRAW_COMMAND_UNIFORM:
	{
		DRG_DRAW_COMMAND_UNIFORM_TYPE uniformType = (DRG_DRAW_COMMAND_UNIFORM_TYPE) * (m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(uniformType);

		ProcessUniformCommand(uniformType, currentOffset);
		break;
	}

	default:
		drgPrintWarning("Tried to process an unknown command\r\n");
		return;
	}
}

void drgDrawCommandBuffer::ProcessSwapBuffers(int &currentOffset)
{
	drgSwapBuffersNode *node = (drgSwapBuffersNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgSwapBuffersNode);

	m_Context->ApplySwapBuffers(node->m_Window);
}

void drgDrawCommandBuffer::ProcessClear(int &currentOffset)
{
	drgClearTargetNode *node = (drgClearTargetNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgClearTargetNode);

	m_Context->ApplyClearTarget(node->m_ClearFlags, node->m_ClearColor, node->m_DepthClear, node->m_StencilClear);
}

void drgDrawCommandBuffer::ProcessMakeCurrent(int &currentOffset)
{
	drgMakeCurrentNode *node = (drgMakeCurrentNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgMakeCurrentNode);

	m_Context->ApplyMakeCurrent(node->m_PlatformRenderContext, node->m_PlatformDeviceContext);
}

void drgDrawCommandBuffer::ProcessDrawMesh(int &currentOffset)
{
	// drgMeshNode* node = (drgMeshNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgMeshNode );

	// m_Context->ApplyDrawMesh( node->m_VertexBuffer,
	// 	node->m_Material,
	// 	node->m_StartPrimitive,
	// 	node->m_NumPrimitives,
	// 	node->m_SetupBuffer == 1 ? true : false,
	// 	node->m_UseEffect == 1 ? true : false );
}

void drgDrawCommandBuffer::ProcessDrawModel(int &currentOffset)
{
	// drgModelNode* node = (drgModelNode*)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	// currentOffset += sizeof(drgModelNode);

	// m_Context->ApplyDrawModel(node->m_Model);
}

void drgDrawCommandBuffer::ProcessDrawScene(int &currentOffset)
{
	// drgSceneNode* node = (drgSceneNode*)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	// currentOffset += sizeof(drgSceneNode);

	// m_Context->ApplyDrawScene(node->m_Scene);
}

void drgDrawCommandBuffer::ProcessClipRect(int &currentOffset)
{
	// drgClipRectNode* node = (drgClipRectNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgClipRectNode );

	// m_Context->ApplyClipRect( node->m_Top, node->m_Bottom, node->m_Left, node->m_Right );
}

void drgDrawCommandBuffer::ProcessEnableScissor(int &currentOffset)
{
	drgEnableScissorNode *node = (drgEnableScissorNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgEnableScissorNode);

	m_Context->ApplyScissorEnable(node->m_EnableScissor == 1 ? true : false);
}

void drgDrawCommandBuffer::ProcessScissorState(int &currentOffset)
{
	// drgScissorStateNode* node = (drgScissorStateNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgScissorStateNode );

	// m_Context->ApplyScissorState( node->m_EnableScissor == 1 ? true : false,
	// 	node->m_Top,
	// 	node->m_Bottom,
	// 	node->m_Left,
	// 	node->m_Right );
}

void drgDrawCommandBuffer::ProcessViewport(int &currentOffset)
{
	drgViewportNode *node = (drgViewportNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgViewportNode);

	m_Context->ApplyViewport(node->m_Top, node->m_Bottom, node->m_Left, node->m_Right);
}

void drgDrawCommandBuffer::ProcessColorBlend(int &currentOffset)
{
	drgColorBlendNode *node = (drgColorBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgColorBlendNode);

	m_Context->ApplyColorBlend(node->m_Enable == 1 ? true : false,
							   node->m_SourceColor,
							   node->m_DestColor,
							   node->m_BlendEquation);
}

void drgDrawCommandBuffer::ProcessAlphaBlend(int &currentOffset)
{
	drgAlphaBlendNode *node = (drgAlphaBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgAlphaBlendNode);

	m_Context->ApplyAlphaBlend(node->m_Enable == 1 ? true : false,
							   node->m_SourceAlpha,
							   node->m_DestAlpha,
							   node->m_BlendEquation);
}

void drgDrawCommandBuffer::ProcessEnableColorBlend(int &currentOffset)
{
	drgEnableColorBlendNode *node = (drgEnableColorBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgEnableColorBlendNode);

	m_Context->ApplyColorBlendEnable(node->m_Enable == 1 ? true : false);
}

void drgDrawCommandBuffer::ProcessColorSourceBlend(int &currentOffset)
{
	drgSourceColorBlendNode *node = (drgSourceColorBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgSourceColorBlendNode);

	m_Context->ApplyColorSourceBlend(node->m_SourceColorBlend);
}

void drgDrawCommandBuffer::ProcessColorDestBlend(int &currentOffset)
{
	drgDestColorBlendNode *node = (drgDestColorBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDestColorBlendNode);

	m_Context->ApplyColorDestBlend(node->m_DestColorBlend);
}

void drgDrawCommandBuffer::ProcessColorOperation(int &currentOffset)
{
	drgColorBlendOperationNode *node = (drgColorBlendOperationNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgColorBlendOperationNode);

	m_Context->ApplyColorOperation(node->m_Operation);
}

void drgDrawCommandBuffer::ProcessEnableAlphaBlend(int &currentOffset)
{
	drgEnableAlphaBlendNode *node = (drgEnableAlphaBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgEnableAlphaBlendNode);

	m_Context->ApplyAlphaBlendEnable(node->m_Enable == 1 ? true : false);
}

void drgDrawCommandBuffer::ProcessAlphaSourceBlend(int &currentOffset)
{
	drgSourceAlphaBlendNode *node = (drgSourceAlphaBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgSourceAlphaBlendNode);

	m_Context->ApplyAlphaSourceBlend(node->m_SourceAlphaBlend);
}

void drgDrawCommandBuffer::ProcessAlphaDestBlend(int &currentOffset)
{
	drgDestAlphaBlendNode *node = (drgDestAlphaBlendNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDestAlphaBlendNode);

	m_Context->ApplyAlphaDestBlend(node->m_DestAlphaBlend);
}

void drgDrawCommandBuffer::ProcessAlphaOperation(int &currentOffset)
{
	drgAlphaBlendOperationNode *node = (drgAlphaBlendOperationNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgAlphaBlendOperationNode);

	m_Context->ApplyAlphaOperation(node->m_Operation);
}

void drgDrawCommandBuffer::ProcessDepthState(int &currentOffset)
{
	drgDepthStateNode *node = (drgDepthStateNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDepthStateNode);

	m_Context->ApplyDepthState(node->m_EnableDepthTest == 1 ? true : false,
							   node->m_EnableDepthWrite == 1 ? true : false,
							   node->m_CompareFunc);
}

void drgDrawCommandBuffer::ProcessStencilState(int &currentOffset)
{
	drgStencilStateNode *node = (drgStencilStateNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgStencilStateNode);

	m_Context->ApplyStencilState(node->m_EnableStencilTest == 1 ? true : false,
								 node->m_EnableStencilWrite == 1 ? true : false,
								 node->m_CompareFunc,
								 node->m_StencilRef,
								 node->m_StencilMask);
}

void drgDrawCommandBuffer::ProcessEnableDepthTest(int &currentOffset)
{
	drgDepthTestEnableNode *node = (drgDepthTestEnableNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDepthTestEnableNode);

	m_Context->ApplyDepthTestEnable(node->m_Enable == 1 ? true : false);
}

void drgDrawCommandBuffer::ProcessEnableDepthWrite(int &currentOffset)
{
	drgDepthWriteEnableNode *node = (drgDepthWriteEnableNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDepthWriteEnableNode);

	m_Context->ApplyDepthWriteEnable(node->m_Enable == 1 ? true : false);
}

void drgDrawCommandBuffer::ProcessDepthCompareFunc(int &currentOffset)
{
	drgDepthCompareFuncNode *node = (drgDepthCompareFuncNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDepthCompareFuncNode);

	m_Context->ApplyDepthCompareFunc(node->m_CompareFunc);
}

void drgDrawCommandBuffer::ProcessEnableStencilTest(int &currentOffset)
{
	// drgStencilTestEnableNode* node = (drgStencilTestEnableNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgStencilTestEnableNode );

	// m_Context->ApplyStencilTestEnable( node->m_Enable == 1 ? true : false );
}

void drgDrawCommandBuffer::ProcessEnableStencilWrite(int &currentOffset)
{
	// drgStencilWriteEnableNode* node = (drgStencilWriteEnableNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgStencilWriteEnableNode );

	// m_Context->ApplyStencilWriteEnable( node->m_Enable == 1 ? true : false );
}

void drgDrawCommandBuffer::ProcessStencilCompareFunc(int &currentOffset)
{
	// drgStencilCompareFuncNode* node = (drgStencilCompareFuncNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgStencilCompareFuncNode );

	// m_Context->ApplyStencilCompareFunc( node->m_CompareFunc );
}

void drgDrawCommandBuffer::ProcessStencilRef(int &currentOffset)
{
	// drgStencilRefNode* node = (drgStencilRefNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgStencilRefNode );

	// m_Context->ApplyStencilRef( node->m_StencilRef );
}

void drgDrawCommandBuffer::ProcessStencilMask(int &currentOffset)
{
	// drgStencilMaskNode* node = (drgStencilMaskNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgStencilMaskNode );

	// m_Context->ApplyStencilMask( node->m_StencilMask );
}

void drgDrawCommandBuffer::ProcessCullMode(int &currentOffset)
{
	drgCullNode *node = (drgCullNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgCullNode);

	m_Context->ApplyCullMode(node->m_CullMode);
}

void drgDrawCommandBuffer::ProcessFrontFace(int &currentOffset)
{
	drgFrontFaceNode *node = (drgFrontFaceNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgFrontFaceNode);

	m_Context->ApplyFrontFace(node->m_WindingOrder);
}

void drgDrawCommandBuffer::ProcessFillMode(int &currentOffset)
{
	// drgFillNode* node = (drgFillNode*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgFillNode );

	// m_Context->ApplyFillMode( node->m_FillMode );
}

void drgDrawCommandBuffer::ProcessBindRenderTarget(int &currentOffset)
{
	// drgBindRenderTarget* node = (drgBindRenderTarget*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset );
	// currentOffset += sizeof( drgBindRenderTarget );

	// m_Context->ApplyBindRenderTarget( node->m_Target, node->m_Index );
}

void drgDrawCommandBuffer::ProcessUnbindRenderTarget(int &currentOffset)
{
	// drgUnbindRenderTarget* node = (drgUnbindRenderTarget*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgUnbindRenderTarget );

	// m_Context->ApplyUnbindRenderTarget( node->m_Target );
}

void drgDrawCommandBuffer::ProcessUpdateSurfaces(int &currentOffset)
{
	m_Context->ApplyUpdateSurfaces();
}

void drgDrawCommandBuffer::ProcessRunCommandList(int &currentOffset)
{
	// drgRunCommandBuffer* node = (drgRunCommandBuffer*)(m_DataBuffer[ m_CurrentDrawBuffer ] + currentOffset);
	// currentOffset += sizeof( drgRunCommandBuffer );

	//
	// Just run the list here, it should be entirely platform agnostic.
	//

	// node->m_CommandList->Draw( node->m_Flush == 1 ? true : false );
}

void drgDrawCommandBuffer::ProcessUniformCommand(DRG_DRAW_COMMAND_UNIFORM_TYPE uniformType, int &currentOffset)
{
	switch (uniformType)
	{
	case DRG_UNIFORM_AMBIENT_COLOR:
	{
		drgColorUniform *color = (drgColorUniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgColorUniform);
		m_Context->SetAmbientColor(color->m_Color.r, color->m_Color.g, color->m_Color.b, color->m_Color.a);
		break;
	}
	case DRG_UNIFORM_DIFFUSE:
	{
		drgColorUniform *color = (drgColorUniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgColorUniform);
		m_Context->SetDiffuseColor(&color->m_Color);
		break;
	}
	case DRG_UNIFORM_SPECULAR:
	{
		drgColorUniform *color = (drgColorUniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgColorUniform);
		m_Context->SetSpecular(&color->m_Color);
		break;
	}
	case DRG_UNIFORM_VIEW_POSITION:
	{
		drgVector3Uniform *vec = (drgVector3Uniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgVector3Uniform);
		m_Context->SetViewPosition(&vec->m_Vector);
		break;
	}
	case DRG_UNIFORM_VIEW_DIRECTION:
	{
		drgVector3Uniform *vec = (drgVector3Uniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgVector3Uniform);
		m_Context->SetViewDirection(&vec->m_Vector);
		break;
	}
	case DRG_UNIFORM_TEXCOORD_OFFSET:
	{
		drgVector4Uniform *vec = (drgVector4Uniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgVector4Uniform);
		m_Context->SetTexCoordOffset(&vec->m_Vector);
		break;
	}
	case DRG_UNIFORM_WORLD:
	{
		drgMatrixUniform *mat = (drgMatrixUniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgMatrixUniform);
		m_Context->SetWorldMatrix(&mat->m_Matrix);
		break;
	}
	// case DRG_UNIFORM_VIEW_PROJECTION:
	// {
	// 	drgViewProjectionUniform *mat = (drgWorldViewProjectionUniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	// 	currentOffset += sizeof(drgWorldViewProjectionUniform);
	// 	m_Context->SetWorldViewProjectionMatrix(&mat->m_World, &mat->m_ViewProjection);
	// 	break;
	// }
	case DRG_UNIFORM_BONE_MATRIX:
	{
		drgBoneMatrixUniform *bone = (drgBoneMatrixUniform *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
		currentOffset += sizeof(drgBoneMatrixUniform);

		m_Context->SetBoneMatrix(&bone->m_Matrix, bone->m_BoneIndex);
	}
	}
}