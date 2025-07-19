
#ifndef _DRG_DIB_H_
#define _DRG_DIB_H_

#include "system/global.h"

#define DRG_DIB_FLAG_RGBA (1 << 0)
#define DRG_DIB_FLAG_UINT (1 << 1)
#define DRG_DIB_FLAG_FLOAT (1 << 2)
#define DRG_DIB_FLAG_NO_ALPHA (1 << 3)
#define DRG_DIB_FLAG_DEFAULT (DRG_DIB_FLAG_RGBA)

struct drgDibTexel
{
	unsigned char r, g, b, a;
};

struct drgDibColor
{
	union
	{
		float m_FColor;
		unsigned int m_UIColor;
		drgDibTexel m_Texel;
	};
};

class drgDibLayer
{
public:
	enum FILTER_TYPE
	{
		FILTER_MITCHELL,
		FILTER_LANCZOS
	};

	drgDibLayer(const char *desc);
	~drgDibLayer();

	void CleanUp();
	void Init(unsigned int width, unsigned int height, unsigned int flags);
	void Fill(void *data);
	void FillRectColor(drgDibColor color);
	bool HasAlpha();
	bool IsCutOut();

	void WriteToTga(const char *filename, bool bit_24 = false);
	void ReadFromTga(const char *filename);
	void ReadFromTga(void *data, int datalen);
	void WriteToBmp(const char *filename);
	void ReadFromBmp(const char *filename);
	void ReadFromBmp(void *data, int datalen);
	void WriteToPng(const char *filename);
	void ReadFromPng(const char *filename);
	void ReadFromPng(void *data, int datalen);
	void WriteToJpg(const char *filename);
	void ReadFromJpg(const char *filename);
	void ReadFromJpg(void *data, int datalen);
	void WriteToDxt(const char *filename);
	void ReadFromDxt(const char *filename);
	void ReadFromDxt(void *data, int datalen);
	void WriteToTex(const char *filename);

	void Scale(unsigned int width, unsigned int height, FILTER_TYPE filter);
	void GaussianScale(unsigned int width, unsigned int height, int filter);
	void Resize(unsigned int width, unsigned int height);
	void PadToPowerOf2();

	drgDibColor *m_pBits;
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_DeltaScan;
	unsigned int m_NumPixels;
	unsigned int m_Flags;
	float m_HalfWidth;
	float m_HalfHeight;
	const char *m_Description;
};

class drgDibInfo
{
public:
	drgDibInfo(const char *desc = NULL);
	~drgDibInfo();

