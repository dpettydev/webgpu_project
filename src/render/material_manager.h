// drgMaterialManager
// This class is responsible for managing shared material resources and global material information

#ifndef _DRG_MATERIAL_MANAGER_H_
#define _DRG_MATERIAL_MANAGER_H_

#include "system/global.h"
#include "render/material.h"
#include "render/material_manager_ogl.h"
#include "render/material_manager_wgpu.h"

#define DRG_MATERIAL_INVALID_NUM 1000000

class drgMaterialManager
{
public:
	static void Init(const char *materialPoolFilepath);
	static void LoadMaterial(const char *filename, bool basic = false);
	static void ReleaseGPUResources();
	static void RecreateGPUResources();
	static void ReloadMaterials();
	static void StopRenderForce();
	static void CreateVertDefs();
	static void Cleanup();
	static const char *GetMaterialName(unsigned int idx);
	static drgMaterial *FindMaterial(char *matname);

	static inline void EnableRenderShadows(drgMaterial::DRG_MATERIAL_PASS_MODE enable) { m_RenderShadows = enable; }
	static inline void EnableRenderDepth(bool enable) { m_RenderDepth = enable; }
	static inline bool IsRenderingShadows() { return (m_RenderShadows != drgMaterial::PASS_MODE_DIFFUSE); }
	static inline bool IsRenderingDepth() { return (m_RenderDepth); }

	static void FlushCache();

	static inline drgUInt32 GetCurrentVertDecl() { return (m_CurrentVertDeclNum); }

	// material effect
	static void LoadExternalMaterials(const char *filename);

	static drgUInt64 GetMaterialNumByName(const char *name);
	static drgUInt64 GetMaterialVertexAttributes(drgUInt64 materialNum);
	static drgUInt64 GetMaterialVertexAttributes(const char *name);

	//	private:
	static bool m_FirstInit;
	static bool m_PassBegin;
	static char m_MaterialNameBasic[DRG_MAX_PATH];
	static char m_MaterialNameExt[DRG_MAX_PATH];
	static void *m_HeaderBasicPtr;
	static void *m_HeaderExtPtr;
	static drgUInt32 m_CurrentVertDeclNum;
	static drgUInt64 m_CurrentEffectVtx;
	static drgUInt64 m_CurrentEffectPxl;
	static drgMaterial::DRG_MATERIAL_PASS_MODE m_RenderShadows;
	static bool m_RenderDepth;

	friend class drgMaterial;
#if USE_OPEN_GL
	friend class drgMaterialManagerOGL;
#endif
};

class drgMaterialListNode
{
public:
	drgMaterialListNode();
	~drgMaterialListNode();

	void ClearList();
	void SetNewMatPtr(drgUInt64 *mat, drgUInt64 *newmat);
	drgUInt64 *GetNewMatPtr(drgUInt64 *mat);
	drgUInt64 *AddMatPtr(drgUInt64 *mat);
	drgUInt64 *RemoveMatPtr(drgUInt64 *mat);
	drgMaterialListNode *FindMatPtr(drgUInt64 *mat);

protected:
	drgUInt64 *m_MatPtr;
	drgUInt64 *m_MatNewPtr;
	drgMaterialListNode *m_Next;
};

#endif // _DRG_MATERIAL_MANAGER_H_