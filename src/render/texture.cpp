
#include "system/extern_inc.h"
#include "render/texture.h"
#include "system/file.h"
#include "util/filename.h"
#include "system/global.h"
#include "render/context.h"

drgTexture *drgTexture::m_NullTexture = NULL;
drgTexture **drgTexture::m_GlobalTextureList = NULL;
DRG_TEX_HARDWARE_TYPE drgTexture::m_HardwareType = DRG_TEX_HARDWARE_NOT_SET;

#if USE_OPEN_GL
int gl_formats[] =
	{
		-1,		  // DRG_FMT_UNKNOWN
		GL_RGB,	  // DRG_FMT_R8G8B8
		GL_RGBA,  // DRG_FMT_A8R8G8B8
		-1,		  // DRG_FMT_X8R8G8B8
		-1,		  // DRG_FMT_R5G6B5
		-1,		  // DRG_FMT_X1R5G5B5
		-1,		  // DRG_FMT_A1R5G5B5
		-1,		  // DRG_FMT_A4R4G4B4
		-1,		  // DRG_FMT_R3G3B2
		GL_ALPHA, // DRG_FMT_A8
		-1,		  // DRG_FMT_A8R3G3B2
		-1,		  // DRG_FMT_X4R4G4B4
		-1,		  // DRG_FMT_A2B10G10R10
		-1,		  // DRG_FMT_A8B8G8R8
		-1,		  // DRG_FMT_X8B8G8R8
		-1,		  // DRG_FMT_G16R16
		-1,		  // DRG_FMT_A2R10G10B10
		-1,		  // DRG_FMT_A16B16G16R16
		-1		  // DRG_FMT_PAL8
};

GLenum GetOpenGLImageFormat(GLenum internalFormat)
{
	switch (internalFormat)
	{
	case GL_RGB8:
		return GL_RGB;
	case GL_RGBA8:
		return GL_RGBA;
	case GL_DEPTH_STENCIL:
	case GL_DEPTH24_STENCIL8:
		return GL_DEPTH_STENCIL;
	default:
		drgPrintWarning("Provided an unrecognized internal format");
		// If you hit this, find the appropriate format and add it.
		return GL_RGB;
	}
}

GLenum GetOpenGLComponentType(GLenum internalFormat)
{
	switch (internalFormat)
	{
	case GL_RGB8:
	case GL_RGBA8:
		return GL_UNSIGNED_BYTE;
	case GL_DEPTH24_STENCIL8:
	case GL_DEPTH_STENCIL:
		return GL_UNSIGNED_INT_24_8;
	default:
		drgPrintWarning("Provided an unrecognized internal format");
		return GL_UNSIGNED_BYTE;
	}
}

#if defined(_WINDOWS)
extern HGLRC g_MainWindContext;
#elif defined(_LINUX)
#endif
#endif

#if defined(EMSCRIPTEN)
extern "C" void *drgLibDrawLoadTexture(unsigned char *pixels, int width, int height);
#endif

//! drgTexture constructor
drgTexture::drgTexture()
{
	m_RefCount = 0;
	m_RenderList = NULL;
	m_Dib = NULL;
	m_Format = DRG_FMT_UNKNOWN;
	m_Width = 0;
	m_Height = 0;
	m_Flags = 0;
	m_Texture = NULL;
}

//! drgTexture destructor
drgTexture::~drgTexture()
{
	CleanUp();
}

void *drgTexture::GetTextureGL(unsigned int idx)
{
	if (m_GlobalTextureList[idx]->NeedLoadHdwr())
		InternalLoad(m_GlobalTextureList[idx], m_GlobalTextureList[idx]->m_TexName);
	return (void *)m_GlobalTextureList[idx]->m_Texture;
}

