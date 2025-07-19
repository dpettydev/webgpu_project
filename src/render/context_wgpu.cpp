

#include "system/global.h"

#if USE_WEBGPU

#include "render/context_wgpu.h"
#include "render/material.h"
#include "render/vertexbuff.h"
#include "render/material_manager.h"
#include "render/dynamic_draw_command_buffer.h"
#include "render/render_target.h"
#include "util/stringn.h"

const char *WEBGPU_Adapter_Types[] =
	{
		"Adapter NULL",
		"Discrete GPU",
		"Integrated GPU",
		"CPU",
		"Unknown",
};

const char *WEBGPU_Backend_Types[] =
	{
		"Undefined",
		"Null",
		"WebGPU",
		"D3D11",
		"D3D12",
		"Metal",
		"Vulkan",
		"OpenGL",
		"OpenGLES",
		"Force32",
};

void HandleWGPUError(int error, const char *message)
{
}

drgRenderContext_WEBGPU *g_GlobalContextWGPU = NULL;
void SetContextWGPU(drgRenderContext_WEBGPU *context)
{
	g_GlobalContextWGPU = context;
}

drgRenderContext_WEBGPU *GetContextWGPU()
{
	return g_GlobalContextWGPU;
}

void WEBGPU_OnDeviceError(WGPUErrorType type, WGPUStringView message, void *userdata)
{
	printf("**Error**: %s\r\n", message.data);
	assert(message.data);
}

