// MaterialManager.cpp
// This will define the functionality for a drgMaterialManager

#include "system/global.h"

#if USE_WEBGPU

// Includes
#include "material_manager.h"
#include "material_manager_wgpu.h"
#include "system/extern_inc.h"
#include "system/file.h"
#include "util/stringn.h"
#include "context.h"
#include "vertexbuff.h"
#include "render/context_wgpu.h"

struct MaterialLoatVertData
{
	char name[64] = "";
	char name_file[256] = "";
	unsigned int atrib = 0;
};
const int g_NumMaterialData = 1;
MaterialLoatVertData g_MaterialData[g_NumMaterialData] = {
	{"VS_UI", "shader/webgpu/ui.wgsl", DRG_VERTEX_TEXTURED_COLORED::attribs},
};

// Static Initialization
bool drgMaterialManager::m_FirstInit = true;
void *drgMaterialManager::m_HeaderBasicPtr = NULL;
void *drgMaterialManager::m_HeaderExtPtr = NULL;
drgUInt32 drgMaterialManager::m_CurrentVertDeclNum = 999;
drgUInt64 drgMaterialManager::m_CurrentEffectVtx = 999;
drgUInt64 drgMaterialManager::m_CurrentEffectPxl = 999;
bool drgMaterialManager::m_PassBegin = false;
drgMaterial::DRG_MATERIAL_PASS_MODE drgMaterialManager::m_RenderShadows = drgMaterial::PASS_MODE_DIFFUSE;
bool drgMaterialManager::m_RenderDepth = false;

unsigned int g_ShaderProgram = 0;
int g_a_PositionHandle = 0;
int g_a_ColorHandle = 0;
int g_a_TexHandle = 0;

// SetUpUniformVal
// Will retreive the glsl uniform location and store them.
inline void SetUpUniformVal(int *handle, unsigned int ShaderProgram, char *uniform)
{
	//	int val = pglGetUniformLocation(ShaderProgram, uniform);					GL_ERROR( "Failed to get uniform location in shader" );
	// drgPrintOut("%s  %i\n", uniform, val);
	//	(*handle) = val;
}

void SetUpUniforms(drgMaterialDataWEBGPU *data, unsigned int ShaderProgram, const char *desc)
{
	// get handles to the global shader variables
	// drgPrintOut("\nShader Uniforms %s\n", desc);
	// SetUpUniformVal(&data->m_hWorldViewProjection, ShaderProgram, "g_mWorldViewProjection");
	// SetUpUniformVal(&data->m_hWorld, ShaderProgram, "g_mWorld");
	// SetUpUniformVal(&data->m_hBoneMatrix[0], ShaderProgram, "g_Wld0");
	// SetUpUniformVal(&data->m_hBoneMatrix[1], ShaderProgram, "g_Wld1");
	// SetUpUniformVal(&data->m_hBoneMatrix[2], ShaderProgram, "g_Wld2");
	// SetUpUniformVal(&data->m_hBoneMatrix[3], ShaderProgram, "g_Wld3");
	// SetUpUniformVal(&data->m_hBoneMatrix[4], ShaderProgram, "g_Wld4");
	// SetUpUniformVal(&data->m_hBoneMatrix[5], ShaderProgram, "g_Wld5");
	// SetUpUniformVal(&data->m_hBoneMatrix[6], ShaderProgram, "g_Wld6");
	// SetUpUniformVal(&data->m_hBoneMatrix[7], ShaderProgram, "g_Wld7");
	// SetUpUniformVal(&data->m_hAmbientColor, ShaderProgram, "g_AmbientColor");
	// SetUpUniformVal(&data->m_hSpecularLevel, ShaderProgram, "g_SpecularLevel");
	// SetUpUniformVal(&data->m_hDiffuseColor, ShaderProgram, "g_DiffuseColor");
	// SetUpUniformVal(&data->m_hTexOffsetUV, ShaderProgram, "g_TexOffsetUV");
	// SetUpUniformVal(&data->m_hViewPos, ShaderProgram, "g_ViewPos");
	// SetUpUniformVal(&data->m_hViewVec, ShaderProgram, "g_ViewVec");
	//
	// data->m_SamplerLocations[ 0 ] = pglGetUniformLocation(ShaderProgram, "Flat_TextureSampler0");
	// data->m_SamplerLocations[ 1 ] = pglGetUniformLocation(ShaderProgram, "Flat_TextureSampler1");
	// data->m_SamplerLocations[ 2 ] = pglGetUniformLocation(ShaderProgram, "Flat_TextureSampler2");
	// data->m_SamplerLocations[ 3 ] = pglGetUniformLocation(ShaderProgram, "Flat_TextureSampler3");
	//
	// data->m_ShadowMapSamplerLocation = pglGetUniformLocation(ShaderProgram, "Flat_TextureSamplerShadowMap");
}