//! Finds a drgTexture given a texture name
//! \param texname The name of the texture
drgTexture *drgTexture::FindTexture(const char *texname)
{
	if (m_GlobalTextureList == NULL)
		return NULL;

	for (unsigned int texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
	{
		if (m_GlobalTextureList[texcnt]->m_RefCount && (m_GlobalTextureList[texcnt]->m_Flags & DRG_TEX_FLAGS_RENDER_TARGET) == 0)
		{
			char tmpname[32];
			drgString::Copy(tmpname, texname, 32);
			tmpname[31] = '\0';
			if (!drgString::CompareNoCase(m_GlobalTextureList[texcnt]->m_TexName, tmpname))
				return (m_GlobalTextureList[texcnt]);
		}
	}

	return NULL;
}

//! Finds a location in the global texture list for a new texture
drgTexture *drgTexture::FindOpenTexture()
{
	unsigned int texcnt;
	if (m_GlobalTextureList == NULL)
	{
		m_GlobalTextureList = (drgTexture **)drgMemAlloc(sizeof(drgTexture *) * DRG_MAX_NUM_TEXTURES);
		drgMemory::MemSet(m_GlobalTextureList, 0, sizeof(drgTexture *) * DRG_MAX_NUM_TEXTURES);
		for (texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
		{
			MEM_INFO_SET_NEW;
			m_GlobalTextureList[texcnt] = new drgTexture();
		}
	}

	for (texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
	{
		if (!m_GlobalTextureList[texcnt]->m_RefCount)
		{
			m_GlobalTextureList[texcnt]->m_TexIdx = texcnt;
			return (m_GlobalTextureList[texcnt]);
		}
	}

	drgDebug::Notify("Could not find open texture slot, increase DRG_MAX_NUM_TEXTURES.  current: %i", DRG_MAX_NUM_TEXTURES);
	return NULL;
}

DRG_TEX_HARDWARE_TYPE drgTexture::FindHardwareType()
{
	// m_HardwareType = DRG_TEX_HARDWARE_TEX;
	if (m_HardwareType != DRG_TEX_HARDWARE_NOT_SET)
		return m_HardwareType;

	m_HardwareType = DRG_TEX_HARDWARE_UNKNOWN;

#ifdef _IPHONE
	m_HardwareType = DRG_TEX_HARDWARE_DXT;
#else
#if USE_OPEN_GL
	int numCompTypes = 0;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompTypes);
	GL_ERROR("Failed to get texture format!");
	int *pCompTypes = (int *)drgMemAlloc(numCompTypes * sizeof(int));
	glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, pCompTypes);
	GL_ERROR("Failed to get texture format!");
	for (int i = 0; i < numCompTypes; i++)
	{
		switch (pCompTypes[i])
		{
		case 0:
			break;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_RGB_S3TC:
		case GL_RGB4_S3TC:
		case GL_RGBA_S3TC:
		case GL_RGBA4_S3TC:
		case GL_RGBA_DXT5_S3TC:
		case GL_RGBA4_DXT5_S3TC:
			if (m_HardwareType < DRG_TEX_HARDWARE_DXT) // this is our most favorite
			{
				drgPrintOut("TEXTURE SUPPORT: DXT\r\n");
				m_HardwareType = DRG_TEX_HARDWARE_DXT;
			}
			break;
		case GL_COMPRESSED_RGB_FXT1_3DFX:
		case GL_COMPRESSED_RGBA_FXT1_3DFX:
			if (m_HardwareType < DRG_TEX_HARDWARE_TEX) // this is our least favorite
			{
				drgPrintOut("TEXTURE SUPPORT: FXT\r\n");
				m_HardwareType = DRG_TEX_HARDWARE_TEX;
			}
			break;
			break;
		case GL_PALETTE4_RGB8_OES:
		case GL_PALETTE4_RGBA8_OES:
		case GL_PALETTE4_R5_G6_B5_OES:
		case GL_PALETTE4_RGBA4_OES:
		case GL_PALETTE4_RGB5_A1_OES:
		case GL_PALETTE8_RGB8_OES:
		case GL_PALETTE8_RGBA8_OES:
		case GL_PALETTE8_R5_G6_B5_OES:
		case GL_PALETTE8_RGBA4_OES:
		case GL_PALETTE8_RGB5_A1_OES:
			// Not Used
			break;
		default:
			drgPrintOut("texture supported: %x\r\n", pCompTypes[i]);
			break;
		}
	}
	drgMemFree(pCompTypes);
#endif
#endif

	drgPrintOut("Done searching for texture types");
	return m_HardwareType;
}

void drgTexture::PrintLoaded()
{
#ifdef _DEBUG
	drgPrintOut("===Loaded Textures===\n");
	for (unsigned int i = 0; i < DRG_MAX_NUM_TEXTURES; ++i)
	{
		if (m_GlobalTextureList[i]->m_RefCount)
			drgPrintOut("%s, %i, %i, %i\n", m_GlobalTextureList[i]->m_TexName, m_GlobalTextureList[i]->m_Width, m_GlobalTextureList[i]->m_Height, m_GlobalTextureList[i]->m_Width * m_GlobalTextureList[i]->m_Height);
	}
	drgPrintOut("====End Loaded Tex===\n");
#endif
}

drgTexture *drgTexture::LoadFromMem(unsigned char *data, unsigned int data_len, const char *indexname, bool copy)
{
	drgTexture *ret = FindTexture(indexname);
	if (ret)
	{
		ret->IncRef();
		return ret;
	}

	ret = FindOpenTexture();
	drgString::Copy(ret->m_TexName, indexname, 31);
	ret->m_TexName[31] = '\0';
	drgString::Copy(ret->m_TexPath, indexname);

	if (copy)
	{
		ret->m_AssetData.OpenBuff(data_len);
		drgMemory::MemCopy(ret->m_AssetData.GetFileMem(), data, data_len);
	}
	else
	{
		ret->m_AssetData.OpenBuff(data, data_len);
	}

	InternalLoad(ret, indexname);
	ret->IncRef();

	return ret;
}

//! Loads in a drgTexture given a filename
//! \param filename The filename of the texture to load
#define OUTPUT_TEXTURES 0
#define OUTPUT_TO_HALF 0
#define REPLACE_TEX_NAME 0
char REPLACE_TO_TYPE[32] = ".atc";
drgTexture *drgTexture::LoadFromFile(const char *filename, bool async)
{
#if REPLACE_TEX_NAME
	string8 hdwr_filename = filename;
	if (m_HardwareType != DRG_TEX_HARDWARE_NOT_SET)
	{
		if (REPLACE_TO_HALF == true)
		{
			int assetpos = hdwr_filename.find("/Assets/") + strlen("/Assets/");
			hdwr_filename = hdwr_filename.substr(0, assetpos);
			hdwr_filename += "low_rez/";
			hdwr_filename += &filename[assetpos];
		}

		int tga_pos = hdwr_filename.find(".png");
		if (tga_pos == 0x7FFF)
			tga_pos = hdwr_filename.find(".PNG");
		if (tga_pos == 0x7FFF)
			tga_pos = hdwr_filename.find(".jpg");
		if (tga_pos == 0x7FFF)
			tga_pos = hdwr_filename.find(".JPG");
		if (tga_pos == 0x7FFF)
			tga_pos = hdwr_filename.find(".tga");
		if (tga_pos == 0x7FFF)
			tga_pos = hdwr_filename.find(".TGA");
		if (tga_pos < 0x7FFF)
		{
			hdwr_filename = hdwr_filename.substr(0, tga_pos);
			hdwr_filename += REPLACE_TO_TYPE;
			filename = hdwr_filename.c_str();
		}
	}
#endif

	// drgPrintOut("Load Texture: %s\r\n", filename);

	drgFilename texfile(filename);
	drgTexture *ret = FindTexture(texfile.GetFilename());
	if (ret)
	{
		ret->IncRef();
		return ret;
	}

	ret = FindOpenTexture();
	drgString::Copy(ret->m_TexName, texfile.GetFilename(), 31);
	ret->m_TexName[31] = '\0';
	drgString::Copy(ret->m_TexPath, filename);

	ret->IncRef();

#if 0 // ASYNC
	ret->m_AssetData.OpenFile(filename);
	InternalLoadCB(ret);
	InternalLoad(ret, ret->m_TexName);
#else
	if (async == true)
	{
		ret->m_AssetData.OpenAsync(filename, InternalLoadCB, ret, true);
	}
	else
	{
		ret->m_AssetData.OpenFile(filename, true);
		InternalLoadCB(ret);
	}
#endif
	return ret;
}

void drgTexture::InternalLoadCB(void *data)
{
	if (data == NULL)
	{
		drgPrintOut("Texture load async failed!");
		return;
	}
	drgTexture *ret = (drgTexture *)data;

	unsigned int data_len = ret->m_AssetData.GetFileSize();
	unsigned char *tex_data = ret->m_AssetData.GetFileMem();
	if (!IsNativeData(ret, tex_data))
	{
		if (ret->m_Dib == NULL)
		{
			LoadDib(ret, tex_data, data_len, ret->m_TexName);
			ret->m_AssetData.Close();
		}
	}

	// if(data!=NULL)
	//	drgMemFree(data);
	ret->m_Flags |= DRG_TEX_FLAGS_LOADED;
}

void drgTexture::InternalLoad(drgTexture *ret, const char *indexname)
{
	if (ret->m_Dib != NULL)
	{
		LoadDataFromDib(ret);
	}
	else
	{
		unsigned int data_len = ret->m_AssetData.GetFileSize();
		unsigned char *data = ret->m_AssetData.GetFileMem();
		if (!LoadNativeData(ret, data))
		{
			if (ret->m_Dib == NULL)
			{
				LoadDib(ret, data, data_len, indexname);
				ret->m_AssetData.Close();
			}
			LoadDataFromDib(ret);
		}
	}
	ret->m_Flags |= DRG_TEX_FLAGS_LOADED_HARDWARE;
}

void drgTexture::LoadDib(drgTexture *ret, unsigned char *data, unsigned int data_len, const char *indexname)
{
	MEM_INFO_SET_NEW;
	ret->m_Dib = new drgDibInfo(indexname);
	if (data == NULL)
	{
		ret->m_Dib->Init(1, 16, 16);
#if defined(RETAIL_BUILD)
		ret->m_Dib->FillRectRGB(0, 0, 0, 0, 255);
#else
		ret->m_Dib->FillRectRGB(0, 0, 255, 0, 255);
#endif
	}
	else
	{
		ret->m_Dib->ReadFromImageInMemory(data, data_len);
	}
	if (ret->m_Dib->Width(0) <= 0)
	{
		ret->m_Dib->Resize(16, 16);
#if defined(RETAIL_BUILD)
		ret->m_Dib->FillRectRGB(0, 0, 0, 0, 255);
#else
		ret->m_Dib->FillRectRGB(0, 0, 255, 0, 255);
#endif
	}

	if (!IsPowerOfTwo(ret->m_Dib->Width(0)) || !IsPowerOfTwo(ret->m_Dib->Height(0)))
		drgPrintOut("====  Texture Not Power Of 2! %s (%ix%i)  ====\n", indexname, ret->m_Dib->Width(0), ret->m_Dib->Height(0));

	ret->m_Width = ret->m_Dib->Width(0);
	ret->m_Height = ret->m_Dib->Height(0);
	ret->m_Format = DRG_FMT_A8R8G8B8;

	unsigned int tex_size = ((ret->m_Width * ret->m_Height * 4) / 1024) / 1024;
	if (tex_size > 2)
		drgPrintOut("Load Texture: (%uM) %s\r\n", (ret->m_Width * ret->m_Height * 4) / 1024, ret->m_AssetData.GetName());
}

drgTexture *drgTexture::CreateTexture(const char *texname, DRG_TEX_FORMAT format, unsigned int width, unsigned int height, drgColor *col, void *texheader)
{
	drgTexture *ret = FindTexture(texname);
	if (ret)
	{
		ret->IncRef();
		return ret;
	}

	ret = FindOpenTexture();
	drgString::Copy(ret->m_TexName, texname);
	drgString::Copy(ret->m_TexPath, "");

	ret->m_Dib = new drgDibInfo();
	ret->m_Dib->Init(1, width, height);
	ret->m_Dib->FillRectRGB(0, col->r, col->g, col->b, col->a);

	ret->m_Width = ret->m_Dib->Width(0);
	ret->m_Height = ret->m_Dib->Height(0);
	unsigned int data_len = ret->m_Dib->Width(0) * ret->m_Dib->Height(0) * 4;

	ret->m_AssetData.OpenBuff(data_len);
	drgMemory::MemCopy(ret->m_AssetData.GetFileMem(), ret->m_Dib->GetPixelPtr(0, 0), data_len);

	ret->m_Format = DRG_FMT_A8R8G8B8;

	InternalLoad(ret, texname);
	ret->IncRef();
	return ret;
}

drgTexture *drgTexture::CreateRenderTargetTexture(unsigned int width, unsigned int height, DRG_TEX_FORMAT format)
{
	drgTexture *texture = FindOpenTexture();
	assert(texture && "Out of textures!");

	texture->m_Width = width;
	texture->m_Height = height;
	texture->m_Format = format;
	texture->m_Dib = 0;
	texture->m_Flags |= DRG_TEX_FLAGS_RENDER_TARGET;
	texture->IncRef();

	// Make sure the primary context is bound.
#if USE_OPEN_GL
	// #if defined(_WINDOWS)
	// 	HGLRC oldRC = wglGetCurrentContext();
	// 	HDC oldDC = wglGetCurrentDC();
	// #endif
	drgEngine::NativeContextSetMain();

	// Create our gl texture handle
	glGenTextures(1, &texture->m_Texture);
	GL_ERROR("Failed to generate render target texture");
	glBindTexture(GL_TEXTURE_2D, texture->m_Texture);
	GL_ERROR("Failed to bind the render target texture when initializing");

	// Set the texture data parameters and some filtering data.
	// TODO: We need to pass in platform agnostic filtering information here.
	GLenum platformFormat = GetPlatformTextureFormat(format);
	GLenum imageFormat = GetOpenGLImageFormat(platformFormat);
	GLenum componentType = GetOpenGLComponentType(platformFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, platformFormat, width, height, 0, imageFormat, componentType, 0);
	GL_ERROR("Failed to set the texture image parameters for a render target texture");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL_ERROR("Failed to set the filtering parameters for a render target texture");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_ERROR("Failed to set the filtering parameters for a render target texture");

	// Rebind the previous context.
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_ERROR("Failed to rebind an empty texture after create render target texture");
// #if defined(_WINDOWS)
// 	wglMakeCurrent(oldDC, oldRC);
// #endif
#endif

	return texture;
}

//! Releases all textures in the global texture list
void drgTexture::ReleaseTextureList()
{
	if (m_GlobalTextureList == NULL)
		return;

	for (unsigned int texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
	{
		if (m_GlobalTextureList[texcnt]->m_RefCount)
			m_GlobalTextureList[texcnt]->CleanUp();
		delete m_GlobalTextureList[texcnt];
	}
	drgMemFree(m_GlobalTextureList);
	m_GlobalTextureList = NULL;
}

void drgTexture::ReleaseByPrefix(char *prefix)
{
	if (!m_GlobalTextureList)
		return;

	int prefixLength = drgString::Length(prefix);
	for (unsigned int texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
	{
		if (m_GlobalTextureList[texcnt]->m_RefCount)
		{
			if (drgString::Compare(m_GlobalTextureList[texcnt]->GetTextureName(), prefix, prefixLength) == 0)
				m_GlobalTextureList[texcnt]->CleanUp();
		}
	}
}

void drgTexture::Create(DRG_TEX_FORMAT format)
{
}

void drgTexture::UpdateData(bool update_asset)
{
	m_Width = m_Dib->Width(0);
	m_Height = m_Dib->Height(0);
	unsigned int data_len = m_Dib->Width(0) * m_Dib->Height(0) * 4;

	if (update_asset)
	{
		m_AssetData.OpenBuff(data_len);
		drgMemory::MemCopy(m_AssetData.GetFileMem(), m_Dib->GetPixelPtr(0, 0), data_len);
	}

	LoadDataFromDib(this);
}

//! Frees any resources used by the texture
void drgTexture::CleanUp()
{
#if USE_OPEN_GL
	if (m_Texture)
	{
#if defined(_WINDOWS)
		// HGLRC oldRC = wglGetCurrentContext();
		// HDC oldDC = wglGetCurrentDC();
		drgEngine::NativeContextSetMain();
		// if (wglGetCurrentContext() != NULL)
		{
			// glDeleteTextures(1, &m_Texture);
			// GL_ERROR("Failed to delete texture!");
		}
		// wglMakeCurrent(oldDC, oldRC);
#else
		drgEngine::NativeContextSetMain();
		glDeleteTextures(1, &m_Texture);
		GL_ERROR("Failed to delete texture!");
#endif
		m_Texture = 0;
	}
#endif

#if USE_WEBGPU
	if (m_Texture)
	{
		m_Texture = NULL;
	}
#endif

	SAFE_DELETE(m_Dib);
	m_AssetData.Close();

	m_Flags = 0;
	m_RefCount = 0;
	m_Format = DRG_FMT_UNKNOWN;
}

void drgTexture::ReleaseGPUResources()
{
#if USE_OPEN_GL
	glDeleteTextures(1, &m_Texture);
#endif
}

void drgTexture::RecreateGPUResources()
{
	if ((m_Dib != NULL) || (m_AssetData.GetFileMem() != NULL))
	{
		InternalLoad(this, m_TexPath);
	}
}

void drgTexture::ReleaseAllGPUResources()
{
	for (unsigned int texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
	{
		if (m_GlobalTextureList[texcnt]->m_RefCount)
		{
			m_GlobalTextureList[texcnt]->ReleaseGPUResources();
		}
	}
}

void drgTexture::RecreateAllGPUResources()
{
	for (unsigned int texcnt = 0; texcnt < DRG_MAX_NUM_TEXTURES; ++texcnt)
	{
		if (m_GlobalTextureList[texcnt]->m_RefCount)
		{
			m_GlobalTextureList[texcnt]->RecreateGPUResources();
		}
	}
}

void *drgTexture::Lock(unsigned int *pitch)
{
	void *ret = NULL;
	if (!m_Texture)
		return ret;
	if (pitch)
		(*pitch) = m_Width;
	ret = m_Dib->GetPixelPtr(0, 0);
	m_Flags |= DRG_TEX_FLAGS_LOCKED;
	return ret;
}

void drgTexture::UnLock()
{
	m_Flags &= (~DRG_TEX_FLAGS_LOCKED);
}

bool drgTexture::HasFormat(int format)
{
	return (m_Format == format);
}

void drgTexture::SetAsRenderTexture(DRG_TEX_RENDER_MODE mode)
{
	if (m_Flags & DRG_TEX_FLAGS_WAIT_FOR_UNLOCK)
		while (m_Flags & DRG_TEX_FLAGS_LOCKED)
		{
		}; // wait
	assert(!(m_Flags & DRG_TEX_FLAGS_LOCKED));
	// drgEngine::getInstance()->SetRenderTexture( m_Texture, mode, m_Width, m_Height, m_Format );
}

int drgTexture::GetSurface(int level, void *surface)
{
	return -1;
}

#if USE_OPEN_GL

bool drgTexture::LoadDataFromDib(drgTexture *ret)
{
#if USE_OPEN_GL
	// #if defined(_WINDOWS)
	// 	HGLRC oldRC = wglGetCurrentContext();
	// 	HDC oldDC = wglGetCurrentDC();
	// #endif
	drgEngine::NativeContextSetMain();
	if (ret->m_Texture == NULL)
	{
		glGenTextures(1, &ret->m_Texture);
	}
	GL_ERROR("Failed to generate texture!");
	glBindTexture(GL_TEXTURE_2D, ret->m_Texture);
	GL_ERROR("Failed to bind texture!");
	if (ret->m_Dib->HasFlag(0, DRG_DIB_FLAG_NO_ALPHA))
	{
		int count = ret->m_Dib->Width(0) * ret->m_Dib->Height(0) * 4;
		int size = ret->m_Dib->Width(0) * ret->m_Dib->Height(0) * 3;
		unsigned char *pixels_out = (unsigned char *)drgMemAlloc(size);
		unsigned char *pixels_in = (unsigned char *)ret->m_Dib->GetPixelPtr(0, 0);
		size--;
		count--;
		while (size >= 0)
		{
			count--;
			pixels_out[size--] = pixels_in[count--];
			pixels_out[size--] = pixels_in[count--];
			pixels_out[size--] = pixels_in[count--];
		}
#if USE_GLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ret->m_Dib->Width(0), ret->m_Dib->Height(0), 0, GL_RGB, GL_UNSIGNED_BYTE, pixels_out);
		GL_ERROR("Fail to set texure image!");
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, ret->m_Dib->Width(0), ret->m_Dib->Height(0), 0, GL_RGB, GL_UNSIGNED_BYTE, pixels_out);
		GL_ERROR("Fail to set texure image!");
#endif
		drgMemFree(pixels_out);
	}
	else
	{
#if USE_GLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret->m_Dib->Width(0), ret->m_Dib->Height(0), 0, GL_RGBA, GL_UNSIGNED_BYTE, ret->m_Dib->GetPixelPtr(0, 0));
		GL_ERROR("Fail to set texure image!");
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ret->m_Dib->Width(0), ret->m_Dib->Height(0), 0, GL_RGBA, GL_UNSIGNED_BYTE, ret->m_Dib->GetPixelPtr(0, 0));
		GL_ERROR("Fail to set texure image!");
#endif
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL_ERROR("Fail to set texture parameter!");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_ERROR("Fail to set texture parameter!");
	// #if defined(_WINDOWS)
	// 	wglMakeCurrent(oldDC, oldRC);
	// #endif

#elif defined(EMSCRIPTEN)
	ret->m_Texture = (unsigned int)drgLibDrawLoadTexture((unsigned char *)ret->m_Dib->GetPixelPtr(0, 0), ret->m_Dib->Width(0), ret->m_Dib->Height(0));
#endif

	return true;
}

bool drgTexture::IsNativeData(drgTexture *tex, unsigned char *data)
{
	if (data == NULL)
		return false;

	if ((m_HardwareType == DRG_TEX_HARDWARE_DXT) || (m_HardwareType == DRG_TEX_HARDWARE_ATC) || (m_HardwareType == DRG_TEX_HARDWARE_TEX))
	{
		DRG_DDS_HEADER *pHeader = (DRG_DDS_HEADER *)data;
		bool isDDSMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
		if (!isDDSMagicValid)
			return false;
		return true;
	}
	else if (m_HardwareType == DRG_TEX_HARDWARE_PVR)
	{
	}
	return false;
}

bool drgTexture::LoadNativeData(drgTexture *tex, unsigned char *data)
{
#ifdef _IPHONE
	return false;
#endif

	if (data == NULL)
		return false;

	if ((m_HardwareType == DRG_TEX_HARDWARE_DXT) || (m_HardwareType == DRG_TEX_HARDWARE_ATC) || (m_HardwareType == DRG_TEX_HARDWARE_TEX))
	{
		DRG_DDS_HEADER *pHeader = (DRG_DDS_HEADER *)data;
		bool isDDSMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
		if (!isDDSMagicValid)
			return false;

		// #if defined(_WINDOWS)
		// 		HGLRC oldRC = wglGetCurrentContext();
		// 		HDC oldDC = wglGetCurrentDC();
		// #endif
		drgEngine::NativeContextSetMain();
		glGenTextures(1, &tex->m_Texture);
		GL_ERROR("Failed to generate texture!");
		glBindTexture(GL_TEXTURE_2D, tex->m_Texture);
		GL_ERROR("Failed to bind texture!");

		unsigned int offset = 0;
		unsigned int width = pHeader->m_Width;
		unsigned int height = pHeader->m_Height;

		bool isMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
		bool isDXT1 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "DXT1", 4) == 0;
		bool isDXT5 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "DXT5", 4) == 0;
		bool isATC = memcmp(pHeader->m_FourCharIdOfPixelFormat, "ATC ", 4) == 0;
		bool isATCI = memcmp(pHeader->m_FourCharIdOfPixelFormat, "ATCI", 4) == 0;
		bool isTEX0 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "TEX0", 4) == 0;
		bool isTEX1 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "TEX1", 4) == 0;
		bool isTEX2 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "TEX2", 4) == 0;
		bool isValidDdsFile = isMagicValid && (isDXT1 || isDXT5 || isATC || isATCI || isTEX0 || isTEX1 || isTEX2);
		unsigned int format = 0;
		unsigned int blocksize = 16;
		unsigned int mip = 0;
		if (isTEX0 || isTEX1 || isTEX2)
		{
			unsigned int format2 = 0;
			if (isTEX0)
			{
				blocksize = 16;
				format = GL_RGB5;
				format2 = GL_UNSIGNED_SHORT_5_5_5_1_EXT;
			}
			else if (isTEX1)
			{
				blocksize = 16;
				format = GL_RGB5_A1;
				format2 = GL_UNSIGNED_SHORT_5_5_5_1_EXT;
			}
			else if (isTEX2)
			{
				blocksize = 16;
				format = GL_RGBA4;
				format2 = GL_UNSIGNED_SHORT_4_4_4_4_EXT;
			}
			do
			{
				unsigned int Size = width * height * 2;
				glTexImage2D(GL_TEXTURE_2D, mip, format, width, height, 0, GL_RGBA, format2, (data + sizeof(DRG_DDS_HEADER)) + offset);
				GL_ERROR("Fail to set texure image!");
				offset += Size;
				if ((width <<= 1) == 0)
					width = 1;
				if ((height <<= 1) == 0)
					height = 1;
				mip++;
			} while (mip < pHeader->m_MipMapCount);
		}
		else
		{
			if (isDXT1)
			{
				blocksize = 8;
				format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			}
			else if (isDXT5)
			{
				blocksize = 16;
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			}
			do
			{
				unsigned int Size = ((width + 3) >> 2) * ((height + 3) >> 2) * blocksize;
				glCompressedTexImage2D(GL_TEXTURE_2D, mip, format, width, height, 0, Size, (data + sizeof(DRG_DDS_HEADER)) + offset);
				GL_ERROR("Fail to set compressed texture!");
				offset += Size;
				if ((width <<= 1) == 0)
					width = 1;
				if ((height <<= 1) == 0)
					height = 1;
				mip++;
			} while (mip < pHeader->m_MipMapCount);
		}

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GL_ERROR("Fail to set texture parameter!");
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GL_ERROR("Fail to set texture parameter!");
#if defined(_WINDOWS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pHeader->m_MipMapCount - 1);
		GL_ERROR("Fail to set texture parameter!");
		// wglMakeCurrent(oldDC, oldRC);