void WEBGPU_OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userdata)
{
	if (status != WGPURequestAdapterStatus_Success)
	{
		// cannot find adapter?
		printf("**Error**: %s\r\n", message.data);
		assert(message.data);
	}
	else
	{
		// use first adapter provided
		WGPUAdapter *result = (WGPUAdapter *)userdata;
		if (*result == NULL)
		{
			*result = adapter;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// drgRenderContext_WEBGPU
////////////////////////////////////////////////////////////////////////////////////////////////////

drgRenderContext_WEBGPU::drgRenderContext_WEBGPU()
{
	m_Platform = DRG_RENDER_PLATFORM_WEBGPU;
	// m_RenderCache->SetupCache();

	for (int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget)
	{
		m_BoundRenderTargets[currentTarget] = NULL;
	}

	m_CurrentFrameBufferObject = 0;
	m_FrameBufferObject = -1;
	m_BoundDepthTarget = NULL;

	m_Adapter = NULL;
	m_Instance = NULL;
	m_Device = NULL;
	m_Surface = NULL;
	m_Queue = NULL;
	m_Encoder = NULL;
	m_Pipeline = NULL;
	m_VBuffer = NULL;
	m_RenderTexture = NULL;
	m_RenderPass = NULL;
}

drgRenderContext_WEBGPU::~drgRenderContext_WEBGPU()
{
	drgEngine::NativeContextSetMain();
}

void drgRenderContext_WEBGPU::Init()
{
	// drgEngine::NativeContextSetMain();

	int width, height;
	drgEngine::GetScreenSize(&width, &height);

	drgPrintOut("We got the screen size: %d, %d\r\n", width, height);
	m_CurrentFrameBufferObject = 0;
}

void drgRenderContext_WEBGPU::Begin()
{
	// Bind the back buffer
	m_CurrentFrameBufferObject = 0;
	drgRenderContext::Begin();

	if (m_RenderTexture != NULL)
	{
		return;
	}
	WGPUSurfaceTexture surfaceTex;
	wgpuSurfaceGetCurrentTexture(m_Surface, &surfaceTex);
	if (surfaceTex.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
	{
		drgPrintError("Cannot acquire next swap chain texture!");
	}

	WGPUTextureViewDescriptor surfaceViewDesc =
		{
			.format = wgpuTextureGetFormat(surfaceTex.texture),
			.dimension = WGPUTextureViewDimension_2D,
			.mipLevelCount = 1,
			.arrayLayerCount = 1,
			.aspect = WGPUTextureAspect_All,
			.usage = WGPUTextureUsage_RenderAttachment,
		};

	WGPUTextureView surfaceView = wgpuTextureCreateView(surfaceTex.texture, &surfaceViewDesc);
	assert(surfaceView);

	// SWAPCHAIN		m_RenderTexture = wgpuSwapChainGetCurrentTextureView(m_SwapChain);
	// SWAPCHAIN		if (!m_RenderTexture) {
	// SWAPCHAIN			drgPrintError("Cannot acquire next swap chain texture\r\n");
	// SWAPCHAIN			return;
	// SWAPCHAIN		}

	WGPUCommandEncoderDescriptor commandEncoderDesc = {};
	commandEncoderDesc.nextInChain = nullptr;
	commandEncoderDesc.label = {"Command Encoder", strlen("Command Encoder")};
	m_Encoder = wgpuDeviceCreateCommandEncoder(m_Device, &commandEncoderDesc);

	WGPURenderPassDescriptor renderPassDesc = {};
	renderPassDesc.nextInChain = nullptr;

	WGPURenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = surfaceView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
	renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
	renderPassColorAttachment.clearValue = WGPUColor{0.2, 0.2, 0.2, 1.0};
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;

	renderPassDesc.depthStencilAttachment = nullptr;
	// renderPassDesc.timestampWriteCount = 0;
	renderPassDesc.timestampWrites = nullptr;
	m_RenderPass = wgpuCommandEncoderBeginRenderPass(m_Encoder, &renderPassDesc);

	// In its overall outline, drawing a triangle is as simple as this:
	// Select which render pipeline to use
	wgpuRenderPassEncoderSetPipeline(m_RenderPass, m_Pipeline);
}

void drgRenderContext_WEBGPU::End()
{
	if (m_RenderTexture == NULL)
	{
		return;
	}
	wgpuRenderPassEncoderEnd(m_RenderPass);
	wgpuTextureViewRelease(m_RenderTexture);
	m_RenderTexture = NULL;

	WGPUCommandBufferDescriptor cmdBufferDesc = {};
	cmdBufferDesc.nextInChain = nullptr;
	cmdBufferDesc.label = {"Command buffer", strlen("Command buffer")};
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(m_Encoder, &cmdBufferDesc);
	wgpuQueueSubmit(m_Queue, 1, &command);

	wgpuSurfacePresent(m_Surface);
	// SWAPCHAIN		wgpuSwapChainPresent(m_SwapChain);
}

void drgRenderContext_WEBGPU::SetWorldViewProjectionMatrix(drgMat44 *world, drgMat44 *viewProjection)
{
	if (viewProjection)
	{
		if (world)
		{
			m_WorldViewProjectionMatrix = (*viewProjection) * (*world);
		}
		else
		{
			m_WorldViewProjectionMatrix = (*viewProjection);
			// m_WorldViewProjectionMatrix.Transpose();
		}
	}
	else
	{
		m_WorldViewProjectionMatrix = (*world);
	}
}

void drgRenderContext_WEBGPU::SetAmbientColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	m_AmbientColor = drgColor(red, green, blue, alpha);
}

void drgRenderContext_WEBGPU::SetBoneMatrix(drgMat44 *matrix, int index)
{
	assert(index >= 0 && index < NUM_BONE_MATRICES);

	drgMat44 mat = *matrix;
	m_BoneMatrices[index] = mat;
}

void drgRenderContext_WEBGPU::SetWorldMatrix(drgMat44 *world)
{
	m_WorldMatrix = *world;
}

void drgRenderContext_WEBGPU::SetSpecular(drgColor *spec)
{
	m_SpecularColor = *spec;
}

void drgRenderContext_WEBGPU::SetDiffuseColor(drgColor *color)
{
	m_DiffuseColor = *color;
}

void drgRenderContext_WEBGPU::SetTexCoordOffset(drgVec4 *offset)
{
	m_TexCoordOffset = *offset;
}

void drgRenderContext_WEBGPU::SetViewPosition(drgVec3 *position)
{
	m_ViewPosition = *position;
}

void drgRenderContext_WEBGPU::SetViewDirection(drgVec3 *direction)
{
	m_ViewDirection = *direction;
}

void drgRenderContext_WEBGPU::RenderPrimitive(unsigned int renderFlags, int primType, int numPrims, int numVertsTot, void *data, int vertSize, void *indices, bool setStream, int startVert, void *startData, int numVerts, int streamOffset)
{
	numVerts = numPrims;
	switch (primType)
	{
	case PRIM_LINELIST:
		startVert *= 2;
		numVerts *= 2;
		break;
	case PRIM_LINESTRIP:
		numVerts += 1;
		break;
	case PRIM_TRIANGLELIST:
		startVert *= 3;
		numVerts *= 3;
		break;
	case PRIM_TRIANGLESTRIP:
	case PRIM_TRIANGLEFAN:
		numVerts += 2;
		break;
	case PRIM_POINTLIST:
	default:
		break;
	}

	if (indices != NULL)
	{ // this is an indexed array
	}
	else
	{
		wgpuRenderPassEncoderSetPipeline(m_RenderPass, m_Pipeline);
		wgpuRenderPassEncoderDraw(m_RenderPass, numVerts, 1, startVert, 0);
	}
}

void drgRenderContext_WEBGPU::ApplyMakeCurrent(void *renderContext, void *deviceContext)
{
	// #if defined(_WINDOWS)
	// #if USE_OPEN_GL
	// 	wglMakeCurrent((HDC)deviceContext, (HGLRC)renderContext);		assert(GetLastError() == 0);
	// #endif
	// #endif
}

void drgRenderContext_WEBGPU::ApplySwapBuffers(GUI_Window *window)
{
	drgEngine::NativeWindowEnd(window); // There are different swaps for different OGL versions.
}

void drgRenderContext_WEBGPU::ApplyClearTarget(unsigned int clearFlags, drgColor color, float depth, unsigned int stencil)
{
	// GLbitfield flags = 0;
	if (clearFlags & DRG_CLEAR_COLOR)
	{
		// glClearColor((float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f);

		// flags |= GL_COLOR_BUFFER_BIT;
	}

	if (clearFlags & DRG_CLEAR_DEPTH)
	{
		// glClearDepth(depth);
		// flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (clearFlags & DRG_CLEAR_STENCIL)
	{
		// glClearStencil(stencil);
		// flags |= GL_STENCIL_BUFFER_BIT;
	}
}

void drgRenderContext_WEBGPU::ApplyScissorEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyClipRect(short top, short bottom, short left, short right)
{
}

void drgRenderContext_WEBGPU::ApplyScissorState(bool enable, short top, short bottom, short left, short right)
{
	ApplyScissorEnable(enable);
	ApplyClipRect(top, bottom, left, right);
}

void drgRenderContext_WEBGPU::ApplyViewport(short top, short bottom, short left, short right)
{
	//
	// This is a bit of hack for coordinate spaces. We can change this around if we want to.
	// But basically we assume that the screen orgin is at the top left corner of the screen.
	// But OpenGL assumes it's in the lower left corner of the screen. So we need to move our Y accordingly.
	//

	top = (short)m_DrawHeight - top - bottom;
}

void drgRenderContext_WEBGPU::ApplyColorBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyAlphaBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyColorBlendEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyColorSourceBlend(DRG_BLEND_FUNCTION func)
{
#if defined(USE_GLES)
	GLenum oldDest = GL_SRC_COLOR;
#else
	// GLenum oldDest = GL_ZERO;
#endif
}

void drgRenderContext_WEBGPU::ApplyColorDestBlend(DRG_BLEND_FUNCTION func)
{
#if defined(USE_GLES)
	GLenum oldSrc = GL_DST_COLOR;
#else
	// GLenum oldSrc = GL_ONE;
#endif
}

void drgRenderContext_WEBGPU::ApplyColorOperation(DRG_BLEND_OPERATION op)
{
	//
	// This isn't supported on low end platforms
	//
}

void drgRenderContext_WEBGPU::ApplyAlphaBlendEnable(bool enable)
{
	ApplyColorBlendEnable(enable); // Same code as color blend
}

void drgRenderContext_WEBGPU::ApplyAlphaSourceBlend(DRG_BLEND_FUNCTION func)
{
}

void drgRenderContext_WEBGPU::ApplyAlphaDestBlend(DRG_BLEND_FUNCTION func)
{
}

void drgRenderContext_WEBGPU::ApplyAlphaOperation(DRG_BLEND_OPERATION op)
{
	//
	// Currently not supported
	//
}

void drgRenderContext_WEBGPU::ApplyDepthState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func)
{
	ApplyDepthTestEnable(enableTest);
	ApplyDepthWriteEnable(enableWrite);
	ApplyDepthCompareFunc(func);
}

void drgRenderContext_WEBGPU::ApplyStencilState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask)
{
	ApplyStencilTestEnable(enableTest);
	ApplyStencilWriteEnable(enableWrite);
	ApplyStencilCompareFunc(func);
	ApplyStencilRef(ref);
	ApplyStencilMask(mask);
}

void drgRenderContext_WEBGPU::ApplyDepthTestEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyDepthWriteEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyDepthCompareFunc(DRG_COMPARE_FUNC func)
{
}

void drgRenderContext_WEBGPU::ApplyStencilTestEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_WEBGPU::ApplyStencilWriteEnable(bool enable)
{
	// Always writes.
}

void drgRenderContext_WEBGPU::ApplyStencilCompareFunc(DRG_COMPARE_FUNC func)
{
}

void drgRenderContext_WEBGPU::ApplyStencilRef(int ref)
{
}

void drgRenderContext_WEBGPU::ApplyStencilMask(int mask)
{
}

void drgRenderContext_WEBGPU::ApplyCullMode(DRG_CULL_MODE mode)
{
}

void drgRenderContext_WEBGPU::ApplyFrontFace(DRG_WINDING_ORDER order)
{
}

void drgRenderContext_WEBGPU::ApplyFillMode(DRG_FILL_MODE mode)
{
}

void drgRenderContext_WEBGPU::ApplyBindRenderTarget(drgRenderTarget *target, int index)
{
	if (index >= 0)
	{
		m_BoundRenderTargets[index] = target;
	}
	else
	{
		m_BoundDepthTarget = target;
	}
}

void drgRenderContext_WEBGPU::ApplyUnbindRenderTarget(drgRenderTarget *target)
{
	for (int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget)
	{
		if (m_BoundRenderTargets[currentTarget] == target)
		{
			target->Unbind();
			m_BoundRenderTargets[currentTarget] = NULL;
			break;
		}
	}
}

void drgRenderContext_WEBGPU::ApplyUpdateSurfaces()
{
#if (defined(_ANDROID) || defined(_IPHONE))
	drgPrintError("This needs to be revisited");
#endif
	if (m_BoundRenderTargets[0] == NULL)
	{
		m_CurrentFrameBufferObject = 0;
	}
	else
	{
		//
		// First we bind our context frame buffer if necessary.
		//

		if (m_CurrentFrameBufferObject != m_FrameBufferObject)
		{
			m_CurrentFrameBufferObject = m_FrameBufferObject;
		}

		if (m_BoundDepthTarget)
		{
			m_BoundDepthTarget->Bind(-1);
			// m_BoundDepthTarget->Unbind();
		}
	}
}

static void FatalError(WGPUStringView message)
{
	drgPrintOut("%.*s", (int)message.length, message.data);
	assert(0);
}

static void OnDeviceError(const WGPUDevice *device, WGPUErrorType type, WGPUStringView message, void *userdata1, void *userdata2)
{
	FatalError(message);
}

#ifdef EMSCRIPTEN
static void OnRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata)
{
	UserData *data = (UserData *)userdata;
	if (status != WGPURequestAdapterStatus_Success)
	{
		FatalError(message);
	}
	else
	{
		data->adapter = adapter;
	}
	data->requestEnded = true;
}
#else
static void OnRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userdata1, void *userdata2)
{
	if (status != WGPURequestAdapterStatus_Success)
	{
		FatalError(message);
	}
	else
	{
		WGPUAdapter *result = (WGPUAdapter *)userdata1;
		if (*result == NULL)
		{
			*result = adapter;
		}
	}
}
#endif