inline void SetUpAttribVal(int *handle, unsigned int ShaderProgram, char *attrib)
{
	// int val = pglGetAttribLocation(ShaderProgram, attrib);			GL_ERROR( "Failed to find the attribute location in the shader" );
	// if(val<0)
	//{
	//	*handle = 0;
	//	return;
	// }

	// drgPrintOut("%s  %i\n", attrib, val);
	//(*handle) = val;
}

void SetUpAttribs(unsigned int ShaderProgram, drgMaterialDataWEBGPU *data, const char *desc)
{
	// drgPrintOut("\nShader Attributes %s\n", desc);
	// SetUpAttribVal(&data->m_PositionAttribute, ShaderProgram, "MaudeInput_Position");
	// SetUpAttribVal(&data->m_Color0Attribute, ShaderProgram, "MaudeInput_Color0");
	// SetUpAttribVal(&data->m_Color1Attribute, ShaderProgram, "MaudeInput_Color1");
	// SetUpAttribVal(&data->m_TexCoord0Attribute, ShaderProgram, "MaudeInput_TexUV0");
	// SetUpAttribVal(&data->m_TexCoord1Attribute, ShaderProgram, "MaudeInput_TexUV0");
	// SetUpAttribVal(&data->m_NormalAttribute, ShaderProgram, "MaudeInput_Normal");
	// SetUpAttribVal(&data->m_TangentAttribute, ShaderProgram, "MaudeInput_Tangent");
	// SetUpAttribVal(&data->m_Weight0Attribute, ShaderProgram, "MaudeInput_Weight0");
	// SetUpAttribVal(&data->m_Weight1Attribute, ShaderProgram, "MaudeInput_Weight1");
	// SetUpAttribVal(&data->m_NormalMorphAttribute, ShaderProgram, "MaudeInput_MorphNormal");
	// SetUpAttribVal(&data->m_PosMorphAttribute, ShaderProgram, "MaudeInput_MorphPosition");
}

void LoadShader(drgShaderDataWEBGPU *data, drgRenderContext_WEBGPU *context)
{
	WGPUShaderModuleWGSLDescriptor wgslDesc = {};
	wgslDesc.chain.next = NULL;
	wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
	wgslDesc.code = WGPUStringView{data->m_ShaderCode, (unsigned int)data->m_ShaderCodeLength};
	WGPUShaderModuleDescriptor shaderDesc = {};
	shaderDesc.nextInChain = &wgslDesc.chain;
	data->m_GLShaderHandle = wgpuDeviceCreateShaderModule(context->m_Device, &shaderDesc);
	assert(data->m_GLShaderHandle);
	//drgPrintOut("Shader module: 0x%x\r\n", data->m_GLShaderHandle);
	//drgPrintOut("Shader code: %s\r\n", data->m_ShaderCode);

	data->m_GLProgramHandle = 0;
}