	void Init(unsigned int layers, unsigned int width, unsigned int height, unsigned int flags = DRG_DIB_FLAG_DEFAULT);
	void CleanUp();
	void Fill(unsigned int layer, void *data);
	void FillRectRGB(unsigned int layer, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void FillRectUI(unsigned int layer, unsigned int val);
	void FillRectFloat(unsigned int layer, float val);
	void FillRectColor(unsigned int layer, drgDibColor color);

	void WriteToTga(unsigned int layer, char *filename);
	void ReadFromTga(const char *filename);
	void ReadFromTga(void *data, int datalen);
	void WriteToBmp(unsigned int layer, char *filename);
	void ReadFromBmp(const char *filename);
	void ReadFromBmp(void *data, int datalen);
	void WriteToPng(unsigned int layer, char *filename);
	void ReadFromPng(const char *filename);
	void ReadFromPng(void *data, int datalen);
	void WriteToJpg(unsigned int layer, char *filename);
	void ReadFromJpg(const char *filename);
	void ReadFromJpg(void *data, int datalen);
	void WriteToDxt(unsigned int layer, char *filename);
	void ReadFromDxt(const char *filename);
	void ReadFromDxt(void *data, int datalen);
	void WriteToTex(unsigned int layer, char *filename);
	void WriteToImageFile(unsigned int layer, char *filename);
	void ReadFromImageFile(const char *filename);
	void ReadFromImageInMemory(void *buf, int buflen);

	void BuildLayers();
	void FlagsSet(unsigned int flag);
	void FlagsAdd(unsigned int flag);
	void FlagsRemove(unsigned int flag);
	bool HasFlag(unsigned int layer, unsigned int flag);

	inline const char *GetDescription()
	{
		return m_Description;
	}

	inline int Layers() { return m_Layers; };
	inline drgDibLayer *GetLayer(unsigned int layer)
	{
		assert(layer < m_Layers);
		return m_LayerInfo[layer];
	};

	inline int Height(unsigned int layer)
	{
		assert(layer < m_Layers);
		return m_LayerInfo[layer]->m_Height;
	};
	inline int Width(unsigned int layer)
	{
		if (layer >= m_Layers)
			return -1; /*assert(layer<m_Layers);*/
		return m_LayerInfo[layer]->m_Width;
	};
	inline float HalfHeight(unsigned int layer)
	{
		assert(layer < m_Layers);
		return m_LayerInfo[layer]->m_HalfHeight;
	};
	inline float HalfWidth(unsigned int layer)
	{
		assert(layer < m_Layers);
		return m_LayerInfo[layer]->m_HalfWidth;
	};
	inline int DeltaScan(unsigned int layer)
	{
		assert(layer < m_Layers);
		return m_LayerInfo[layer]->m_DeltaScan;
	};
	inline int NumPixels(unsigned int layer)
	{
		assert(layer < m_Layers);
		return m_LayerInfo[layer]->m_NumPixels;
	};
	inline drgDibColor *GetPixelPtr(unsigned int layer, unsigned int count)
	{
		assert(layer < m_Layers);
		return &m_LayerInfo[layer]->m_pBits[count];
	};
	inline void SetPixel(unsigned int layer, unsigned int count, unsigned int uicolor)
	{
		assert(layer < m_Layers);
		m_LayerInfo[layer]->m_pBits[count].m_UIColor = uicolor;
	};
	inline void SetPixel(unsigned int layer, unsigned int count, float fcolor)
	{
		assert(layer < m_Layers);
		m_LayerInfo[layer]->m_pBits[count].m_FColor = fcolor;
	};
	void Resize(int width_new, int height_new);
	void Scale(int width_new, int height_new, drgDibLayer::FILTER_TYPE filter);
	void GaussianScale(int width_new, int height_new, int filter);
	void PadToPowerOf2(unsigned int layer);
	bool HasAlpha();
	bool IsCutOut();

protected:
	drgDibLayer **m_LayerInfo;
	unsigned int m_Layers;
	char m_Description[DRG_MAX_PATH];
};

typedef struct
{
	char identsize;		// size of ID field that follows 18 byte header (0 usually)
	char colourmaptype; // type of colour map 0=none, 1=has palette
	char imagetype;		// type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

	short colourmapstart; // first colour map entry in palette
	// short colourmaplength;    // number of colours in palette
	char colourmapbits; // number of bits per palette entry 15,16,24,32

	short xstart;	 // image x origin
	short ystart;	 // image y origin
	short width;	 // image width in pixels
	short height;	 // image height in pixels
	char bits;		 // image bits per pixel 8,16,24,32
	char descriptor; // image descriptor bits (vh flip bits)
					 // pixel data follows header
} DRG_TGA_HEADER;

typedef struct
{
	char m_Magic[4];
	unsigned int m_Size;
	unsigned int m_Flags;
	unsigned int m_Height;
	unsigned int m_Width;
	unsigned int m_PitchOrLinearSize;
	unsigned int m_Depth;
	unsigned int m_MipMapCount;
	unsigned int m_Reserved1[11];
	unsigned int m_SizeOfPixelFormat;
	unsigned int m_FlagsOfPixelFormat;
	char m_FourCharIdOfPixelFormat[4];
	char m_Data[40];
} DRG_DDS_HEADER;

#endif // _DRG_DIB_H_
