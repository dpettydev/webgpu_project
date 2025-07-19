#ifndef _DRG_DRAW_COMMAND_BUFFER_MANAGER_H_
#define _DRG_DRAW_COMMAND_BUFFER_MANAGER_H_

#include "render/draw_command_buffer.h"
#include "render/context.h"

#define DRG_MAX_COMMAND_BUFFERS 32
#define DRG_INVALID_COMMAND_ID 0xBAADF00D

class GUI_Window;
class drgRenderContext;
class drgModel;
class drgRenderTarget;
class drgRenderCache;

class drgDrawCommandBufferManager
{
public:
	static void Init();
	static void CreateCommandBuffer(int bufferID, drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type, bool createCache = true);
	static unsigned int CreateCommandBuffer(drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type, bool createCache = true);
	static void RemoveCommandBuffer(int bufferID);

	static void ClearTarget(unsigned int clearFlags, drgColor clearColor, float depth, unsigned int stencil, int bufferID);
	static void SwapBuffers(GUI_Window *window, int bufferID);

	static void SetScissorState(bool enable, short top, short bottom, short left, short right, int bufferID);
	static void EnableScissor(bool enable, int bufferID);
	static void SetClipRect(short top, short bottom, short left, short right, int bufferID);
	static void SetViewport(short top, short bottom, short left, short right, int bufferID);

	static void SetColorBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION operation, int bufferID);
	static void SetAlphaBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION operation, int bufferID);
	static void EnableColorBlend(bool enable, int bufferID);
	static void SetSourceColorBlend(DRG_BLEND_FUNCTION func, int bufferID);
	static void SetDestColorBlend(DRG_BLEND_FUNCTION func, int bufferID);
	static void SetColorBlendOperation(DRG_BLEND_OPERATION operation, int bufferID);
	static void EnableAlphaBlend(bool enable, int bufferID);
	static void SetSourceAlphaBlend(DRG_BLEND_FUNCTION func, int bufferID);
	static void SetDestAlphaBlend(DRG_BLEND_FUNCTION func, int bufferID);
	static void SetAlphaBlendOperation(DRG_BLEND_OPERATION operation, int bufferID);

	static void SetDepthState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int bufferID);
	static void SetStencilState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask, int bufferID);
	static void EnableDepthTest(bool enable, int bufferID);
	static void EnableDepthWrite(bool enable, int bufferID);
	static void SetDepthCompareFunc(DRG_COMPARE_FUNC func, int bufferID);
	static void EnableStencilTest(bool enable, int bufferID);
	static void EnableStencilWrite(bool enable, int bufferID);
	static void SetStencilCompareFunc(DRG_COMPARE_FUNC func, int bufferID);
	static void SetStencilRef(int ref, int bufferID);
	static void SetStencilMask(int mask, int bufferID);

	static void SetCullMode(DRG_CULL_MODE mode, int bufferID);
	static void SetFillMode(DRG_FILL_MODE mdoe, int bufferID);
	static void SetFrontFace(DRG_WINDING_ORDER order, int bufferID);

	static void SetUniform(void *uniformStructure, unsigned int uniformSize, DRG_DRAW_COMMAND_UNIFORM_TYPE uniform, int bufferID);
	static void DrawMesh(drgVertBuffer *buffer, drgMaterial *material, int startPrimitive, int numPrimitives, bool setup, bool useEffect, int bufferID);
	static void DrawModel(drgModel *model, int bufferID);

	static void BindRenderTarget(drgRenderTarget *target, int index, int bufferID);
	static void BindDepthTarget(drgRenderTarget *target, int bufferID);
	static void UnbindRenderTarget(drgRenderTarget *target, int bufferID);
	static void UpdateSurfaces(int bufferID);

	static void RunCommandList(drgDrawCommandBuffer *list, int bufferID, bool flush = true);
	static void MakeCurrent(void *renderContext, void *deviceContext, int bufferID);

	static void Clear();

	static drgDrawCommandBuffer *GetCommandBuffer(int bufferID);
	static drgRenderCache *GetCommandBufferRenderCache(int bufferID);
	static void ResetAllRenderCaches();

protected:
	static unsigned int GetNextUnusedListID();
	static drgDrawCommandBuffer *m_CommandBuffers[DRG_MAX_COMMAND_BUFFERS]; // One for each field in an int. This matches the draw list ids.
	static drgRenderCache *m_CommandCaches[DRG_MAX_COMMAND_BUFFERS];
};

#endif