#ifdef EMSCRIPTEN
static void OnRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata)
{
	UserData *data = (UserData *)userdata;
	if (status != WGPURequestDeviceStatus_Success)
	{
		FatalError(message);
	}
	else
	{
		data->device = device;
	}
	data->requestEnded = true;
}
#else
static void OnRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userdata1, void *userdata2)
{
	if (status != WGPURequestDeviceStatus_Success)
	{
		FatalError(message);
	}
	else
	{
		WGPUDevice *result = (WGPUDevice *)userdata1;
		if (*result == NULL)
		{
			*result = device;
		}
	}
}
#endif

void drgRenderContext_WEBGPU::SetupWebGPU()
{

	WGPURequestAdapterOptions options = {};
	options.compatibleSurface = m_Surface;
	options.powerPreference = WGPUPowerPreference_HighPerformance;

#ifdef EMSCRIPTEN
	UserData userData;
	userData.requestEnded = false;
	wgpuInstanceRequestAdapter(instance, &options, OnRequestAdapter, &userData);
	while (!userData.requestEnded)
	{
		wgpuInstanceProcessEvents(instance);
		// Waiting for 200ms to avoid asking too often to process events
		emscripten_sleep(100);
	}
	adapter = userData.adapter;
#else
	WGPURequestAdapterCallbackInfo reqadapterinfo = {};
	reqadapterinfo.mode = WGPUCallbackMode_WaitAnyOnly;
	reqadapterinfo.callback = &OnRequestAdapter;
	reqadapterinfo.userdata1 = &m_Adapter;

	WGPUFuture adapterfuture = wgpuInstanceRequestAdapter(m_Instance, &options, reqadapterinfo);
	WGPUFutureWaitInfo adapterwait = {};
	adapterwait.future = adapterfuture;

	WGPUWaitStatus adapterstatus = wgpuInstanceWaitAny(m_Instance, 1, &adapterwait, 0);
	assert(adapterstatus == WGPUWaitStatus_Success);
#endif

	assert(m_Adapter && "Failed to get WebGPU adapter");

	// can query extra details on what adapter supports:
	// wgpuAdapterEnumerateFeatures
	// wgpuAdapterGetLimits
	// wgpuAdapterGetProperties
	// wgpuAdapterHasFeature

	WGPUAdapterInfo adapterinfo = {0};
	wgpuAdapterGetInfo(m_Adapter, &adapterinfo);

	const char *adapter_types[] =
		{
			"NONE",
			"Discrete GPU",
			"Integrated GPU",
			"CPU",
			"unknown",
		};

#ifdef EMSCRIPTEN
	drgPrintOut("Device        = %s\n"
				"Description   = %s\n"
				"Vendor        = %s\n"
				"Architecture  = %s\n"
				"Adapter Type  = %s\n",
				adapterinfo.device,
				adapterinfo.description,
				adapterinfo.vendor,
				adapterinfo.architecture,
				adapter_types[adapterinfo.adapterType]);
#else
	drgPrintOut("Device        = %.*s\n"
				"Description   = %.*s\n"
				"Vendor        = %.*s\n"
				"Architecture  = %.*s\n"
				"Adapter Type  = %s\n",
				(int)adapterinfo.device.length, adapterinfo.device.data,
				(int)adapterinfo.description.length, adapterinfo.description.data,
				(int)adapterinfo.vendor.length, adapterinfo.vendor.data,
				(int)adapterinfo.architecture.length, adapterinfo.architecture.data,
				adapter_types[adapterinfo.adapterType]);
#endif

#ifdef EMSCRIPTEN
	userData.requestEnded = false;
	WGPUDeviceDescriptor devicedesc = {};
	wgpuAdapterRequestDevice(adapter, &devicedesc, OnRequestDevice, &userData);
	while (!userData.requestEnded)
	{
		wgpuInstanceProcessEvents(instance);
		// Waiting for 200ms to avoid asking too often to process events
		emscripten_sleep(1000);
	}
	device = userData.device;
#else
	// if you want to be sure device will support things you'll use, you can specify requirements here:

	// WGPUSupportedLimits supported = { 0 };
	// wgpuAdapterGetLimits(adapter, &supported);

	// supported.limits.maxTextureDimension2D = kTextureWidth;
	// supported.limits.maxBindGroups = 1;
	// supported.limits.maxBindingsPerBindGroup = 3; // uniform buffer for vertex shader, and texture + sampler for fragment
	// supported.limits.maxSampledTexturesPerShaderStage = 1;
	// supported.limits.maxSamplersPerShaderStage = 1;
	// supported.limits.maxUniformBuffersPerShaderStage = 1;
	// supported.limits.maxUniformBufferBindingSize = 4 * 4 * sizeof(float); // 4x4 matrix
	// supported.limits.maxVertexBuffers = 1;
	// supported.limits.maxBufferSize = sizeof(kVertexData);
	// supported.limits.maxVertexAttributes = 3; // pos, texcoord, color
	// supported.limits.maxVertexBufferArrayStride = kVertexStride;
	// supported.limits.maxColorAttachments = 1;

	WGPUDeviceDescriptor devicedesc = {};
	devicedesc.uncapturedErrorCallbackInfo.callback = &OnDeviceError;
	// extra features: https://dawn.googlesource.com/dawn/+/refs/heads/main/src/dawn/native/Features.cpp
	//.requiredFeaturesCount = n
	//.requiredFeatures = (WGPUFeatureName[]) { ... }
	//.requiredLimits = &(WGPURequiredLimits) { .limits = supported.limits },

	WGPURequestDeviceCallbackInfo deviceinfo = {};
	deviceinfo.mode = WGPUCallbackMode_WaitAnyOnly;
	deviceinfo.callback = &OnRequestDevice;
	deviceinfo.userdata1 = &m_Device;

	WGPUFuture devicefuture = wgpuAdapterRequestDevice(m_Adapter, &devicedesc, deviceinfo);

	WGPUFutureWaitInfo devicewait = {};
	devicewait.future = devicefuture;

	WGPUWaitStatus devicestatus = wgpuInstanceWaitAny(m_Instance, 1, &devicewait, 0);
	assert(devicestatus == WGPUWaitStatus_Success);
#endif
	assert(m_Device && "Failed to create WebGPU device");

	m_Queue = wgpuDeviceGetQueue(m_Device);
	assert(m_Queue);

	// m_Info->surface_info.width = m_WindowWidth;
	// m_Info->surface_info.height = m_WindowHeight;

	int width, height;
	drgEngine::GetScreenSize(&width, &height);

	// Configure surface
	WGPUSurfaceCapabilities surfaceCaps = {};
	wgpuSurfaceGetCapabilities(m_Surface, m_Adapter, &surfaceCaps);
	m_SurfaceFormat = surfaceCaps.formats[0]; // Use first supported format
	WGPUSurfaceConfiguration surfaceConfig = {};
	surfaceConfig.device = m_Device;
	surfaceConfig.format = m_SurfaceFormat;
	surfaceConfig.usage = WGPUTextureUsage_RenderAttachment;
	surfaceConfig.width = width;
	surfaceConfig.height = height;
	surfaceConfig.presentMode = WGPUPresentMode_Fifo;
	wgpuSurfaceConfigure(m_Surface, &surfaceConfig);
	/*
			// Create texture
			WGPUTextureDescriptor texture_desc = {};
			texture_desc.nextInChain = NULL;
			texture_desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
			texture_desc.dimension = WGPUTextureDimension_2D;
			texture_desc.size = {m_Info->surface_info.width, m_Info->surface_info.height, 1};
			texture_desc.format = WGPUTextureFormat_RGBA8Unorm;
			texture_desc.mipLevelCount = 1;
			texture_desc.sampleCount = 1;
			texture_desc.viewFormatCount = 0;
			texture_desc.viewFormats = NULL;
			m_Info->draw_texture = wgpuDeviceCreateTexture(m_Info->device, &texture_desc);

			// Create texture view
			WGPUTextureViewDescriptor texture_view_desc = {};
			texture_view_desc.nextInChain = NULL;
			texture_view_desc.format = WGPUTextureFormat_RGBA8Unorm;
			texture_view_desc.dimension = WGPUTextureViewDimension_2D;
			texture_view_desc.baseMipLevel = 0;
			texture_view_desc.mipLevelCount = 1;
			texture_view_desc.baseArrayLayer = 0;
			texture_view_desc.arrayLayerCount = 1;
			texture_view_desc.aspect = WGPUTextureAspect_All;
			m_Info->draw_texture_view = wgpuTextureCreateView(m_Info->draw_texture, &texture_view_desc);

			// Create sampler
			WGPUSamplerDescriptor sampler_desc = {};
			sampler_desc.nextInChain = NULL;
			sampler_desc.addressModeU = WGPUAddressMode_ClampToEdge;
			sampler_desc.addressModeV = WGPUAddressMode_ClampToEdge;
			sampler_desc.addressModeW = WGPUAddressMode_ClampToEdge;
			sampler_desc.magFilter = WGPUFilterMode_Linear;
			sampler_desc.minFilter = WGPUFilterMode_Linear;
			sampler_desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
			sampler_desc.lodMinClamp = 0.0f;
			sampler_desc.lodMaxClamp = 32.0f;
			sampler_desc.compare = WGPUCompareFunction_Undefined;
			sampler_desc.maxAnisotropy = 1;
			m_Draw_sampler = wgpuDeviceCreateSampler(m_Device, &sampler_desc);
			*/
	/*
		// Create shader module
		FILE *fp = NULL;
		fopen_s(&fp, "C:/TEMP/webgpu_project/runtime/engine/shader/webgpu/ui.wgsl", "rb");
		assert(fp);
		fseek(fp, 0, SEEK_END);
		unsigned int file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *shaderSource = (char *)malloc(file_len + 1);
		fread(shaderSource, file_len, 1, fp);
		shaderSource[file_len] = '\0';
		fclose(fp);

		struct Vertex
		{
			float position[3];
			float uv[2];
			unsigned int color;
		};
		const uint32_t kVertexStride = sizeof(struct Vertex);

		WGPUVertexAttribute vb_atrrib[] = {
			{NULL, WGPUVertexFormat_Float32x3, offsetof(struct Vertex, position), 0},
			{NULL, WGPUVertexFormat_Float32x2, offsetof(struct Vertex, uv), 1},
			{NULL, WGPUVertexFormat_Unorm8x4, offsetof(struct Vertex, color), 2},
		};

		WGPUVertexBufferLayout vb_layout[] = {{}};
		vb_layout[0].arrayStride = kVertexStride;
		vb_layout[0].stepMode = WGPUVertexStepMode_Vertex;
		vb_layout[0].attributeCount = 3;
		vb_layout[0].attributes = vb_atrrib;

		WGPUShaderModuleWGSLDescriptor wgslDesc = {};
		wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
		wgslDesc.code = WGPUStringView{shaderSource, file_len};
		WGPUShaderModuleDescriptor shaderDesc = {};
		shaderDesc.nextInChain = &wgslDesc.chain;
		WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_Device, &shaderDesc);
		assert(shaderModule);

		// Create bind group layout
		WGPUBindGroupLayoutEntry bindGroupLayoutEntry[0] = {};

		WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
		bindGroupLayoutDesc.entryCount = 0;
		bindGroupLayoutDesc.entries = bindGroupLayoutEntry;
		WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_Device, &bindGroupLayoutDesc);
		assert(bindGroupLayout);

		// Create pipeline layout
		WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
		pipelineLayoutDesc.bindGroupLayoutCount = 1;
		pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
		WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(m_Device, &pipelineLayoutDesc);
		assert(pipelineLayout);
		/*
			// Create bind group
			WGPUBindGroupEntry bindGroupEntry[0] = {};

			WGPUBindGroupDescriptor bindGroupDesc = {};
			bindGroupDesc.layout = bindGroupLayout;
			bindGroupDesc.entryCount = 0;
			bindGroupDesc.entries = bindGroupEntry;
			m_BindGroup = wgpuDeviceCreateBindGroup(m_Device, &bindGroupDesc);
			assert(m_Info->bindGroup);
		*/
	/*
		// Create render pipeline
		WGPURenderPipelineDescriptor pipelineDesc = {};
		pipelineDesc.layout = pipelineLayout;

		WGPUVertexState vertexState = {};
		vertexState.bufferCount = 1;
		vertexState.buffers = vb_layout;
		vertexState.module = shaderModule;
		vertexState.entryPoint = WGPUStringView{"main_vs", strlen("main_vs")};
		pipelineDesc.vertex = vertexState;

		WGPUFragmentState fragmentState = {};
		fragmentState.module = shaderModule;
		fragmentState.entryPoint = WGPUStringView{"main_fs", strlen("main_fs")};
		WGPUColorTargetState colorTarget = {};
		colorTarget.format = m_SurfaceFormat;
		colorTarget.writeMask = WGPUColorWriteMask_All;
		fragmentState.targets = &colorTarget;
		fragmentState.targetCount = 1;
		pipelineDesc.fragment = &fragmentState;

		pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
		pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
		pipelineDesc.primitive.cullMode = WGPUCullMode_None;

		pipelineDesc.multisample.count = 1;
		pipelineDesc.multisample.mask = ~0u;

		m_Pipeline = wgpuDeviceCreateRenderPipeline(m_Device, &pipelineDesc);
		assert(m_Pipeline);
		*/
}

#endif