#endif
		tex->m_Width = pHeader->m_Width;
		tex->m_Height = pHeader->m_Height;
		tex->m_Format = DRG_FMT_UNKNOWN;
		return true;
	}
	else if (m_HardwareType == DRG_TEX_HARDWARE_PVR)
	{
	}
	return false;
}

#elif USE_WEBGPU
bool drgTexture::LoadDataFromDib(drgTexture *ret)
{
	drgRenderContext_WEBGPU *webgpu_ctx = (drgRenderContext_WEBGPU *)drgEngine::NativeContextSetMain();

	unsigned int width = (unsigned int)ret->m_Dib->Width(0);
	unsigned int height = (unsigned int)ret->m_Dib->Height(0);
	WGPUTextureView texture_view;

	WGPUTextureDescriptor desc_texture = {
		.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
		.dimension = WGPUTextureDimension_2D,
		.size = {width, height, 1},
		.format = WGPUTextureFormat_RGBA8Unorm,
		.mipLevelCount = 1,
		.sampleCount = 1,
	};

	WGPUTexture texture = wgpuDeviceCreateTexture(webgpu_ctx->m_Device, &desc_texture);

	// upload pixels
	WGPUTexelCopyTextureInfo dst =
		{
			.texture = texture,
			.mipLevel = 0,
		};
	WGPUTexelCopyBufferLayout src =
		{
			.offset = 0,
			.bytesPerRow = width * 4, // 4 bytes per pixel
			.rowsPerImage = height,
		};
	int pixel_count = width * height * 4;
	wgpuQueueWriteTexture(webgpu_ctx->m_Queue, &dst, ret->m_Dib->GetPixelPtr(0, 0), pixel_count, &src, &desc_texture.size);

	WGPUTextureViewDescriptor view_desc =
		{
			.format = desc_texture.format,
			.dimension = WGPUTextureViewDimension_2D,
			.baseMipLevel = 0,
			.mipLevelCount = 1,
			.baseArrayLayer = 0,
			.arrayLayerCount = 1,
			.aspect = WGPUTextureAspect_All,
		};
	texture_view = wgpuTextureCreateView(texture, &view_desc);
	wgpuTextureRelease(texture);

	WGPUSampler sampler;
	WGPUSamplerDescriptor desc_samp =
		{
			.addressModeU = WGPUAddressMode_Repeat,
			.addressModeV = WGPUAddressMode_Repeat,
			.addressModeW = WGPUAddressMode_Repeat,
			.magFilter = WGPUFilterMode_Linear,
			.minFilter = WGPUFilterMode_Linear,
			.mipmapFilter = WGPUMipmapFilterMode_Linear,
			.lodMinClamp = 0.0f,
			.lodMaxClamp = 1.0f,
			.compare = WGPUCompareFunction_Undefined,
			.maxAnisotropy = 1,
		};
	sampler = wgpuDeviceCreateSampler(webgpu_ctx->m_Device, &desc_samp);

	WGPUBindGroupLayoutEntry bind_group_layout_entry[] = {{}, {}};
	// texture for fragment shader
	bind_group_layout_entry[0].binding = 0;
	bind_group_layout_entry[0].visibility = WGPUShaderStage_Fragment;
	bind_group_layout_entry[0].texture.sampleType = WGPUTextureSampleType_Float;
	bind_group_layout_entry[0].texture.viewDimension = WGPUTextureViewDimension_2D;
	bind_group_layout_entry[0].texture.multisampled = 0;

	// sampler for fragment shader
	bind_group_layout_entry[1].binding = 1;
	bind_group_layout_entry[1].visibility = WGPUShaderStage_Fragment;
	bind_group_layout_entry[1].sampler.type = WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bind_group_layout_desc = {};

	bind_group_layout_desc.entryCount = 2;
	bind_group_layout_desc.entries = bind_group_layout_entry;

	WGPUBindGroupEntry bind_group_entry[] = {
		// texure for fragment shader
		{.binding = 0, .textureView = texture_view},
		// sampler for fragment shader
		{.binding = 1, .sampler = sampler},
	};

	WGPUBindGroupLayout bind_group_layout = wgpuDeviceCreateBindGroupLayout(webgpu_ctx->m_Device, &bind_group_layout_desc);

	WGPUBindGroupDescriptor bind_group_desc = {};
	bind_group_desc.layout = bind_group_layout;
	bind_group_desc.entryCount = 2;
	bind_group_desc.entries = bind_group_entry;

	ret->m_Texture = wgpuDeviceCreateBindGroup(webgpu_ctx->m_Device, &bind_group_desc);
	wgpuBindGroupLayoutRelease(bind_group_layout);

	// release resources that are now owned by pipeline and will not be used in this code later
	wgpuSamplerRelease(sampler);
	wgpuTextureViewRelease(texture_view);

	// ret->m_Texture = (unsigned int)drgLibDrawLoadTexture((unsigned char*)ret->m_Dib->GetPixelPtr(0, 0), width, height);
	return true;
}

