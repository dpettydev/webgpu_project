// MaterialManager.cpp
// This will define the functionality for a drgMaterialManager
#include "system/global.h"

#if USE_OPEN_GL

// Includes
#include "material_manager.h"
#include "material_manager_ogl.h"
#include "system/extern_inc.h"
#include "system/file.h"
#include "util/stringn.h"
#include "context.h"
#include "vertexbuff.h"
#include "render/context_ogl.h"

struct MaterialLoatVertData
{
	char name[64] = "";
	char name_file[256] = "";
	unsigned int atrib = 0;
};
const int g_NumMaterialVertData = 1;
MaterialLoatVertData g_MaterialVertData[g_NumMaterialVertData] = {
	{"VS_UI", "shader/opengl/ui.vs", DRG_VERTEX_TEXTURED_COLORED::attribs},
};

struct MaterialLoatFragData
{
	char name[64] = "";
	char name_file[256] = "";
};
const int g_NumMaterialFragData = 1;
MaterialLoatFragData g_MaterialFragData[g_NumMaterialFragData] = {
	{"PS_UI", "shader/opengl/ui.fs"},
};

struct MaterialLoatData
{
	char name_vert[256] = "";
	char name_frag[256] = "";
};
const int g_NumMaterialData = 1;
MaterialLoatData g_MaterialData[g_NumMaterialData] = {
	{"VS_UI", "PS_UI"},
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

inline void SetUpUniformVal(int *handle, unsigned int ShaderProgram, const char *uniform)
{
	int val = glGetUniformLocation(ShaderProgram, uniform);
	GL_ERROR("Failed to get uniform location in shader");
	drgPrintOut("%s  %i\n", uniform, val);
	(*handle) = val;
}

void SetUpUniforms(drgMaterialDataOGL *data, unsigned int ShaderProgram, const char *desc)
{
	// get handles to the global shader variables
	drgPrintOut("\nShader Uniforms %s\n", desc);
	SetUpUniformVal(&data->m_hWorldViewProjection, ShaderProgram, "g_mWorldViewProjection");
	SetUpUniformVal(&data->m_hWorld, ShaderProgram, "g_mWorld");
	SetUpUniformVal(&data->m_hBoneMatrix[0], ShaderProgram, "g_Wld0");
	SetUpUniformVal(&data->m_hBoneMatrix[1], ShaderProgram, "g_Wld1");
	SetUpUniformVal(&data->m_hBoneMatrix[2], ShaderProgram, "g_Wld2");
	SetUpUniformVal(&data->m_hBoneMatrix[3], ShaderProgram, "g_Wld3");
	SetUpUniformVal(&data->m_hBoneMatrix[4], ShaderProgram, "g_Wld4");
	SetUpUniformVal(&data->m_hBoneMatrix[5], ShaderProgram, "g_Wld5");
	SetUpUniformVal(&data->m_hBoneMatrix[6], ShaderProgram, "g_Wld6");
	SetUpUniformVal(&data->m_hBoneMatrix[7], ShaderProgram, "g_Wld7");
	SetUpUniformVal(&data->m_hAmbientColor, ShaderProgram, "g_AmbientColor");
	SetUpUniformVal(&data->m_hSpecularLevel, ShaderProgram, "g_SpecularLevel");
	SetUpUniformVal(&data->m_hDiffuseColor, ShaderProgram, "g_DiffuseColor");
	SetUpUniformVal(&data->m_hTexOffsetUV, ShaderProgram, "g_TexOffsetUV");
	SetUpUniformVal(&data->m_hViewPos, ShaderProgram, "g_ViewPos");
	SetUpUniformVal(&data->m_hViewVec, ShaderProgram, "g_ViewVec");

	data->m_SamplerLocations[0] = glGetUniformLocation(ShaderProgram, "Flat_TextureSampler0");
	data->m_SamplerLocations[1] = glGetUniformLocation(ShaderProgram, "Flat_TextureSampler1");
	data->m_SamplerLocations[2] = glGetUniformLocation(ShaderProgram, "Flat_TextureSampler2");
	data->m_SamplerLocations[3] = glGetUniformLocation(ShaderProgram, "Flat_TextureSampler3");

	data->m_ShadowMapSamplerLocation = glGetUniformLocation(ShaderProgram, "Flat_TextureSamplerShadowMap");
}

inline void SetUpAttribVal(int *handle, unsigned int ShaderProgram, char *attrib)
{
	int val = glGetAttribLocation(ShaderProgram, attrib);
	GL_ERROR("Failed to find the attribute location in the shader");
	if (val < 0)
	{
		*handle = 0;
		return;
	}

	drgPrintOut("%s  %i\n", attrib, val);
	(*handle) = val;
}

void SetUpAttribs(unsigned int ShaderProgram, drgMaterialDataOGL *data, const char *desc)
{
	drgPrintOut("\nShader Attributes %s\n", desc);
	SetUpAttribVal(&data->m_PositionAttribute, ShaderProgram, "MaudeInput_Position");
	SetUpAttribVal(&data->m_Color0Attribute, ShaderProgram, "MaudeInput_Color0");
	SetUpAttribVal(&data->m_Color1Attribute, ShaderProgram, "MaudeInput_Color1");
	SetUpAttribVal(&data->m_TexCoord0Attribute, ShaderProgram, "MaudeInput_TexUV0");
	SetUpAttribVal(&data->m_TexCoord1Attribute, ShaderProgram, "MaudeInput_TexUV0");
	SetUpAttribVal(&data->m_NormalAttribute, ShaderProgram, "MaudeInput_Normal");
	SetUpAttribVal(&data->m_TangentAttribute, ShaderProgram, "MaudeInput_Tangent");
	SetUpAttribVal(&data->m_Weight0Attribute, ShaderProgram, "MaudeInput_Weight0");
	SetUpAttribVal(&data->m_Weight1Attribute, ShaderProgram, "MaudeInput_Weight1");
	SetUpAttribVal(&data->m_NormalMorphAttribute, ShaderProgram, "MaudeInput_MorphNormal");
	SetUpAttribVal(&data->m_PosMorphAttribute, ShaderProgram, "MaudeInput_MorphPosition");
}

void LoadShader(drgShaderDataOGL *data, GLenum shaderType)
{
	int compiled = 0;
	char errormsg[512] = {0};

	data->m_GLShaderHandle = glCreateShader(shaderType);
	GL_ERROR("Failed to create a shader");
	assert(data->m_GLShaderHandle != 0);

	int sourceLen = drgString::Length(data->m_ShaderCode);
	const char *code = data->m_ShaderCode;
	glShaderSource((GLuint)data->m_GLShaderHandle, 1, &code, &sourceLen);
	GL_ERROR("Failed to load the shader source");

	data->m_ShaderCodeLength = sourceLen;

	glCompileShader(data->m_GLShaderHandle);
	glGetShaderiv((GLuint)data->m_GLShaderHandle, GL_COMPILE_STATUS, &compiled);
	if (compiled == 0)
	{
		glGetShaderInfoLog((GLuint)data->m_GLShaderHandle, 500, NULL, errormsg);
		drgPrintError("Failed to Compile %s, errors:\n\n%s\n", data->m_ShaderName, errormsg);
	}

	data->m_GLProgramHandle = 0;
}

// unsigned int LoadVertexAttributes( drgXMLNode* vertexDefNode )
// {
// 	assert( vertexDefNode->GetData()->IsEmpty() == false );

// 	const char* vertexAttributeNames[] =
// 	{
// 		"DRG_VERTEX_POSITION",
// 		"DRG_VERTEX_NORMAL",
// 		"DRG_VERTEX_TANGENT",
// 		"DRG_VERTEX_COLOR_0",
// 		"DRG_VERTEX_COLOR_1",
// 		"DRG_VERTEX_TEX_COORD_0",
// 		"DRG_VERTEX_TEX_COORD_1",
// 		"DRG_VERTEX_WEIGHT_0",
// 		"DRG_VERTEX_WEIGHT_1",
// 		"DRG_VERTEX_POS_MORPH",
// 		"DRG_VERTEX_NORMAL_MORPH",
// 	};

// 	const int vertexAttributeValues[] =
// 	{
// 		DRG_VERTEX_ATTRIB_POSITION,
// 		DRG_VERTEX_ATTRIB_NORMAL,
// 		DRG_VERTEX_ATTRIB_TANGENT,
// 		DRG_VERTEX_ATTRIB_COLOR0,
// 		DRG_VERTEX_ATTRIB_COLOR1,
// 		DRG_VERTEX_ATTRIB_TEX0,
// 		DRG_VERTEX_ATTRIB_TEX1,
// 		DRG_VERTEX_ATTRIB_WEIGHT0,
// 		DRG_VERTEX_ATTRIB_WEIGHT1,
// 		DRG_VERTEX_ATTRIB_POS_MORPH,
// 		DRG_VERTEX_ATTRIB_NORMAL_MORPH,
// 	};

// 	int numAttributes = sizeof( vertexAttributeValues ) / sizeof( int );
// 	unsigned int toReturn = 0;

// 	//
// 	// Walk the attribute list and determine our attributes
// 	//

// 	drgXMLNode attributeNode;
// 	drgXMLNode attributeTypeNode;
// 	for( unsigned int currentChild = 0; currentChild < vertexDefNode->GetData()->GetNumChild(); ++currentChild )
// 	{
// 		attributeNode = vertexDefNode->GetData()->GetChildNode( currentChild );
// 		assert( attributeNode->IsEmpty() == false );

// 		attributeTypeNode = attributeNode.GetData()->GetChildNode( "VERTEX_ATTRIBUTE_TYPE" );
// 		assert( attributeTypeNode->IsEmpty() == false );

// 		//
// 		// Find the vertex attribute type
// 		//

// 		const char* type = attributeTypeNode->GetText();
// 		bool foundAttribute = false;
// 		for( int currentAttribute = 0; currentAttribute < numAttributes; ++currentAttribute )
// 		{
// 			int length = drgString::Length( type );
// 			if( drgString::CompareNoCase( type, vertexAttributeNames[ currentAttribute ], length ) == 0 )
// 			{
// 				toReturn |= vertexAttributeValues[ currentAttribute ];
// 				foundAttribute = true;
// 				break;
// 			}
// 		}

// 		//
// 		// Make sure that we have found attribute.
// 		//

// 		if( foundAttribute == false )
// 		{
// 			drgDebug::Error( "Failed to find a vertex attribute matching %s", type );
// 		}
// 	}

// 	return toReturn;
// }

void LinkShaders(drgShaderDataOGL *vertShader, drgShaderDataOGL *pixelShader, drgMaterialDataOGL *materialData)
{
	assert(vertShader && "Tried to link a NULL vertex shader");
	assert(pixelShader && "Tried to link a NULL pixel shader");
	assert(materialData && "Tried to link a NULL material data");

	drgPrintOut("Link Shader %s\n", vertShader->m_ShaderName);

	int linked = 0;
	char errormsg[512] = {0};

	if (vertShader->m_GLShaderHandle && pixelShader->m_GLShaderHandle)
	{
		vertShader->m_GLProgramHandle = glCreateProgram();
		GL_ERROR("Failed to create the shader program");
		glAttachShader(vertShader->m_GLProgramHandle, vertShader->m_GLShaderHandle);
		GL_ERROR("Failed to attach the vertex shader to the program");
		glAttachShader(vertShader->m_GLProgramHandle, pixelShader->m_GLShaderHandle);
		GL_ERROR("Failed to attach the pixel shader to the program ");
		glLinkProgram(vertShader->m_GLProgramHandle);
		GL_ERROR("Failed to link the shader program");
		glGetProgramiv(vertShader->m_GLProgramHandle, GL_LINK_STATUS, &linked);
		GL_ERROR("Failed to get the link status");
		if (linked == 0)
		{
			glGetProgramInfoLog(vertShader->m_GLProgramHandle, 500, NULL, errormsg);
			drgPrintError("Failed to Link %s, errors:\n\n%s\n", vertShader->m_ShaderName, errormsg);
			vertShader->m_GLProgramHandle = 0;
		}
		else
		{
			glUseProgram(vertShader->m_GLProgramHandle);
			GL_ERROR("Failed to use the program");
			SetUpAttribs(vertShader->m_GLProgramHandle, materialData, vertShader->m_ShaderName);
			SetUpUniforms(materialData, vertShader->m_GLProgramHandle, vertShader->m_ShaderName);
		}
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

	// Allocate the material pool
	// drgUInt64* headerptr = (drgUInt64*)drgFile::MallocFile(filename);
	// assert( headerptr );

	// if(basic)
	//	m_HeaderBasicPtr = headerptr;
	// else
	//	m_HeaderExtPtr = headerptr;

	MEM_INFO_SET_NEW;
	drgMaterialPoolOGL *pool = new drgMaterialPoolOGL;

	// Now load vertex shader list
	pool->m_NumVertexShaders = g_NumMaterialVertData;

	// Get the vertex shader data
	assert(pool->m_NumVertexShaders != 0);
	pool->m_VertexShaders = (drgShaderDataOGL **)drgMemAllocZero(pool->m_NumVertexShaders * sizeof(drgShaderDataOGL *));
	for (int currentVertexShader = 0; currentVertexShader < pool->m_NumVertexShaders; ++currentVertexShader)
	{
		MEM_INFO_SET_NEW;
		pool->m_VertexShaders[currentVertexShader] = new drgShaderDataOGL;
		drgShaderDataOGL *shaderData = pool->m_VertexShaders[currentVertexShader];

		int nameLen = drgString::Length(g_MaterialVertData[currentVertexShader].name);
		shaderData->m_ShaderName = (char *)drgMemAllocZero(nameLen + 1);
		drgString::CopySafe(shaderData->m_ShaderName, g_MaterialVertData[currentVertexShader].name, nameLen);

		drgAssetReader shader_reader;
		shader_reader.OpenFile(g_MaterialVertData[currentVertexShader].name_file);
		unsigned int sourceLen = shader_reader.GetFileSize();

		shaderData->m_ShaderCode = (char *)drgMemAllocZero(sourceLen + 1);
		drgString::CopySafe(shaderData->m_ShaderCode, (char *)shader_reader.GetFileMem(), sourceLen);

		// Load the shader
		LoadShader(shaderData, GL_VERTEX_SHADER);

		// Determine the vertex attributes
		// shaderData->m_VertexAttributes = LoadVertexAttributes(&attributeNode);
		shaderData->m_VertexAttributes = g_MaterialVertData[currentVertexShader].atrib;
	}

	// Now load pixel shader list
	pool->m_NumPixelShaders = g_NumMaterialFragData;

	// Get the pixel shader data
	assert(pool->m_NumPixelShaders != 0);
	pool->m_PixelShaders = (drgShaderDataOGL **)drgMemAllocZero(pool->m_NumPixelShaders * sizeof(drgShaderDataOGL *));
	for (int currentPixelShader = 0; currentPixelShader < pool->m_NumPixelShaders; ++currentPixelShader)
	{
		MEM_INFO_SET_NEW;
		pool->m_PixelShaders[currentPixelShader] = new drgShaderDataOGL;
		drgShaderDataOGL *shaderData = pool->m_PixelShaders[currentPixelShader];

		int nameLen = drgString::Length(g_MaterialFragData[currentPixelShader].name);
		shaderData->m_ShaderName = (char *)drgMemAllocZero(nameLen + 1);
		;
		drgString::CopySafe(shaderData->m_ShaderName, g_MaterialFragData[currentPixelShader].name, nameLen);

		drgAssetReader shader_reader;
		shader_reader.OpenFile(g_MaterialFragData[currentPixelShader].name_file);
		unsigned int sourceLen = shader_reader.GetFileSize();

		shaderData->m_ShaderCode = (char *)drgMemAllocZero(sourceLen + 1);
		;
		drgString::CopySafe(shaderData->m_ShaderCode, (char *)shader_reader.GetFileMem(), sourceLen);

		shaderData->m_VertexAttributes = 0;

		// Load the shader
		LoadShader(shaderData, GL_FRAGMENT_SHADER);
	}

	// Now we need to load all the materials
	pool->m_NumMaterials = g_NumMaterialData;

	// Get the material data
	assert(pool->m_NumMaterials != 0);
	pool->m_Materials = (drgMaterialDataOGL **)drgMemAllocZero(pool->m_NumMaterials * sizeof(drgMaterialDataOGL *));
	for (int currentMaterial = 0; currentMaterial < pool->m_NumMaterials; ++currentMaterial)
	{
		MEM_INFO_SET_NEW;
		pool->m_Materials[currentMaterial] = new drgMaterialDataOGL;
		drgMaterialDataOGL *materialData = pool->m_Materials[currentMaterial];

		materialData->m_VertexShaderName = g_MaterialData[currentMaterial].name_vert;
		materialData->m_PixelShaderName = g_MaterialData[currentMaterial].name_frag;
		materialData->m_PixelShaderIndex = materialData->m_VertexShaderIndex = -1;

		// Find the shader indices
		for (int currentVertexShader = 0; currentVertexShader < pool->m_NumVertexShaders; ++currentVertexShader)
		{
			if (drgString::Compare(materialData->m_VertexShaderName, pool->m_VertexShaders[currentVertexShader]->m_ShaderName) == 0)
			{
				materialData->m_VertexShaderIndex = currentVertexShader;
			}
		}

		assert(materialData->m_VertexShaderIndex != -1);

		for (int currentPixelShader = 0; currentPixelShader < pool->m_NumPixelShaders; ++currentPixelShader)
		{
			if (drgString::Compare(materialData->m_PixelShaderName, pool->m_PixelShaders[currentPixelShader]->m_ShaderName) == 0)
			{
				materialData->m_PixelShaderIndex = currentPixelShader;
			}
		}

		assert(materialData->m_PixelShaderIndex != -1);

		// Now setup the program for the shaders
		drgShaderDataOGL *vertShader = pool->m_VertexShaders[materialData->m_VertexShaderIndex];
		drgShaderDataOGL *pixelShader = pool->m_PixelShaders[materialData->m_PixelShaderIndex];

		LinkShaders(vertShader, pixelShader, materialData);
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
		drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)m_HeaderBasicPtr;
		if (pool)
		{
			// Release our vertex shaders
			for (int currentVertexShader = 0; currentVertexShader < pool->m_NumVertexShaders; ++currentVertexShader)
			{
				drgShaderDataOGL *vertShader = pool->m_VertexShaders[currentVertexShader];
				if (vertShader == NULL)
					continue;

				if (vertShader->m_GLShaderHandle != 0)
				{
					glDeleteShader(vertShader->m_GLShaderHandle);
					vertShader->m_GLShaderHandle = 0;
				}

				if (vertShader->m_GLProgramHandle != 0)
				{
					glDeleteProgram(vertShader->m_GLProgramHandle);
					vertShader->m_GLProgramHandle = 0;
				}
			}

			// Release our pixel shaders
			for (int currentPixelShader = 0; currentPixelShader < pool->m_NumPixelShaders; ++currentPixelShader)
			{
				drgShaderDataOGL *pixelShader = pool->m_PixelShaders[currentPixelShader];
				if (pixelShader == NULL)
					continue;

				if (pixelShader->m_GLShaderHandle != 0)
				{
					glDeleteShader(pixelShader->m_GLShaderHandle);
					pixelShader->m_GLShaderHandle = 0;
				}

				if (pixelShader->m_GLProgramHandle != 0)
				{
					glDeleteProgram(pixelShader->m_GLProgramHandle);
					pixelShader->m_GLProgramHandle = 0;
				}
			}
		}
	}
}

void drgMaterialManager::RecreateGPUResources()
{
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)m_HeaderBasicPtr;
		if (pool)
		{
			// Recreate our vertex shaders
			for (int currentVertexShader = 0; currentVertexShader < pool->m_NumVertexShaders; ++currentVertexShader)
			{
				drgShaderDataOGL *vertShader = pool->m_VertexShaders[currentVertexShader];
				if (vertShader == NULL)
					continue;

				LoadShader(vertShader, GL_VERTEX_SHADER);
			}

			// Recreate our pixel shaders
			for (int currentPixelShader = 0; currentPixelShader < pool->m_NumPixelShaders; ++currentPixelShader)
			{
				drgShaderDataOGL *pixelShader = pool->m_PixelShaders[currentPixelShader];
				if (pixelShader == NULL)
					continue;

				LoadShader(pixelShader, GL_FRAGMENT_SHADER);
			}

			// Recreate our shader programs
			for (int currentMaterial = 0; currentMaterial < pool->m_NumMaterials; ++currentMaterial)
			{
				drgMaterialDataOGL *materialData = pool->m_Materials[currentMaterial];
				if (materialData == NULL)
					continue;

				// Now setup the program for the shaders
				drgShaderDataOGL *vertShader = pool->m_VertexShaders[materialData->m_VertexShaderIndex];
				drgShaderDataOGL *pixelShader = pool->m_PixelShaders[materialData->m_PixelShaderIndex];

				LinkShaders(vertShader, pixelShader, materialData);
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

//
// CreateVertDefs
//

void drgMaterialManager::CreateVertDefs()
{
}

//
// Cleanup
//

void drgMaterialManager::Cleanup()
{
	// declarations
	if (m_HeaderBasicPtr)
	{
		drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)m_HeaderBasicPtr;
		if (pool)
		{
			SAFE_FREE_ARRAY_OF_NEWS(pool->m_Materials, pool->m_NumMaterials);
			SAFE_FREE_ARRAY_OF_NEWS(pool->m_VertexShaders, pool->m_NumVertexShaders);
			SAFE_FREE_ARRAY_OF_NEWS(pool->m_PixelShaders, pool->m_NumPixelShaders);

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
		drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)m_HeaderBasicPtr;
		for (int currentMaterial = 0; currentMaterial < pool->m_NumMaterials; ++currentMaterial)
		{
			if (drgString::CompareNoCase(name, pool->m_Materials[currentMaterial]->m_VertexShaderName, drgString::Length(name)) == 0)
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
		drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)m_HeaderBasicPtr;
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);

		int vertexShaderIndex = pool->m_Materials[materialNum]->m_VertexShaderIndex;
		return pool->m_VertexShaders[vertexShaderIndex]->m_VertexAttributes;
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

// drgMaterialManagerOGL Methods
// These methods are meant to extend the material manager without #ifdef-ing all over the place
drgShaderDataOGL *drgMaterialManagerOGL::GetVertexShaderData(int materialNum)
{
	drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)drgMaterialManager::m_HeaderBasicPtr;
	if (pool)
	{
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);
		int vertShaderIndex = pool->m_Materials[materialNum]->m_VertexShaderIndex;
		return pool->m_VertexShaders[vertShaderIndex];
	}

	return NULL;
}

drgShaderDataOGL *drgMaterialManagerOGL::GetVertexShaderData(char *materialName)
{
	drgUInt64 num = drgMaterialManager::GetMaterialNumByName(materialName);
	return GetVertexShaderData((int)num);
}

drgShaderDataOGL *drgMaterialManagerOGL::GetPixelShaderData(int materialNum)
{
	drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)drgMaterialManager::m_HeaderBasicPtr;
	if (pool)
	{
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);
		int pixelShaderIndex = pool->m_Materials[materialNum]->m_PixelShaderIndex;
		return pool->m_PixelShaders[pixelShaderIndex];
	}

	return NULL;
}

drgShaderDataOGL *drgMaterialManagerOGL::GetPixelShaderData(char *materialName)
{
	drgUInt64 num = drgMaterialManager::GetMaterialNumByName(materialName);
	return GetPixelShaderData((int)num);
}

drgMaterialDataOGL *drgMaterialManagerOGL::GetMaterialData(int materialNum)
{
	drgMaterialPoolOGL *pool = (drgMaterialPoolOGL *)drgMaterialManager::m_HeaderBasicPtr;
	if (pool)
	{
		assert(materialNum >= 0 && materialNum < pool->m_NumMaterials);
		return pool->m_Materials[materialNum];
	}

	return NULL;
}

drgMaterialDataOGL *drgMaterialManagerOGL::GetMaterialData(char *materialName)
{
	drgUInt64 num = drgMaterialManager::GetMaterialNumByName(materialName);
	return GetMaterialData((int)num);
}

void drgMaterialManagerOGL::ApplyUniforms(drgMaterial *material, drgRenderContext *context)
{
	drgMaterialDataOGL *materialData = GetMaterialData(material->GetMaterialNum());

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

#endif // USE_OPEN_GL
