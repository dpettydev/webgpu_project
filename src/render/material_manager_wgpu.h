// material_manager_wgpu.h
// This file is used to shared any webgpu specific material information with materials

#ifndef _DRG_MATERIAL_MANAGER_WEB_GPU_H_
#define _DRG_MATERIAL_MANAGER_WEB_GPU_H_

#include "system/global.h"

#if USE_WEBGPU

#include "vertexbuff.h"

#define NUM_SAMPLERS 4

// Declarations
class drgShaderDataWEBGPU
{
public:
	void *m_GLShaderHandle;
	int m_GLProgramHandle;
	int m_ShaderCodeLength;
	int m_VertexAttributes;
	char *m_ShaderCode;
	char *m_ShaderName;

	drgShaderDataWEBGPU()
	{
		m_GLShaderHandle = 0;
		m_GLProgramHandle = 0;
		m_ShaderCodeLength = 0;
		m_VertexAttributes = 0;
		m_ShaderCode = NULL;
		m_ShaderName = NULL;
	}

	~drgShaderDataWEBGPU()
	{
		SAFE_FREE(m_ShaderCode);
		SAFE_FREE(m_ShaderName);
	}
};

struct drgMaterialDataWEBGPU
{
	const char *m_ShaderName;
	int m_ShaderIndex;

	int m_VertexAttributes;

	int m_PositionAttribute;
	int m_NormalAttribute;
	int m_Color0Attribute;
	int m_Color1Attribute;
	int m_TexCoord0Attribute;
	int m_TexCoord1Attribute;
	int m_Weight0Attribute;
	int m_Weight1Attribute;
	int m_TangentAttribute;
	int m_PosMorphAttribute;
	int m_NormalMorphAttribute;

	int m_hWorldViewProjection;
	int m_hWorld;
	int m_hBoneMatrix[NUM_BONE_MATRICES];
	int m_hAmbientColor;
	int m_hSpecularLevel;
	int m_hDiffuseColor;
	int m_hTexOffsetUV;
	int m_hViewPos;
	int m_hViewVec;

	int m_SamplerLocations[NUM_SAMPLERS];
	int m_ShadowMapSamplerLocation;
};

struct drgMaterialPoolWEBGPU
{
	int m_NumShaders;
	int m_NumMaterials;
	drgShaderDataWEBGPU **m_Shaders;
	drgMaterialDataWEBGPU **m_Materials;
};

class drgMaterialManagerWEBGPU
{
public:
	static drgShaderDataWEBGPU *GetShaderData(int materialNum);
	static drgShaderDataWEBGPU *GetShaderData(char *materialName);

	static drgMaterialDataWEBGPU *GetMaterialData(int materialNum);
	static drgMaterialDataWEBGPU *GetMaterialData(char *materialName);

	static void ApplyUniforms(drgMaterial *material, drgRenderContext *context);
};

#endif // USE_WEBGPU

#endif // _DRG_MATERIAL_MANAGER_WEB_GPU_H_
