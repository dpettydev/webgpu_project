// material_manager_ogl.h
// This file is used to shared any open gl specific material information with materials

#ifndef _DRG_MATERIAL_MANAGER_OGL_H_
#define _DRG_MATERIAL_MANAGER_OGL_H_

#include "system/global.h"

#if USE_OPEN_GL

#include "system/extern_inc.h"
#include "render/vertexbuff.h"

#define NUM_SAMPLERS 4

// Declarations
class drgShaderDataOGL
{
public:
	int m_GLShaderHandle;
	int m_GLProgramHandle;
	int m_ShaderCodeLength;
	int m_VertexAttributes;
	char *m_ShaderCode;
	char *m_ShaderName;

	drgShaderDataOGL()
	{
		m_GLShaderHandle = 0;
		m_GLProgramHandle = 0;
		m_ShaderCodeLength = 0;
		m_VertexAttributes = 0;
		m_ShaderCode = NULL;
		m_ShaderName = NULL;
	}

	~drgShaderDataOGL()
	{
		SAFE_FREE(m_ShaderCode);
		SAFE_FREE(m_ShaderName);
	}
};

struct drgMaterialDataOGL
{
	const char *m_VertexShaderName;
	const char *m_PixelShaderName;
	int m_VertexShaderIndex;
	int m_PixelShaderIndex;

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

struct drgMaterialPoolOGL
{
	int m_NumVertexShaders;
	int m_NumPixelShaders;
	int m_NumMaterials;
	drgShaderDataOGL **m_VertexShaders;
	drgShaderDataOGL **m_PixelShaders;
	drgMaterialDataOGL **m_Materials;
};

class drgMaterialManagerOGL
{
public:
	static drgShaderDataOGL *GetVertexShaderData(int materialNum);
	static drgShaderDataOGL *GetPixelShaderData(int materialNum);
	static drgShaderDataOGL *GetVertexShaderData(char *materialName);
	static drgShaderDataOGL *GetPixelShaderData(char *materialName);

	static drgMaterialDataOGL *GetMaterialData(int materialNum);
	static drgMaterialDataOGL *GetMaterialData(char *materialName);

	static void ApplyUniforms(drgMaterial *material, drgRenderContext *context);
};

#endif //  USE_OPEN_GL

#endif // _DRG_MATERIAL_MANAGER_OGL_H_
