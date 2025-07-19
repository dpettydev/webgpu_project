// drgMaterial(WEBGPU)

#include "system/global.h"

#if USE_WEBGPU

// Includes
#include "material.h"
#include "material_manager.h"
#include "vertexbuff.h"
#include "texture.h"

#include "system/extern_inc.h"

drgMaterial::drgMaterial()
{
	drgMemory::MemSet(m_MaterialData.m_Texture, 0, sizeof(drgTexture*)*DRG_MAX_MAT_TEXTURES);
	drgMemory::MemSet(m_MaterialData.m_Decal, 0, sizeof(drgUInt64)*DRG_MAX_MAT_DECALS);
	drgMemory::MemSet(m_MaterialData.m_DecalIndex, 0, sizeof(drgUInt16)*DRG_MAX_MAT_DECALS);
	m_MaterialData.m_VertDeclNum = 0;
	m_MaterialData.m_EffectNum = 0;
	m_MaterialData.m_VertexAttributes = 0;
	m_MaterialData.m_NumTextures = 0;
	m_MaterialData.m_NumDecals = 0;
	m_MaterialData.m_ShadowEffectNum = -1;
}

drgMaterial::~drgMaterial()
{

}

bool drgMaterial::StartRender( drgRenderContext* _context)
{	
	drgUInt64* effectNum = (drgUInt64*)&m_MaterialData.m_EffectNum;
	if (drgMaterialManager::m_RenderShadows == PASS_MODE_SHADOW) {
		effectNum = (drgUInt64*)m_MaterialData.m_ShadowEffectNum;
	}
	unsigned int cureffect = 0;

	// Get our shader data
	// drgShaderDataWEBGPU* data = drgMaterialManagerWEBGPU::GetVertexShaderData( (int)(*effectNum) );
	// if( data == NULL ) {
	// 	drgPrintWarning("Tried to bind a material with invalid shader data");
	// 	return false;
	// }

	drgMaterialDataWEBGPU* platformMaterialData = drgMaterialManagerWEBGPU::GetMaterialData( (int)*effectNum );
	if( platformMaterialData == NULL ) {
		drgPrintWarning("Tried to bind a material with invalid material data");
		return false;
	}

	// Make sure we have a valid program handle
	//cureffect = data->m_GLProgramHandle;
	//if (cureffect == 0) {
	//	return false;
	//}

	// Bind our program handle if it's different from the last used program
	//if( drgMaterialManager::m_CurrentEffectVtx != cureffect )
	{
//		pglUseProgram(cureffect);		GL_ERROR( "Failed to set the program as active" );
	}
	
	
	drgRenderContext_WEBGPU* context = (drgRenderContext_WEBGPU*)_context;

	// We need to bind attributes and uniforms here.
	if( context ) {
		drgMaterialManagerWEBGPU::ApplyUniforms( this, context );
	}

	// Bind our textures
	int numSamplers = DRG_MIN( m_MaterialData.m_NumTextures, NUM_SAMPLERS );
	for( int currentTexture = 0; currentTexture < numSamplers; ++currentTexture ) {
		WGPUBindGroup TextureBindGroup = NULL;

		// Set our sampler location
//		pglUniform1i( platformMaterialData->m_SamplerLocations[ currentTexture ], currentTexture );								GL_ERROR( "Failed to set the texture index uniform location" );

		// Determine our texture handle
		if( m_MaterialData.m_Texture[ currentTexture ] != 0 ) {
			drgTexture* texture = (drgTexture*)m_MaterialData.m_Texture[ currentTexture ];
			TextureBindGroup = (WGPUBindGroup)drgTexture::GetTextureGL(texture->m_TexIdx);
			wgpuRenderPassEncoderSetBindGroup(context->m_RenderPass, 0, TextureBindGroup, 0, NULL);
//			glTextureHandle = drgTexture::GetTextureGL( texture->m_TexIdx );													GL_ERROR( "Failed to get the OpenGL texture handle" );
//			pglActiveTexture(GL_TEXTURE0 + currentTexture);																		GL_ERROR("Failed making the texture active");
		}

//		glBindTexture( GL_TEXTURE_2D, glTextureHandle );																		GL_ERROR( "Failed to bind the texture" );

//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);													GL_ERROR("Fail to set texture parameter!");
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);													GL_ERROR("Fail to set texture parameter!");
	}

	// Setup our cache.
	drgMaterialManager::m_CurrentEffectVtx		= cureffect;
	drgMaterialManager::m_CurrentEffectPxl		= cureffect;
	drgMaterialManager::m_PassBegin				= true;
	drgMaterialManager::m_CurrentVertDeclNum	= m_MaterialData.m_VertDeclNum;
	return true;
}