void LinkShaders(drgShaderDataWEBGPU *linkShader, drgMaterialDataWEBGPU *materialData, drgRenderContext_WEBGPU *context)
{
	assert(linkShader && "Tried to link a NULL shader");
	assert(materialData && "Tried to link a NULL material data");

	drgPrintOut("Link Shader %s\n", linkShader->m_ShaderName);

	int linked = 0;
	char errormsg[512] = {0};

	if (linkShader->m_GLShaderHandle)
	{
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

		// Create bind group layout
		WGPUBindGroupLayoutEntry bindGroupLayoutEntry[0] = {};

		WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
		bindGroupLayoutDesc.entryCount = 0;
		bindGroupLayoutDesc.entries = bindGroupLayoutEntry;
		WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(context->m_Device, &bindGroupLayoutDesc);
		assert(bindGroupLayout);

		// Create pipeline layout
		WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
		pipelineLayoutDesc.bindGroupLayoutCount = 1;
		pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
		WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(context->m_Device, &pipelineLayoutDesc);
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

		// Create render pipeline
		WGPURenderPipelineDescriptor pipelineDesc = {};
		pipelineDesc.layout = pipelineLayout;

		WGPUVertexState vertexState = {};
		vertexState.bufferCount = 1;
		vertexState.buffers = vb_layout;
		vertexState.module = (WGPUShaderModule)linkShader->m_GLShaderHandle;
		vertexState.entryPoint = WGPUStringView{"main_vs", strlen("main_vs")};
		pipelineDesc.vertex = vertexState;

		WGPUFragmentState fragmentState = {};
		fragmentState.module = (WGPUShaderModule)linkShader->m_GLShaderHandle;
		fragmentState.entryPoint = WGPUStringView{"main_fs", strlen("main_fs")};
		WGPUColorTargetState colorTarget = {};
		colorTarget.format = context->m_SurfaceFormat;
		colorTarget.writeMask = WGPUColorWriteMask_All;
		fragmentState.targets = &colorTarget;
		fragmentState.targetCount = 1;
		pipelineDesc.fragment = &fragmentState;

		pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
		pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
		pipelineDesc.primitive.cullMode = WGPUCullMode_None;

		pipelineDesc.multisample.count = 1;
		pipelineDesc.multisample.mask = ~0u;

		context->m_Pipeline = wgpuDeviceCreateRenderPipeline(context->m_Device, &pipelineDesc);
		drgPrintOut("Render pipeline: 0x%x\r\n", context->m_Pipeline);
	}
}

// Init
void drgMaterialManager::Init(const char *materialPoolFilepath)
{
	// Get the material filepath
	drgPrintOut("Loading material manager from: %s\r\n", materialPoolFilepath);
	LoadMaterial(materialPoolFilepath, true);
	m_FirstInit = false;
}

