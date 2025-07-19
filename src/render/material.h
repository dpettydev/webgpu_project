#ifndef __DRG_MATERIAL_H__
#define __DRG_MATERIAL_H__

#include "system/engine.h"

#define DRG_MAX_MATERIALS (512)
#define DRG_MAX_MAT_TEXTURES (6)
#define DRG_MAX_MAT_DECALS (2)

#define DRG_MATERIAL_DEFAULT 1

class drgRenderContext;

struct drgMaterialData
{
	drgUInt32 m_VertDeclNum;
#if (defined(_MAC) || defined(_IPHONE) || defined(_ANDROID))
	drgUInt32 pad1;
#endif
	drgUInt64 m_EffectNum;
	drgUInt64 m_VertexAttributes;
	drgUInt64 m_ShadowEffectNum;
	drgUInt16 m_NumTextures;
	drgUInt16 m_NumDecals;
#if (defined(_MAC) || defined(_IPHONE) || defined(_ANDROID))
	drgUInt32 pad2;
#endif
	drgUInt64 m_Texture[DRG_MAX_MAT_TEXTURES];
#if (defined(_MAC) || defined(_IPHONE) || defined(_ANDROID))
	drgUInt16 pad3;
#endif
	drgUInt64 m_Decal[DRG_MAX_MAT_DECALS];
	drgUInt16 m_DecalIndex[DRG_MAX_MAT_DECALS];
};

class drgMaterial
{
public:
	drgMaterial();
	~drgMaterial();

	enum DRG_MATERIAL_PASS_MODE
	{
		PASS_MODE_DIFFUSE = 0,
		PASS_MODE_SHADOW,
		PASS_MODE_DEPTH,
		PASS_MODE_MAX
	};

	bool StartRender(drgRenderContext *context);
	void StopRender();

	inline unsigned short GetNumTextures() { return m_MaterialData.m_NumTextures; };
	inline void SetNumTextures(unsigned short num) { m_MaterialData.m_NumTextures = num; };
	inline unsigned short GetNumDecals() { return m_MaterialData.m_NumDecals; };
	inline unsigned int GetVertexAttributes() { return (unsigned int)m_MaterialData.m_VertexAttributes; }

	void InitMaterial();
	void SetEffectByID(int materialNumber);
	void SetEffectByName(const char *name);
	void SetShadowEffectByName(char *name);
	void SetTextureAddr(unsigned int texnum, void *texture);
	inline void *GetTextureAddr(unsigned int texnum) { return (void *)m_MaterialData.m_Texture[texnum]; };
	bool SetRenderTextures();
	void CopyMaterial(drgMaterial *source);
	inline int GetMaterialNum() { return (int)m_MaterialData.m_EffectNum; }

protected:
	drgMaterialData m_MaterialData;
};

#endif // __DRG_MATERIAL_H__
