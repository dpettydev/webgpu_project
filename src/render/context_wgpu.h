
#ifndef _DRG_RENDER_CONTEXT_WIN_WEBGPU_
#define _DRG_RENDER_CONTEXT_WIN_WEBGPU_

#include "system/global.h"

#if USE_WEBGPU

// Includes
#include "render/context.h"
#include "system/extern_inc.h"

// OpenGL Error Handlers

class drgMaterial;
class drgVertBuffer;

// Defines
#define NUM_DRAW_BUFFERS 2

// Class Declaration
class drgRenderContext_WEBGPU : public drgRenderContext
{
public:
	drgRenderContext_WEBGPU();
	virtual ~drgRenderContext_WEBGPU();

	virtual void Init();
	virtual void Begin();
	virtual void End();

	virtual void SetAmbientColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255);
	virtual void SetWorldViewProjectionMatrix(drgMat44 *world, drgMat44 *viewProjection = NULL);
	virtual void SetWorldMatrix(drgMat44 *world);
	virtual void SetBoneMatrix(drgMat44 *boneMat, int index);
	virtual void SetSpecular(drgColor *spec);
	virtual void SetDiffuseColor(drgColor *color);
	virtual void SetTexCoordOffset(drgVec4 *offset);
	virtual void SetViewPosition(drgVec3 *position);
	virtual void SetViewDirection(drgVec3 *direction);

	virtual void RenderPrimitive(unsigned int renderFlags, int primType, int numPrims, int numVertsTot, void *data, int vertSize, void *indices, bool setStream = true, int startVert = 0, void *startData = NULL, int numVerts = 0, int streamOffset = 0);


	void SetupWebGPU();
	WGPUAdapter m_Adapter;
	WGPUInstance m_Instance;
	WGPUDevice m_Device;
    WGPUTextureFormat m_SurfaceFormat = WGPUTextureFormat_Undefined;
	WGPUSurface m_Surface;
	WGPUQueue m_Queue;
	WGPUCommandEncoder m_Encoder;
	WGPURenderPipeline m_Pipeline;
	WGPUBuffer m_VBuffer;
	WGPUTextureView m_RenderTexture;
	WGPURenderPassEncoder m_RenderPass;

protected:
	virtual void ApplySwapBuffers(GUI_Window *window);
	virtual void ApplyClearTarget(unsigned int clearFlags, drgColor color, float depth, unsigned int stencil);
	virtual void ApplyScissorEnable(bool enable);
	virtual void ApplyScissorState(bool enable, short top, short bottom, short left, short right);
	virtual void ApplyClipRect(short top, short bottom, short left, short right);
	virtual void ApplyViewport(short top, short bottom, short left, short right);
	virtual void ApplyColorBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op);
	virtual void ApplyAlphaBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op);
	virtual void ApplyColorBlendEnable(bool enable);
	virtual void ApplyColorSourceBlend(DRG_BLEND_FUNCTION func);
	virtual void ApplyColorDestBlend(DRG_BLEND_FUNCTION func);
	virtual void ApplyColorOperation(DRG_BLEND_OPERATION op);
	virtual void ApplyAlphaBlendEnable(bool enable);
	virtual void ApplyAlphaSourceBlend(DRG_BLEND_FUNCTION func);
	virtual void ApplyAlphaDestBlend(DRG_BLEND_FUNCTION func);
	virtual void ApplyAlphaOperation(DRG_BLEND_OPERATION op);
	virtual void ApplyDepthState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func);
	virtual void ApplyStencilState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask);
	virtual void ApplyDepthTestEnable(bool enable);
	virtual void ApplyDepthWriteEnable(bool enable);
	virtual void ApplyDepthCompareFunc(DRG_COMPARE_FUNC func);
	virtual void ApplyStencilTestEnable(bool enable);
	virtual void ApplyStencilWriteEnable(bool enable);
	virtual void ApplyStencilCompareFunc(DRG_COMPARE_FUNC func);
	virtual void ApplyStencilRef(int ref);
	virtual void ApplyStencilMask(int mask);
	virtual void ApplyCullMode(DRG_CULL_MODE mode);
	virtual void ApplyFrontFace(DRG_WINDING_ORDER order);
	virtual void ApplyFillMode(DRG_FILL_MODE mode);
	virtual void ApplyMakeCurrent(void *platformRenderContext, void *platformDeviceContext);
	virtual void ApplyBindRenderTarget(drgRenderTarget *target, int index);
	virtual void ApplyUnbindRenderTarget(drgRenderTarget *target);
	virtual void ApplyUpdateSurfaces();

	unsigned int m_FrameBufferObject;
	unsigned int m_CurrentFrameBufferObject;
	drgRenderTarget *m_BoundDepthTarget;
	drgRenderTarget *m_BoundRenderTargets[DRG_MAX_RENDER_TARGETS];
};

extern const char *WEBGPU_Adapter_Types[];
extern const char *WEBGPU_Backend_Types[];

extern void WEBGPU_OnDeviceError(WGPUErrorType type, WGPUStringView message, void *userdata);
extern void WEBGPU_OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userdata);

#endif // USE_WEBGPU

#endif // _DRG_RENDER_CONTEXT_WIN_WEBGPU_