// LoadMaterial
void drgMaterialManager::LoadMaterial(const char *filename, bool basic)
{
	unsigned int j = 0;
	unsigned int counter = 0;
	unsigned int vertnum = 0;
	unsigned int pixlnum = 0;
	unsigned int matnum = 0;
	unsigned int curptr = 3;
	drgUInt64 *ptrpos = 0;
	drgUInt64 *ptrvtxpos = 0;
	drgUInt64 *ptrpxlpos = 0;
	char *ptrname = 0;
	char *ptrcode = 0;

	drgRenderContext_WEBGPU *context = GetContextWGPU();

	// Allocate the material pool

	// drgUInt64* headerptr = (drgUInt64*)drgFile::MallocFile(filename);
	// assert( headerptr );

	// if(basic)
	//	m_HeaderBasicPtr = headerptr;
	// else
	//	m_HeaderExtPtr = headerptr;

	MEM_INFO_SET_NEW;
	drgMaterialPoolWEBGPU *pool = new drgMaterialPoolWEBGPU;

	// Now load vertex shader list
	pool->m_NumShaders = g_NumMaterialData;

	// Get the shader data
	assert(pool->m_NumShaders != 0);
	pool->m_Shaders = (drgShaderDataWEBGPU **)drgMemAllocZero(pool->m_NumShaders * sizeof(drgShaderDataWEBGPU *));
	for (int currentShader = 0; currentShader < pool->m_NumShaders; ++currentShader)
	{
		MEM_INFO_SET_NEW;
		pool->m_Shaders[currentShader] = new drgShaderDataWEBGPU;
		drgShaderDataWEBGPU *shaderData = pool->m_Shaders[currentShader];

		int nameLen = drgString::Length(g_MaterialData[currentShader].name);
		shaderData->m_ShaderName = (char *)drgMemAllocZero(nameLen + 1);
		drgString::CopySafe(shaderData->m_ShaderName, g_MaterialData[currentShader].name, nameLen);

		drgAssetReader shader_reader;
		shader_reader.OpenFile(g_MaterialData[currentShader].name_file);
		shaderData->m_ShaderCodeLength = shader_reader.GetFileSize();

		shaderData->m_ShaderCode = (char *)drgMemAllocZero(shaderData->m_ShaderCodeLength);
		drgString::CopySafe(shaderData->m_ShaderCode, (char *)shader_reader.GetFileMem(), shaderData->m_ShaderCodeLength);

		// Load the shader
		LoadShader(shaderData, (drgRenderContext_WEBGPU *)context);

		// Determine the vertex attributes
		shaderData->m_VertexAttributes = g_MaterialData[currentShader].atrib;
	}

	// Now we need to load all the materials
	pool->m_NumMaterials = g_NumMaterialData;

	// Get the material data
	assert(pool->m_NumMaterials != 0);
	pool->m_Materials = (drgMaterialDataWEBGPU **)drgMemAllocZero(pool->m_NumMaterials * sizeof(drgMaterialDataWEBGPU *));
	for (int currentMaterial = 0; currentMaterial < pool->m_NumMaterials; ++currentMaterial)
	{
		MEM_INFO_SET_NEW;
		pool->m_Materials[currentMaterial] = new drgMaterialDataWEBGPU;
		drgMaterialDataWEBGPU *materialData = pool->m_Materials[currentMaterial];

		materialData->m_ShaderName = g_MaterialData[currentMaterial].name;
		materialData->m_ShaderIndex = materialData->m_ShaderIndex = -1;

		// Find the shader indices
		for (int currentVertexShader = 0; currentVertexShader < pool->m_NumShaders; ++currentVertexShader)
		{
			if (drgString::Compare(materialData->m_ShaderName, pool->m_Shaders[currentVertexShader]->m_ShaderName) == 0)
			{
				materialData->m_ShaderIndex = currentVertexShader;
			}
		}

		assert(materialData->m_ShaderIndex != -1);

		// Now setup the program for the shaders
		drgShaderDataWEBGPU *linkShader = pool->m_Shaders[materialData->m_ShaderIndex];

		LinkShaders(linkShader, materialData, (drgRenderContext_WEBGPU *)context);
	}

	m_HeaderBasicPtr = pool; // We're not using the extended header pointer.
}

// ReloadMaterials
void drgMaterialManager::ReloadMaterials()
{
	// Only windows did something with this, but it was predicated on the EXP shaders define.
	// I'm stubbing this out and seeing if we actually need it.
}

void drgMaterialManager::ReleaseGPUResources()
{
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)m_HeaderBasicPtr;
		if (pool)
		{
			// Release our shaders
			for (int currentShader = 0; currentShader < pool->m_NumShaders; ++currentShader)
			{
				drgShaderDataWEBGPU *vertShader = pool->m_Shaders[currentShader];
				if (vertShader == NULL)
					continue;

				if (vertShader->m_GLShaderHandle != 0)
				{
					//					pglDeleteShader( vertShader->m_GLShaderHandle );
					vertShader->m_GLShaderHandle = 0;
				}

				if (vertShader->m_GLProgramHandle != 0)
				{
					//					pglDeleteProgram( vertShader->m_GLProgramHandle );
					vertShader->m_GLProgramHandle = 0;
				}
			}
		}
	}
}

void drgMaterialManager::RecreateGPUResources()
{
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)m_HeaderBasicPtr;
		if (pool)
		{
			// Recreate our shaders
			for (int currentShader = 0; currentShader < pool->m_NumShaders; ++currentShader)
			{
				drgShaderDataWEBGPU *vertShader = pool->m_Shaders[currentShader];
				if (vertShader == NULL)
					continue;

				//				LoadShader( vertShader, GL_VERTEX_SHADER );
			}

			// Recreate our shader programs
			for (int currentMaterial = 0; currentMaterial < pool->m_NumMaterials; ++currentMaterial)
			{
				drgMaterialDataWEBGPU *materialData = pool->m_Materials[currentMaterial];
				if (materialData == NULL)
					continue;

				// Now setup the program for the shaders
				drgShaderDataWEBGPU *linkShader = pool->m_Shaders[materialData->m_ShaderIndex];

				// LinkShaders(linkShader, materialData, (drgRenderContext_WEBGPU*)context);
			}
		}
	}
}

