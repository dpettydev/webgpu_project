#ifndef __DRG_TEXTURE_H__
#define __DRG_TEXTURE_H__

#include "system/engine.h"
#include "util/dib/dib.h"
#include "core/file/file_async.h"

#define DRG_MAX_NUM_TEXTURES		1024

enum DRG_TEX_HARDWARE_TYPE
{
	DRG_TEX_HARDWARE_NOT_SET = 0,
	DRG_TEX_HARDWARE_UNKNOWN,
	DRG_TEX_HARDWARE_PVR,            // Listed by priority, highest is better
	DRG_TEX_HARDWARE_TEX,
	DRG_TEX_HARDWARE_ETC,
	DRG_TEX_HARDWARE_ATC,
	DRG_TEX_HARDWARE_DXT
};

enum DRG_TEX_FORMAT
{
	DRG_FMT_UNKNOWN = 0,
    DRG_FMT_R8G8B8,
    DRG_FMT_A8R8G8B8,
    DRG_FMT_X8R8G8B8,
    DRG_FMT_R5G6B5,
    DRG_FMT_X1R5G5B5,
    DRG_FMT_A1R5G5B5,
    DRG_FMT_A4R4G4B4,
    DRG_FMT_R3G3B2,
    DRG_FMT_A8,
    DRG_FMT_A8R3G3B2,
    DRG_FMT_X4R4G4B4,
    DRG_FMT_A2B10G10R10,
    DRG_FMT_A8B8G8R8,
    DRG_FMT_X8B8G8R8,
    DRG_FMT_G16R16,
    DRG_FMT_A2R10G10B10,
    DRG_FMT_A16B16G16R16,
	DRG_FMT_D24S8,
    DRG_FMT_PAL8,
	DRG_FMT_PAL4,
	DRG_FMT_MAX
};

enum DRG_TEX_ALPHA
{
	DRG_ALPHA_NONE		= (1<<0),
	DRG_ALPHA_CUTOUT	= (1<<1),
	DRG_ALPHA_BLEND		= (1<<2)
};

enum DRG_TEX_FLAGS
{
	DRG_TEX_NOTHING_FLAG			= 0,
	DRG_TEX_FLAGS_LOCKED			= (1 << 0),
	DRG_TEX_FLAGS_DYNAMIC			= (1 << 1),
	DRG_TEX_FLAGS_WAIT_FOR_UNLOCK	= (1 << 2),
	DRG_TEX_FLAGS_NEED_GPU_LOAD		= (1 << 3),
	DRG_TEX_FLAGS_RENDER_TARGET		= (1 << 4),
	DRG_TEX_FLAGS_LOADED			= (1 << 5),
	DRG_TEX_FLAGS_LOADED_HARDWARE	= (1 << 6)
};

class drgTexture
{
public:

	enum DRG_TEX_RENDER_MODE
	{
		RENDER_MODE_TEX0		= 0,
		RENDER_MODE_TEX1,
		RENDER_MODE_TEX2,
		RENDER_MODE_TEX3,
		RENDER_MODE_TEX_SHADOW,
		RENDER_MODE_TEX_SHADOW_ALT,
		RENDER_MODE_TEX_DECAL0,
		RENDER_MODE_TEX_DECAL1,
		RENDER_MODE_TEX_DECAL2
	};

	//! Increments the reference count for the texture
	inline void IncRef() {m_RefCount++;}

	//! Decrements the reference count for the texture
	inline void DecRef() {if(!m_RefCount) return; m_RefCount--; if(!m_RefCount) CleanUp();}

	inline void SetFlag( DRG_TEX_FLAGS flag ) {m_Flags|=flag;};

	void Create( DRG_TEX_FORMAT format );
	void CleanUp();
	void ReleaseGPUResources();
	void RecreateGPUResources();
	void* Lock( unsigned int* pitch=NULL );
	void UnLock();
	inline bool IsLocked() { return !!(m_Flags&DRG_TEX_FLAGS_LOCKED); };
	inline bool IsLoaded() { return !!(m_Flags&DRG_TEX_FLAGS_LOADED); };
	inline bool IsLoadedHdwr() { return !!(m_Flags&DRG_TEX_FLAGS_LOADED_HARDWARE); };
	inline bool NeedLoadHdwr() { return ((m_Flags&DRG_TEX_FLAGS_LOADED) && !(m_Flags&DRG_TEX_FLAGS_LOADED_HARDWARE)); };