bool drgTexture::IsNativeData(drgTexture *tex, unsigned char *data)
{
	if (data == NULL)
	{
		return false;
	}
	if ((m_HardwareType == DRG_TEX_HARDWARE_DXT) || (m_HardwareType == DRG_TEX_HARDWARE_ATC) || (m_HardwareType == DRG_TEX_HARDWARE_TEX))
	{
		DRG_DDS_HEADER *pHeader = (DRG_DDS_HEADER *)data;
		bool isDDSMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
		if (!isDDSMagicValid)
		{
			return false;
		}
		return true;
	}
	return false;
}

bool drgTexture::LoadNativeData(drgTexture *tex, unsigned char *data)
{
	if (data == NULL)
	{
		return false;
	}
	if ((m_HardwareType == DRG_TEX_HARDWARE_DXT) || (m_HardwareType == DRG_TEX_HARDWARE_ATC) || (m_HardwareType == DRG_TEX_HARDWARE_TEX))
	{
		DRG_DDS_HEADER *pHeader = (DRG_DDS_HEADER *)data;
		bool isDDSMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
		if (!isDDSMagicValid)
		{
			return false;
		}
#if defined(_WINDOWS)
//		HGLRC oldRC = wglGetCurrentContext();
//		HDC oldDC = wglGetCurrentDC();
#endif
		drgEngine::NativeContextSetMain();
		// glGenTextures(1, &tex->m_Texture);	           GL_ERROR("Failed to generate texture!");
		// glBindTexture(GL_TEXTURE_2D, tex->m_Texture);	           GL_ERROR("Failed to bind texture!");

		unsigned int offset = 0;
		unsigned int width = pHeader->m_Width;
		unsigned int height = pHeader->m_Height;

		bool isMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
		bool isDXT1 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "DXT1", 4) == 0;
		bool isDXT5 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "DXT5", 4) == 0;
		bool isATC = memcmp(pHeader->m_FourCharIdOfPixelFormat, "ATC ", 4) == 0;
		bool isATCI = memcmp(pHeader->m_FourCharIdOfPixelFormat, "ATCI", 4) == 0;
		bool isTEX0 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "TEX0", 4) == 0;
		bool isTEX1 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "TEX1", 4) == 0;
		bool isTEX2 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "TEX2", 4) == 0;
		bool isValidDdsFile = isMagicValid && (isDXT1 || isDXT5 || isATC || isATCI || isTEX0 || isTEX1 || isTEX2);
		unsigned int format = 0;
		unsigned int blocksize = 16;
		unsigned int mip = 0;
		if (isTEX0 || isTEX1 || isTEX2)
		{
			unsigned int format2 = 0;
			if (isTEX0)
			{
				blocksize = 16;
				// format = GL_RGB5;
				// format2 = GL_UNSIGNED_SHORT_5_5_5_1_EXT;
			}
			else if (isTEX1)
			{
				blocksize = 16;
				// format = GL_RGB5_A1;
				// format2 = GL_UNSIGNED_SHORT_5_5_5_1_EXT;
			}
			else if (isTEX2)
			{
				blocksize = 16;
				// format = GL_RGBA4;
				// format2 = GL_UNSIGNED_SHORT_4_4_4_4_EXT;
			}
			do
			{
				unsigned int Size = width * height * 2;
				// glTexImage2D(GL_TEXTURE_2D, mip, format, width, height, 0, GL_RGBA, format2, (data + sizeof(DRG_DDS_HEADER)) + offset);	           GL_ERROR("Fail to set texure image!");
				offset += Size;
				if ((width <<= 1) == 0)
				{
					width = 1;
				}
				if ((height <<= 1) == 0)
				{
					height = 1;
				}
				mip++;
			} while (mip < pHeader->m_MipMapCount);
		}
		else
		{
			if (isDXT1)
			{
				blocksize = 8;
				// format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			}
			else if (isDXT5)
			{
				blocksize = 16;
				// format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			}
			else if (isATC)
			{
				blocksize = 8;
				// format = GL_ATC_RGB_AMD;
			}
			else if (isATCI)
			{
				blocksize = 16;
				// format = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
			}
			do
			{
				unsigned int Size = ((width + 3) >> 2) * ((height + 3) >> 2) * blocksize;
				// pglCompressedTexImage2D(GL_TEXTURE_2D, mip, format, width, height, 0, Size, (data + sizeof(DRG_DDS_HEADER)) + offset); 	           GL_ERROR("Fail to set compressed texture!");
				offset += Size;
				if ((width <<= 1) == 0)
				{
					width = 1;
				}
				if ((height <<= 1) == 0)
				{
					height = 1;
				}
				mip++;
			} while (mip < pHeader->m_MipMapCount);
		}

		// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	           GL_ERROR("Fail to set texture parameter!");
		// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	           GL_ERROR("Fail to set texture parameter!");
#if defined(_WINDOWS)
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pHeader->m_MipMapCount - 1);	           GL_ERROR("Fail to set texture parameter!");
		// wglMakeCurrent(oldDC, oldRC);
#endif
		tex->m_Width = pHeader->m_Width;
		tex->m_Height = pHeader->m_Height;
		tex->m_Format = DRG_FMT_UNKNOWN;
		return true;
	}
	return false;
}

#else
bool drgTexture::LoadDataFromDib(drgTexture *ret)
{
	return true;
}

bool drgTexture::IsNativeData(drgTexture *tex, unsigned char *data)
{
	return false;
}

bool drgTexture::LoadNativeData(drgTexture *tex, unsigned char *data)
{
	return true;
}

#endif