// StopRenderForce
void drgMaterialManager::StopRenderForce()
{
	if (m_PassBegin)
	{
		m_PassBegin = false;
	}
	m_CurrentVertDeclNum = (drgUInt32)-1;
	m_CurrentEffectVtx = 999;
	m_CurrentEffectPxl = 999;
}

// CreateVertDefs
void drgMaterialManager::CreateVertDefs()
{
}

// Cleanup
void drgMaterialManager::Cleanup()
{
	// declarations
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)m_HeaderBasicPtr;
		if (pool)
		{
			SAFE_FREE_ARRAY_OF_NEWS(pool->m_Materials, pool->m_NumMaterials);
			SAFE_FREE_ARRAY_OF_NEWS(pool->m_Shaders, pool->m_NumShaders);

			delete pool;
			m_HeaderBasicPtr = NULL;
		}
	}
}

// GetMaterialNumByName
drgUInt64 drgMaterialManager::GetMaterialNumByName(const char *name)
{
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)m_HeaderBasicPtr;
		for (int currentMaterial = 0; currentMaterial < pool->m_NumMaterials; ++currentMaterial)
		{
			if (drgString::CompareNoCase(name, pool->m_Materials[currentMaterial]->m_ShaderName, drgString::Length(name)) == 0)
			{
				return (drgUInt64)currentMaterial;
			}
		}
	}

	return DRG_MATERIAL_INVALID_NUM;
}

// GetMaterialVertexAttributes
drgUInt64 drgMaterialManager::GetMaterialVertexAttributes(drgUInt64 materialNum)
{
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)m_HeaderBasicPtr;
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);

		int vertexShaderIndex = pool->m_Materials[materialNum]->m_ShaderIndex;
		return pool->m_Shaders[vertexShaderIndex]->m_VertexAttributes;
	}

	return 0;
}

drgUInt64 drgMaterialManager::GetMaterialVertexAttributes(const char *name)
{
	drgUInt64 num = GetMaterialNumByName(name);
	return GetMaterialVertexAttributes(num);
}

void drgMaterialManager::FlushCache()
{
	m_CurrentEffectVtx = 0;
	m_CurrentEffectPxl = 0;
}

//
// drgMaterialManagerWEBGPU Methods
// These methods are meant to extend the material manager without #ifdef-ing all over the place
//

drgShaderDataWEBGPU *drgMaterialManagerWEBGPU::GetShaderData(int materialNum)
{
	drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)drgMaterialManager::m_HeaderBasicPtr;
	if (pool)
	{
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);
		int vertShaderIndex = pool->m_Materials[materialNum]->m_ShaderIndex;
		return pool->m_Shaders[vertShaderIndex];
	}

	return NULL;
}

drgShaderDataWEBGPU *drgMaterialManagerWEBGPU::GetShaderData(char *materialName)
{
	drgUInt64 num = drgMaterialManager::GetMaterialNumByName(materialName);
	return GetShaderData((int)num);
}

drgMaterialDataWEBGPU *drgMaterialManagerWEBGPU::GetMaterialData(int materialNum)
{
	drgMaterialPoolWEBGPU *pool = (drgMaterialPoolWEBGPU *)drgMaterialManager::m_HeaderBasicPtr;
	if (pool)
	{
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);
		return pool->m_Materials[materialNum];
	}

	return NULL;
}

drgMaterialDataWEBGPU *drgMaterialManagerWEBGPU::GetMaterialData(char *materialName)
{
	drgUInt64 num = drgMaterialManager::GetMaterialNumByName(materialName);
	return GetMaterialData((int)num);
}