	void UpdateData(bool update_asset=true);

	//! Gets the width of the texture
	inline unsigned int GetWidth() {return m_Width;}

	//! Gets the height of the texture
	inline unsigned int GetHeight() {return m_Height;}

	//! Gets the name of the texture
	inline const char *GetTextureName() {return m_TexName;};

	int GetSurface(int level, void *surface);

	void SetAsRenderTexture( DRG_TEX_RENDER_MODE mode=RENDER_MODE_TEX0 );

	static void* GetTextureGL(unsigned int idx);

	//! Finds a drgTexture given a texture name
	//! \param texname The name of the texture
	static drgTexture* FindTexture( const char* texname );

	//! Loads in a drgTexture given a file path
	//! \param Path to the texture file
	static drgTexture* LoadFromFile( const char* filename, bool async=false );
	static drgTexture* LoadFromMem( unsigned char* data, unsigned int data_len, const char* indexname, bool copy = true );
	static drgTexture* CreateTexture( const char* texname, DRG_TEX_FORMAT format, unsigned int width, unsigned int height, drgColor* col, void* texheader=NULL);
	static drgTexture* CreateRenderTargetTexture( unsigned int width, unsigned int height, DRG_TEX_FORMAT format );
	static void ReleaseAllGPUResources();
	static void RecreateAllGPUResources();

	//! Releases all textures in the global texture list
	static void ReleaseTextureList();

	//! Released all textures whose name name the given prefix
	static void ReleaseByPrefix(char* prefix);

	static DRG_TEX_HARDWARE_TYPE FindHardwareType();

	static void PrintLoaded();

	//! Gets the texture path
	inline const char *GetFullTexturePath() {return m_TexPath;};

	//! Gets the render list
	inline void* GetRenderList() {return m_RenderList;};

	//! Sets the render list
	inline void SetRenderList( void* list ) {m_RenderList=list;};

	//! Get the raw texture data
	inline void* GetTextureData() { return m_AssetData.GetFileMem(); }

	bool HasFormat( int format );

	static inline drgTexture* GetNullTexture()
	{
		if(m_NullTexture==NULL)
		{
			drgColor col(255,255,255,255);
			m_NullTexture = CreateTexture("null_texture", DRG_FMT_A8R8G8B8, 16, 16, &col);
		}
		return m_NullTexture;
	}

//protected:
	drgTexture(void);
	~drgTexture(void);

	//! Finds a location in the global texture list for a new texture
	static drgTexture* FindOpenTexture();

	drgDibInfo*			m_Dib;
#if USE_OPEN_GL
	unsigned int		m_Texture; // texture gl
#else
	void*				m_Texture; // texture webgpu
#endif
	void*				m_RenderList;

	unsigned int		m_Width; //!< The width of the texture
	unsigned int		m_Height; //!< The height of the texture

	int					m_Format; //!< Format of texture (e.g. size of each color channel)
	unsigned int		m_RefCount; //!< Number of references that point to this texture
	char				m_TexName[32]; //!< Name of the texture
	char				m_TexPath[DRG_MAX_PATH]; //!< File path of the texture
	unsigned short		m_TexIdx;
	unsigned short		m_Flags;

	static drgTexture*	m_NullTexture;
	static drgTexture**	m_GlobalTextureList;	//!< Array of all textures

	float				m_UScale;
	float				m_VScale;

	drgFileAsync		m_AssetData;

	friend class drgEngine;
	friend class drgMovie;

private:
	static bool IsNativeData(drgTexture* tex, unsigned char* data);
	static bool LoadNativeData(drgTexture* tex, unsigned char* data);
	static bool LoadDataFromDib( drgTexture* ret );
	static DRG_TEX_HARDWARE_TYPE	m_HardwareType;

	static void InternalLoadCB(void* data);
	static void InternalLoad(drgTexture* ret, const char* indexname);
	static void LoadDib( drgTexture* ret, unsigned char* data, unsigned int data_len, const char* indexname );
};

extern unsigned int GetPlatformTextureFormat( DRG_TEX_FORMAT format );

#endif // __DRG_TEXTURE_H__