void drgMaterial::StopRender()
{
}

void drgMaterial::InitMaterial()
{
}

void drgMaterial::SetEffectByName( const char* name )
{
	m_MaterialData.m_EffectNum = drgMaterialManager::GetMaterialNumByName( name );

	// Things to check: 
	// Make sure the material count is the actual number of materials.
	// Make sure the material name is spelt correctly.

	assert( m_MaterialData.m_EffectNum != DRG_MATERIAL_INVALID_NUM && "Tried to set an effect name that isn't in the material list" );

	m_MaterialData.m_VertexAttributes = drgMaterialManager::GetMaterialVertexAttributes( m_MaterialData.m_EffectNum );
}

void drgMaterial::SetEffectByID( int materialNumber )
{
	m_MaterialData.m_EffectNum = materialNumber;
	m_MaterialData.m_VertexAttributes = drgMaterialManager::GetMaterialVertexAttributes( m_MaterialData.m_EffectNum );
}

void drgMaterial::SetShadowEffectByName( char* name )
{
	m_MaterialData.m_ShadowEffectNum = drgMaterialManager::GetMaterialNumByName(name);
}

void drgMaterial::SetTextureAddr( unsigned int texnum, void* texture )
{
	assert( texnum < DRG_MAX_MAT_TEXTURES );
	if( texnum >= m_MaterialData.m_NumTextures )
		m_MaterialData.m_NumTextures = (texnum + 1);

	m_MaterialData.m_Texture[ texnum ] = (drgUInt64)texture;
}

bool drgMaterial::SetRenderTextures()
{
	// TODO: Add in a check here based on the current render pass to not setup textures.
	
	for(int count = 0; count<m_MaterialData.m_NumTextures; ++count)
	{
		if(!m_MaterialData.m_Texture[count])
			return false;

#if DRG_USE_NULL_TEXURES
		if((m_RenderShadows==SHADOW_MODE_OFF) && (count==1))
		{
			((drgTexture*)m_MaterialData.m_Texture[count])->SetAsRenderTexture((drgTexture::DRG_TEX_RENDER_MODE)count);
		}
		else
		{
			drgTexture::GetNullTexture()->SetAsRenderTexture((drgTexture::DRG_TEX_RENDER_MODE)count);
		}

#else
		if( drgMaterialManager::m_RenderShadows != PASS_MODE_SHADOW )
		{
			((drgTexture*)m_MaterialData.m_Texture[count])->SetAsRenderTexture((drgTexture::DRG_TEX_RENDER_MODE)count);
		}
		else if( count == 0 )
		{
			if( drgMaterialManager::m_RenderShadows == PASS_MODE_SHADOW )
				((drgTexture*)m_MaterialData.m_Texture[count])->SetAsRenderTexture((drgTexture::DRG_TEX_RENDER_MODE)count);
		}
#endif
	}
	return true;
}

void drgMaterial::CopyMaterial( drgMaterial* source )
{
	drgMemory::MemCopy( &m_MaterialData, source, sizeof(drgMaterialData) );
}

#endif  // USE_WEBGPU