void drgMaterialManagerWEBGPU::ApplyUniforms(drgMaterial *material, drgRenderContext *context)
{
	drgMaterialDataWEBGPU *materialData = GetMaterialData(material->GetMaterialNum());

	// Bind the world view projection matrix
	if (materialData->m_hWorldViewProjection >= 0)
	{
		drgMat44 *wvp = context->GetWorldViewProjectionMatrix();
		//		pglUniformMatrix4fv( materialData->m_hWorldViewProjection, 1, false, wvp->GetFloatPointer() );
		//		GL_ERROR( "Failed to apply the world view projection matrix uniform" );
	}

	// Bind the world matrix
	if (materialData->m_hWorld >= 0)
	{
		drgMat44 *world = context->GetWorldMatrix();
		//		pglUniformMatrix4fv( materialData->m_hWorld, 1, false, world->GetFloatPointer() );
		//		GL_ERROR( "Failed to apply the world matrix uniform" );
	}

	// Bind the bone matrices
	for (int currentBone = 0; currentBone < NUM_BONE_MATRICES; ++currentBone)
	{
		if (materialData->m_hBoneMatrix[currentBone] >= 0)
		{
			//			pglUniformMatrix4fv( materialData->m_hBoneMatrix[ currentBone ], 1, false, context->GetBoneMatrix( currentBone )->GetFloatPointer() );
			//			GL_ERROR( "Failed to apply the bone matrix uniform" );
		}
	}

	// Bind the ambient color
	if (materialData->m_hAmbientColor >= 0)
	{
		drgColor drgAmbientColor = context->GetAmbientColor();
		float ambient[] = {(float)drgAmbientColor.r / 255.0f, (float)drgAmbientColor.g / 255.0f, (float)drgAmbientColor.b / 255.0f, (float)drgAmbientColor.a / 255.0f};
		//		pglUniform4fv( materialData->m_hAmbientColor, 1, ambient );
		//		GL_ERROR( "Failed to apply the ambient color uniform" );
	}

	// Bind the specular color
	if (materialData->m_hSpecularLevel >= 0)
	{
		drgColor drgSpecularColor = context->GetSpecularColor();
		float spec[] = {(float)drgSpecularColor.r / 255.0f, (float)drgSpecularColor.g / 255.0f, (float)drgSpecularColor.b / 255.0f, (float)drgSpecularColor.a / 255.0f};
		//		pglUniform4fv( materialData->m_hSpecularLevel, 1, spec );
		//		GL_ERROR( "Failed to apply the specular color uniform" );
	}

	// Bind the diffuse color
	if (materialData->m_hDiffuseColor >= 0)
	{
		drgColor drgDiffuseColor = context->GetDiffuseColor();
		float diffuse[] = {(float)drgDiffuseColor.r / 255.0f, (float)drgDiffuseColor.g / 255.0f, (float)drgDiffuseColor.b / 255.0f, (float)drgDiffuseColor.a / 255.0f};
		//		pglUniform4fv( materialData->m_hDiffuseColor, 1, diffuse );
		//		GL_ERROR( "Failed to apply the diffuse color uniform" );
	}

	// Bind the texture offset
	if (materialData->m_hTexOffsetUV >= 0)
	{
		drgVec4 offset = context->GetTexCoordOffset();
		//		pglUniform4fv( materialData->m_hTexOffsetUV, 1, (float*)&offset );
		//		GL_ERROR( "Failed to apply the texture offset uniform" );
	}

	// Bind the view position
	if (materialData->m_hViewPos >= 0)
	{
		drgVec3 viewPos = context->GetViewPosition();
		//		pglUniform3fv( materialData->m_hViewPos, 1, (float*)&viewPos );
		//		GL_ERROR( "Failed to apply the view position uniform" );
	}

	// Bind the view direction
	if (materialData->m_hViewVec >= 0)
	{
		drgVec3 viewDir = context->GetViewDirection();
		//		pglUniform3fv( materialData->m_hViewVec, 1, (float*)&viewDir );
		//		GL_ERROR( "Failed to apply the view direction uniform" );
	}
}

#endif // USE_WEBGPU
