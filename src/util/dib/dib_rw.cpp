
#include "util/dib/dib.h"
#include "system/file.h"
#include "core/file/asset_read.h"
#include "util/math_def.h"
#include "util/stringn.h"
#include "util/dib/png/lodepng.h"
#include "util/dib/jpg/jpegdecoder.h"
#include "util/dib/dxt/dxt.h"


void drgDibLayer::WriteToTga(const char *filename, bool bit_24)
{
	drgFile ofile;
	if(!ofile.Open(filename,drgFile::DRG_MODE_WRITE_BIN))
		return;

	int width = (int)m_Width;
	int height = (int)m_Height;

	DRG_TGA_HEADER tgahead;
	memset(&tgahead,0,sizeof(DRG_TGA_HEADER));
	tgahead.imagetype=2;
	if(bit_24)
		tgahead.bits=24;
	else
		tgahead.bits=32;
	tgahead.descriptor=8;
	tgahead.width=width;
	tgahead.height=height;

	ofile.Write(&tgahead,sizeof(DRG_TGA_HEADER));

	int x,y;
	unsigned int curpixel;
	if(m_Flags & DRG_DIB_FLAG_RGBA)
	{
		for (y=height-1;y>=0;y--)
		{
			for (x=0;x<width;x++)
			{
				curpixel = (y*width)+x;
				ofile.WriteChar(m_pBits[curpixel].m_Texel.b);
				ofile.WriteChar(m_pBits[curpixel].m_Texel.g);
				ofile.WriteChar(m_pBits[curpixel].m_Texel.r);
				if(!bit_24)
					ofile.WriteChar(m_pBits[curpixel].m_Texel.a);
			}
		}
	}
	if(m_Flags & DRG_DIB_FLAG_FLOAT)
	{
		unsigned char outcolor = 128;
		float multval = 1.0f;
		float minval = DRG_BIGFLOAT;
		float maxval = 0.0f;
		for (curpixel=0;curpixel<m_NumPixels;curpixel++)
		{
			if((m_pBits[curpixel].m_FColor>0.0f) && (m_pBits[curpixel].m_FColor<DRG_BIGFLOAT))
			{
				minval = DRG_MIN( minval, m_pBits[curpixel].m_FColor );
				maxval = DRG_MAX( maxval, m_pBits[curpixel].m_FColor );
			}
		}
		if((maxval-minval)==0.0f)
			multval = 0.0f;
		else
			multval = 255.0f/(maxval-minval);
		for (y=height-1;y>=0;y--)
		{
			for (x=0;x<width;x++)
			{
				curpixel = (y*width)+x;
				outcolor = (unsigned char)((m_pBits[curpixel].m_FColor-minval)*multval);
				ofile.WriteChar(outcolor);
				ofile.WriteChar(outcolor);
				ofile.WriteChar(outcolor);
				ofile.WriteChar(255);
			}
		}
	}
	ofile.Close();
}

void drgDibLayer::ReadFromTga(const char *filename)
{
	unsigned int datalen = 0;
	void* filedata = drgFile::MallocFile(filename, &datalen);
	if(datalen>0)
		ReadFromTga(filedata, datalen);
	drgMemFree(filedata);
}

void drgDibLayer::ReadFromTga(void* data, int datalen)
{
	char padd;
	DRG_TGA_HEADER tgahead;
	unsigned int buff_pos = 0;
	char* buffer = (char*)data;
	unsigned char* pixels = (unsigned char*)data;

	tgahead.identsize = buffer[buff_pos++];
	tgahead.colourmaptype = buffer[buff_pos++];
	tgahead.imagetype = buffer[buff_pos++];
	padd = buffer[buff_pos++];  // padding

	tgahead.colourmapstart = *((short*)&buffer[buff_pos]); buff_pos += 2;
	tgahead.colourmapbits = buffer[buff_pos++];
	padd = buffer[buff_pos++];  // padding

	tgahead.xstart = *((short*)&buffer[buff_pos]); buff_pos += 2;
	tgahead.ystart = *((short*)&buffer[buff_pos]); buff_pos += 2;
	tgahead.width = *((short*)&buffer[buff_pos]); buff_pos += 2;
	tgahead.height = *((short*)&buffer[buff_pos]); buff_pos += 2;
	tgahead.bits = buffer[buff_pos++];
	tgahead.descriptor = buffer[buff_pos++];

	char bitDepth = tgahead.bits;
	if (!((bitDepth == 8) || (bitDepth == 16) || (bitDepth == 24) || (bitDepth == 32)))
		return;

	if (tgahead.identsize>0)
		buff_pos += tgahead.identsize;

	Init(tgahead.width, tgahead.height, DRG_DIB_FLAG_RGBA);
	//return;

	int starty = m_Height - 1;
	int endy = -1;
	int incy = -1;
	int curpixel;
	unsigned int x, y;

	if (tgahead.descriptor == 32)
	{
		starty = 0;
		endy = m_Height;
		incy = 1;
	}

	if (tgahead.imagetype == 2)
	{
		if (bitDepth == 24)
			m_Flags |= DRG_DIB_FLAG_NO_ALPHA;
		for (y = starty; y != endy; y += incy)
		{
			for (x = 0; x<m_Width; x++)
			{
				curpixel = (y*m_Width) + x;
				if (bitDepth == 32)
				{
					m_pBits[curpixel].m_Texel.b = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.g = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.r = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.a = pixels[buff_pos++];
				}
				else if (bitDepth == 24)
				{
					m_pBits[curpixel].m_Texel.b = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.g = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.r = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.a = 255;
				}
				else if (bitDepth == 16)
				{
					m_pBits[curpixel].m_Texel.b = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.g = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.r = 255;
					m_pBits[curpixel].m_Texel.a = 255;
				}
				else if (bitDepth == 8)
				{
					m_pBits[curpixel].m_Texel.b = pixels[buff_pos++];
					m_pBits[curpixel].m_Texel.g = 255;
					m_pBits[curpixel].m_Texel.r = 255;
					m_pBits[curpixel].m_Texel.a = 255;
				}
			}
		}
	}
	else if (tgahead.imagetype == 10)
	{
		unsigned char colorbuffer[8];
		unsigned char chunkheader = 0;
		unsigned int bytesPerPixel = bitDepth / 8;
		unsigned int pixelcount = tgahead.height * tgahead.width;
		for (y = starty; y != endy; y += incy)
		{
			for (x = 0; x<m_Width;)
			{
				chunkheader = pixels[buff_pos++];
				if (chunkheader < 128)
				{
					chunkheader++;
					for (short counter = 0; counter < chunkheader; counter++)
					{
						memcpy(colorbuffer, &pixels[buff_pos], bytesPerPixel);
						buff_pos += bytesPerPixel;
						curpixel = (y*m_Width) + x;
						m_pBits[curpixel].m_Texel.r = colorbuffer[2];
						m_pBits[curpixel].m_Texel.g = colorbuffer[1];
						m_pBits[curpixel].m_Texel.b = colorbuffer[0];
						if (bytesPerPixel == 4)
							m_pBits[curpixel].m_Texel.a = colorbuffer[3];
						else
							m_pBits[curpixel].m_Texel.a = 255;
						x++;
					}
				}
				else
				{
					chunkheader -= 127;
					memcpy(colorbuffer, &pixels[buff_pos], bytesPerPixel);
					buff_pos += bytesPerPixel;
					for (short counter = 0; counter < chunkheader; counter++)
					{
						curpixel = (y*m_Width) + x;
						m_pBits[curpixel].m_Texel.r = colorbuffer[2];
						m_pBits[curpixel].m_Texel.g = colorbuffer[1];
						m_pBits[curpixel].m_Texel.b = colorbuffer[0];
						if (bytesPerPixel == 4)
							m_pBits[curpixel].m_Texel.a = colorbuffer[3];
						else
							m_pBits[curpixel].m_Texel.a = 255;
						x++;
					}
				}
			}
		}
	}
}

void drgDibLayer::WriteToBmp(const char *filename)
{
	drgFile ofile;
	if(!ofile.Open(filename,drgFile::DRG_MODE_WRITE_BIN))
		return;

	int i;
	short s;
	int width = (int)m_Width;
	int height = (int)m_Height;
	int lineSize=(width*3+3)&~3;
	s=0x4d42; ofile.Write(&s,sizeof(s));
	i=lineSize*height+14+10*4; ofile.Write(&i,sizeof(i));
	s=0;ofile.Write(&s,sizeof(s));
	s=0;ofile.Write(&s,sizeof(s));//54
	i=0x36; ofile.Write(&i,sizeof(i));//10*4
	
	i=10*4; ofile.Write(&i,sizeof(i));
	i=width; ofile.Write(&i,sizeof(i));
	i=height; ofile.Write(&i,sizeof(i));
	s=1; ofile.Write(&s,sizeof(s));
	s=24; ofile.Write(&s,sizeof(s));
	i=0; ofile.Write(&i,sizeof(i));// compression type 

	i=0; ofile.Write(&i,sizeof(i));
	i=0; ofile.Write(&i,sizeof(i));
	i=0; ofile.Write(&i,sizeof(i));
	i=0; ofile.Write(&i,sizeof(i));
	i=0; ofile.Write(&i,sizeof(i));

	int x,y;
	unsigned int curpixel;
	if(m_Flags & DRG_DIB_FLAG_RGBA)
	{
		for (y=height-1;y>=0;y--)
		{
			for (x=0;x<width;x++)
			{
				curpixel = (y*width)+x;
				ofile.WriteChar(m_pBits[curpixel].m_Texel.b);
				ofile.WriteChar(m_pBits[curpixel].m_Texel.g);
				ofile.WriteChar(m_pBits[curpixel].m_Texel.r);
			}
			x*=3;
			for (;x&3;x++)
				ofile.WriteChar(0);
		}
	}
	if(m_Flags & DRG_DIB_FLAG_FLOAT)
	{
		unsigned char outcolor = 128;
		float multval = 1.0f;
		float minval = DRG_BIGFLOAT;
		float maxval = 0.0f;
		for (curpixel=0;curpixel<m_NumPixels;curpixel++)
		{
			if((m_pBits[curpixel].m_FColor>0.0f) && (m_pBits[curpixel].m_FColor<DRG_BIGFLOAT))
			{
				minval = DRG_MIN( minval, m_pBits[curpixel].m_FColor );
				maxval = DRG_MAX( maxval, m_pBits[curpixel].m_FColor );
			}
		}
		if((maxval-minval)==0.0f)
			multval = 0.0f;
		else
			multval = 255.0f/(maxval-minval);
		for (y=height-1;y>=0;y--)
		{
			for (x=0;x<width;x++)
			{
				curpixel = (y*width)+x;
				outcolor = (unsigned char)((m_pBits[curpixel].m_FColor-minval)*multval);
				ofile.WriteChar(outcolor);
				ofile.WriteChar(outcolor);
				ofile.WriteChar(outcolor);
			}
			x*=3;
			for (;x&3;x++)
				ofile.WriteChar(0);
		}
	}
	ofile.Close();
}

void drgDibLayer::ReadFromBmp(const char *filename)
{
	unsigned int datalen = 0;
	void* filedata = drgFile::MallocFile(filename, &datalen);
	if(datalen>0)
		ReadFromBmp(filedata, datalen);
	drgMemFree(filedata);
}

void drgDibLayer::ReadFromBmp(void* data, int datalen)
{
	char buff[160];
	int x,y;
	int i;
	int curpixel;
	int width,height;
	int offbits,ihSize,clrUsed;
	short bitspp;
	unsigned char pallet[256][3];
	char padd;

	m_Flags |= DRG_DIB_FLAG_NO_ALPHA;
	
	drgAssetReader ifile;
	ifile.OpenBuff(data, datalen);
	ifile.Read(buff, 10);
	ifile.ReadEndian32(&offbits);
	ifile.ReadEndian32(&ihSize);
	ifile.ReadEndian32(&width);
	ifile.ReadEndian32(&height);
	ifile.ReadEndian16(&bitspp);
	ifile.ReadEndian16(&bitspp);

	if (bitspp==8)
	{
		ifile.Seek(20,drgFile::DRG_SEEK_CUR);
		ifile.ReadEndian32(&clrUsed);

		if (clrUsed==0)
			clrUsed=256;
		ifile.Seek(14+ihSize,drgFile::DRG_SEEK_BEGIN);

		// read palette
		memset(pallet,0,sizeof(pallet));
		for (i=0;i<clrUsed;i++)
		{
			ifile.ReadEndian8(&pallet[i][2]);
			ifile.ReadEndian8(&pallet[i][1]);
			ifile.ReadEndian8(&pallet[i][0]);
			ifile.ReadEndian8(&padd);  // padding
		}
	}

	ifile.Seek(offbits,drgFile::DRG_SEEK_BEGIN);

	Init(width, height, DRG_DIB_FLAG_RGBA);


	if (bitspp==8)
	{
		for (y=m_Height-1;y>=0;y--)
		{
			for (x=0;x<(int)m_Width;x++)
			{
				curpixel = (y*m_Width)+x;
				unsigned char c;
				ifile.ReadEndian8(&c);
				m_pBits[curpixel].m_Texel.r=pallet[c][0];
				m_pBits[curpixel].m_Texel.g=pallet[c][1];
				m_pBits[curpixel].m_Texel.b=pallet[c][2];
				m_pBits[curpixel].m_Texel.a=255;
			}
			for (;x&3;x++)
				ifile.ReadEndian8(&padd);  // padding
		}
	}
	if (bitspp==24)
	{
		for (y=m_Height-1;y>=0;y--)
		{
			for (x=0;x<(int)m_Width;x++)
			{
				curpixel = (y*m_Width)+x;
				ifile.ReadEndian8(&m_pBits[curpixel].m_Texel.b);
				ifile.ReadEndian8(&m_pBits[curpixel].m_Texel.g);
				ifile.ReadEndian8(&m_pBits[curpixel].m_Texel.r);
				m_pBits[curpixel].m_Texel.a=255;
			}
			x*=3;
			for (;x&3;x++)
				ifile.ReadEndian8(&padd);  // padding
		}
	} 
}

void drgDibLayer::WriteToPng(const char *filename)
{
	unsigned char* buffer = (unsigned char*)drgMemAlloc(m_Width*m_Height*sizeof(unsigned int));
	unsigned int i,x,y;
	int curpixel;
	for(i=0,y=0;y<m_Height;++y)
	{
		for(x=0;x<m_Width;++x)
		{
			curpixel = (y*m_Width)+x;
			buffer[i++]=m_pBits[curpixel].m_Texel.r;
			buffer[i++]=m_pBits[curpixel].m_Texel.g;
			buffer[i++]=m_pBits[curpixel].m_Texel.b;
			buffer[i++]=m_pBits[curpixel].m_Texel.a;
		}
	}

	unsigned char* png;
	size_t pngsize;
	unsigned int error;
	LodePNGState state;

	lodepng_state_init(&state);
	state.info_raw.colortype = LCT_RGBA;
	state.info_raw.bitdepth = 8;
	state.info_png.color.colortype = LCT_RGBA;
	state.info_png.color.bitdepth = 8;
	lodepng_encode(&png, &pngsize, buffer, m_Width, m_Height, &state);
	error = state.error;
	lodepng_state_cleanup(&state);

	if (!error)
	{
		drgFile ofile;
		if (ofile.Open(filename, drgFile::DRG_MODE_WRITE_BIN))
		{
			ofile.Write((char*)png, pngsize);
			ofile.Close();
		}
	}

	if (error)
		printf("error %u: %s\n", error, lodepng_error_text(error));

	free(png);
	drgMemFree(buffer);
}

void drgDibLayer::ReadFromPng(const char *filename)
{
	unsigned int datalen = 0;
	void* filedata = drgFile::MallocFile(filename, &datalen);
	if(datalen>0)
		ReadFromPng(filedata, datalen);
	drgMemFree(filedata);
}

void drgDibLayer::ReadFromPng(void* data, int datalen)
{
	unsigned char* buffer = NULL;
	unsigned int width, height;
	unsigned int error;

	error = lodepng_decode32(&buffer, &width, &height, (const unsigned char*)data, datalen); //load the image file with given filename

	if(error)
	{
		drgPrintWarning("error: %d\n", error);
	}
	else
	{
		unsigned int i,x,y;
		int curpixel;

		Init(width, height, DRG_DIB_FLAG_RGBA);
		for(i=0,y=0;y<height;++y)
		{
			curpixel = (y*width);
			for(x=0;x<width;++x)
			{
				m_pBits[curpixel].m_Texel.r=buffer[i++];
				m_pBits[curpixel].m_Texel.g=buffer[i++];
				m_pBits[curpixel].m_Texel.b=buffer[i++];
				m_pBits[curpixel].m_Texel.a=buffer[i++];
				curpixel++;
			}
		}
		
		// cleanup decoder
		free(buffer);
	}
}

void drgDibLayer::WriteToJpg(const char *filename)
{
}

void drgDibLayer::ReadFromJpg(const char *filename)
{
	unsigned int datalen = 0;
	void* filedata = drgFile::MallocFile(filename, &datalen);
	if(datalen>0)
		ReadFromJpg(filedata, datalen);
	drgMemFree(filedata);
}

void drgDibLayer::ReadFromJpg(void* data, int datalen)
{
	jpeg_decoder_mem_stream	m_pjpeg_decoder_stream;
	jpeg_decoder				m_pjpeg_decoder;

	if (!m_pjpeg_decoder_stream.open(data, datalen))
	{
		drgPrintError("Unable to create buffer!");
		return;
	}

	m_pjpeg_decoder.start(&m_pjpeg_decoder_stream);
	if (m_pjpeg_decoder.get_error_code() != 0)
	{
		drgPrintError("JPG Decoder failed! Error status: %i", m_pjpeg_decoder.get_error_code());

		// Always be sure to delete the input stream object _after_
		// the decoder is deleted. Reason: the decoder object calls the input
		// stream's detach() method.
		return;
	}

	int curpixel;
	unsigned int width, height;
	width=m_pjpeg_decoder.get_width();
	height=m_pjpeg_decoder.get_height();

	Init(width, height, DRG_DIB_FLAG_RGBA);

	bool isGey=((m_pjpeg_decoder.get_num_components() == 1) ? true : false);
	
	if (m_pjpeg_decoder.begin())
	{
		drgPrintError("JPG Decoder failed! Error status: %i", m_pjpeg_decoder.get_error_code());
		return;
	}

	int lines_decoded = 0;
	for ( ; ; )
	{
		void *Pscan_line_ofs;
		unsigned int scan_line_len;
		if (m_pjpeg_decoder.decode(&Pscan_line_ofs, &scan_line_len))
			break;

		if (m_pjpeg_decoder.get_num_components() == 3)
		{
			unsigned char *Psb = (unsigned char *)Pscan_line_ofs;
			int src_bpp = m_pjpeg_decoder.get_bytes_per_pixel();

			curpixel = (lines_decoded*width);
			for (unsigned int x = 0; x < width; x++, Psb += src_bpp)
			{
				m_pBits[curpixel].m_Texel.r=Psb[0];
				m_pBits[curpixel].m_Texel.b=Psb[2];
				m_pBits[curpixel].m_Texel.g=Psb[1];
				m_pBits[curpixel].m_Texel.a=255;
				curpixel++;
			}
		}
		else
		{
			unsigned char *Psb = (unsigned char *)Pscan_line_ofs;
			int src_bpp = m_pjpeg_decoder.get_bytes_per_pixel();

			curpixel = (lines_decoded*width);
			for (unsigned int x = 0; x < width; x++, Psb += src_bpp)
			{
				m_pBits[curpixel].m_Texel.r=Psb[0];
				m_pBits[curpixel].m_Texel.b=Psb[0];
				m_pBits[curpixel].m_Texel.g=Psb[0];
				m_pBits[curpixel].m_Texel.a=255;
				curpixel++;
			}
		}
		lines_decoded++;
	}
	m_pjpeg_decoder.cleanup();


	if (m_pjpeg_decoder.get_error_code())
	{
		drgPrintError("JPG Decoder failed! Error status: %i", m_pjpeg_decoder.get_error_code());
	}
}

unsigned int IMAGE_MAKEFOURCC(char c1, char c2, char c3, char c4 )
{
	unsigned int retval;
	char vals[4];
	vals[0]=c1;
	vals[1]=c2;
	vals[2]=c3;
	vals[3]=c4;

	retval = *((unsigned int*)vals);
	return retval;
}

inline static int Unpack565(unsigned char const* packed, drgDibColor* colour)
{
	int value = (int)packed[0] | ((int)packed[1] << 8);

	unsigned char red = (unsigned char)((value >> 11) & 0x1f);
	unsigned char green = (unsigned char)((value >> 5) & 0x3f);
	unsigned char blue = (unsigned char)(value & 0x1f);

	colour->m_Texel.r = (red << 3) | (red >> 2);
	colour->m_Texel.g = (green << 2) | (green >> 4);
	colour->m_Texel.b = (blue << 3) | (blue >> 2);
	colour->m_Texel.a = 255;
	return value;
}

bool WriteDXTHeader(drgFile* ofile, drgDibLayer* image, unsigned int dxtnum, unsigned int mipmap)
{
	unsigned int tempwriteval;
	unsigned int i, FourCC, Flags1 = 0, Flags2 = 0, ddsCaps1 = 0, LinearSize, BlockSize, ddsCaps2 = 0;

	Flags1 |= DRG_DDSD_LINEARSIZE | DRG_DDSD_MIPMAPCOUNT | DRG_DDSD_WIDTH | DRG_DDSD_HEIGHT | DRG_DDSD_CAPS | DRG_DDSD_PIXELFORMAT;
	Flags2 |= DRG_DDPF_FOURCC;

	// @TODO:  Fix the pre-multiplied alpha problem.
	if (dxtnum == 2)
		dxtnum = 3;
	else if (dxtnum == 4)
		dxtnum = 5;

	switch (dxtnum)
	{
		case 1:
			FourCC = IMAGE_MAKEFOURCC('D','X','T','1');
			break;
		case 2:
			FourCC = IMAGE_MAKEFOURCC('D','X','T','2');
			break;
		case 3:
			FourCC = IMAGE_MAKEFOURCC('D','X','T','3');
			break;
		case 4:
			FourCC = IMAGE_MAKEFOURCC('D','X','T','4');
			break;
		case 5:
			FourCC = IMAGE_MAKEFOURCC('D','X','T','5');
			break;
		default:
			FourCC = 0;
	}

	ofile->WriteEndian32((void*)"DDS ");
	tempwriteval=124;
	ofile->WriteEndian32(&tempwriteval);	// Size1
	ofile->WriteEndian32(&Flags1);			// Flags1
	ofile->WriteEndian32(&image->m_Height);
	ofile->WriteEndian32(&image->m_Width);

	if (dxtnum == 1)
		BlockSize = 8;
	else
		BlockSize = 16;

	LinearSize = (((image->m_Width + 3)/4) * ((image->m_Height + 3)/4)) * BlockSize;

	ofile->WriteEndian32(&LinearSize);		// LinearSize (TODO: change this when uncompressed formats are supported)
	tempwriteval=0;
	ofile->WriteEndian32(&tempwriteval);	// Depth
	tempwriteval=mipmap;
	ofile->WriteEndian32(&tempwriteval);	// MipMapCount
	tempwriteval=0;
	ofile->WriteEndian32(&tempwriteval);	// AlphaBitDepth

	for (i = 0; i < 10; i++)
		ofile->WriteEndian32(&tempwriteval); // Not used

	unsigned int Size2=32;
	unsigned int RGBBitCount, RBitMask, GBitMask, BBitMask, RGBAlphaBitMask;

	RGBBitCount=0;
	RBitMask=0;
	GBitMask=0;
	BBitMask=0;
	RGBAlphaBitMask=0;
	ddsCaps1 |= DRG_DDSCAPS_TEXTURE | DRG_DDSCAPS_MIPMAP;

	if(dxtnum==11) // Alpha Luminence
	{
		Flags2 = DRG_DDPF_ALPHAPIXELS | DRG_DDPF_LUMINANCE;
		FourCC=0;
		RGBBitCount=8;
		RBitMask=0x0F;
		GBitMask=0;
		BBitMask=0;
		RGBAlphaBitMask=0xF0;
		ddsCaps1 |= DRG_DDSCAPS_LUMINANCE;
	}

	tempwriteval=0;
	ofile->WriteEndian32(&Size2);			// Size2
	ofile->WriteEndian32(&Flags2);			// Flags2
	ofile->WriteEndian32(&FourCC);			// FourCC
	ofile->WriteEndian32(&RGBBitCount);		// RGBBitCount
	ofile->WriteEndian32(&RBitMask);		// RBitMask
	ofile->WriteEndian32(&GBitMask);		// GBitMask
	ofile->WriteEndian32(&BBitMask);		// BBitMask
	ofile->WriteEndian32(&RGBAlphaBitMask);	// RGBAlphaBitMask
	ofile->WriteEndian32(&ddsCaps1);		// ddsCaps1
	ofile->WriteEndian32(&ddsCaps2);		// ddsCaps2
	ofile->WriteEndian32(&tempwriteval);	// ddsCaps3
	ofile->WriteEndian32(&tempwriteval);	// ddsCaps4
	ofile->WriteEndian32(&tempwriteval);	// TextureStage

	return true;
}

void OutputDXTData(drgFile* ofile, drgDibLayer* image, unsigned int dxtnum, unsigned int mipmap)
{
	int x,y,i,acount;
	drgDibTexel ex0, ex1, temp;
	bool HasAlpha=false;
	unsigned int Count = 0;
	drgDibTexel Block[16];
	unsigned char AlphaBlock[16];
	unsigned char AlphaBitMask[6];
	unsigned int BitMask;
	unsigned short* BitMaskAdr=(unsigned short*)(&BitMask);
	unsigned char aval[2];

	unsigned char fillr=255;
	unsigned char fillg=0;
	unsigned char fillb=0;

	for (unsigned int z=0; z<mipmap; z++)
	{
		int flags=kDxt5;
		int sizeout=image->m_Width*image->m_Height;
		if(dxtnum==1)
		{
			flags=kDxt1;
			sizeout/=2;
		}

		flags|=kColourIterativeClusterFit;
		//flags|=kColourClusterFit;
		//flags|=kColourRangeFit;

		unsigned char* rgba=(unsigned char*)drgMemAlloc(image->m_Width*image->m_Height*4);
		void* blocks=drgMemAlloc(sizeout);
		int count=0;
		int curpixel = 0;
		for (int y=0;y<image->m_Height;y++)
		{
			for (int x=0;x<image->m_Width;x++)
			{
				curpixel = (y*image->m_Width)+x;
				rgba[count] = image->m_pBits[curpixel].m_Texel.r; ++count;
				rgba[count] = image->m_pBits[curpixel].m_Texel.g; ++count;
				rgba[count] = image->m_pBits[curpixel].m_Texel.b; ++count;
				rgba[count] = image->m_pBits[curpixel].m_Texel.a; ++count;
			}
		}

		CompressImage( rgba, image->m_Width, image->m_Height, blocks, flags );
//		ofile->Write(blocks, sizeout);
		for(int shortcount=0; shortcount<(sizeout/2); ++shortcount)
			ofile->WriteEndian16(&(((unsigned short*)blocks)[shortcount]));
		drgMemFree(blocks);
		drgMemFree(rgba);

		if (z<mipmap-1)
		{
			int newWidth;
			int newHeight;
			if (image->m_Width>1)
				newWidth=image->m_Width/2;
			if (image->m_Height>1)
				newHeight=image->m_Height/2;
			if (!IsPowerOfTwo(newWidth) || !IsPowerOfTwo(newHeight))
				assert(0);
			image->GaussianScale(newWidth, newHeight, 0);
			//if(!((image->flags&TEXFLAGS_NORMALMAP) || (image->flags&TEXFLAGS_DISPLACEMAP)))
			//	image.Sharpen(1.7);
		}
	}
	//unsigned int 
}

void drgDibLayer::WriteToDxt(const char *filename)
{
	drgFile ofile;
	if(!ofile.Open(filename,drgFile::DRG_MODE_WRITE_BIN))
		return;

	unsigned int dxtnum = 5;
	if(IsCutOut() || !HasAlpha())
		dxtnum = 1;

	if(!WriteDXTHeader(&ofile, this, dxtnum, 1))
	{
		ofile.Close();
		return;
	}
	OutputDXTData(&ofile, this, dxtnum, 1);

	ofile.Close();
}

void drgDibLayer::ReadFromDxt(const char *filename)
{
	unsigned int datalen = 0;
	void* filedata = drgFile::MallocFile(filename, &datalen);
	if (datalen>0)
		ReadFromDxt(filedata, datalen);
	drgMemFree(filedata);
}



void drgDibLayer::ReadFromDxt(void* data, int datalen)
{
	DRG_DDS_HEADER* pHeader = (DRG_DDS_HEADER*)data;
	bool isMagicValid = (memcmp(pHeader->m_Magic, "DDS ", 4) == 0);
	bool isDXT1 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "DXT1", 4) == 0;
	bool isDXT5 = memcmp(pHeader->m_FourCharIdOfPixelFormat, "DXT5", 4) == 0;
	if (!(isMagicValid && (isDXT1 || isDXT5)))
		return;

	Init(pHeader->m_Width, pHeader->m_Height, DRG_DIB_FLAG_RGBA);

	int sy;
	int c, d;
	unsigned char packed;
	unsigned char* block;
	drgDibColor codes[4];
	unsigned short color0, color1;
	int width = pHeader->m_Width;
	int height = pHeader->m_Height;
	drgDibColor* out_rgb = m_pBits;
	unsigned char* sourceBlock = (((unsigned char*)data) + sizeof(DRG_DDS_HEADER));

	if (isDXT1)
	{
		for (int y = 0; y < height; y += 4)
		{
			for (int x = 0; x < width; x += 4)
			{
				color0 = Unpack565(sourceBlock, codes);
				color1 = Unpack565(sourceBlock + 2, codes + 1);
				block = sourceBlock + 4;

				// generate the midpoints
				if (color0 <= color1)
				{
					c = codes[0].m_Texel.r;
					d = codes[1].m_Texel.r;
					codes[2].m_Texel.r = (unsigned char)((c + d) / 2);
					c = codes[0].m_Texel.g;
					d = codes[1].m_Texel.g;
					codes[2].m_Texel.g = (unsigned char)((c + d) / 2);
					c = codes[0].m_Texel.b;
					d = codes[1].m_Texel.b;
					codes[2].m_Texel.b = (unsigned char)((c + d) / 2);
					codes[2].m_Texel.a = 255;
					codes[3].m_Texel.r = 0;
					codes[3].m_Texel.g = 0;
					codes[3].m_Texel.b = 0;
					codes[3].m_Texel.a = 0;
				}
				else
				{
					c = codes[0].m_Texel.r;
					d = codes[1].m_Texel.r;
					codes[2].m_Texel.r = (unsigned char)((2 * c + d) / 3);
					codes[3].m_Texel.r = (unsigned char)((c + 2 * d) / 3);
					c = codes[0].m_Texel.g;
					d = codes[1].m_Texel.g;
					codes[2].m_Texel.g = (unsigned char)((2 * c + d) / 3);
					codes[3].m_Texel.g = (unsigned char)((c + 2 * d) / 3);
					c = codes[0].m_Texel.b;
					d = codes[1].m_Texel.b;
					codes[2].m_Texel.b = (unsigned char)((2 * c + d) / 3);
					codes[3].m_Texel.b = (unsigned char)((c + 2 * d) / 3);
					codes[2].m_Texel.a = 255;
					codes[3].m_Texel.a = 255;
				}

				for (int py = 0; py < 4; ++py)
				{
					packed = block[py];
					sy = y + py;
					if ((x+3) < width && sy < height)
					{
						out_rgb[(width*sy + x)].m_UIColor = codes[(packed & 0x3)].m_UIColor;
						out_rgb[(width*sy + (x + 1))].m_UIColor = codes[((packed >> 2) & 0x3)].m_UIColor;
						out_rgb[(width*sy + (x + 2))].m_UIColor = codes[((packed >> 4) & 0x3)].m_UIColor;
						out_rgb[(width*sy + (x + 3))].m_UIColor = codes[((packed >> 6) & 0x3)].m_UIColor;
					}
				}
				sourceBlock += 8;
			}
		}
		//m_Flags |= DRG_DIB_FLAG_NO_ALPHA;
	}
	else if (isDXT5)
	{
		int idx1;
		int value;
		unsigned char a_indices[16];
		unsigned char a_codes[8];
		for (int y = 0; y < height; y += 4)
		{
			for (int x = 0; x < width; x += 4)
			{
				// decode alpha
				int alpha0 = sourceBlock[0];
				int alpha1 = sourceBlock[1];
				unsigned char const* src = sourceBlock + 2;

				a_codes[0] = (unsigned char)alpha0;
				a_codes[1] = (unsigned char)alpha1;
				if (alpha0 <= alpha1)
				{
					// use 5-alpha codebook
					a_codes[2] = (unsigned char)(((4)*alpha0 + alpha1) / 5);
					a_codes[3] = (unsigned char)(((3)*alpha0 + 2*alpha1) / 5);
					a_codes[4] = (unsigned char)(((2)*alpha0 + 3*alpha1) / 5);
					a_codes[5] = (unsigned char)(((1)*alpha0 + 4*alpha1) / 5);
					a_codes[6] = 0;
					a_codes[7] = 255;
				}
				else
				{
					// use 7-alpha codebook
					a_codes[2] = (unsigned char)(((6)*alpha0 + alpha1) / 7);
					a_codes[3] = (unsigned char)(((5)*alpha0 + 2*alpha1) / 7);
					a_codes[4] = (unsigned char)(((4)*alpha0 + 3*alpha1) / 7);
					a_codes[5] = (unsigned char)(((3)*alpha0 + 4*alpha1) / 7);
					a_codes[6] = (unsigned char)(((2)*alpha0 + 5*alpha1) / 7);
					a_codes[7] = (unsigned char)(((1)*alpha0 + 6*alpha1) / 7);
				}

				value = (src[0]) | (src[1] << 8) | (src[2] << 16);
				a_indices[0] = (unsigned char)((value)& 0x7);
				a_indices[1] = (unsigned char)((value >> 3) & 0x7);
				a_indices[2] = (unsigned char)((value >> 6) & 0x7);
				a_indices[3] = (unsigned char)((value >> 9) & 0x7);
				a_indices[4] = (unsigned char)((value >> 12) & 0x7);
				a_indices[5] = (unsigned char)((value >> 15) & 0x7);
				a_indices[6] = (unsigned char)((value >> 18) & 0x7);
				a_indices[7] = (unsigned char)((value >> 21) & 0x7);
				value = (src[3]) | (src[4] << 8) | (src[5] << 16);
				a_indices[8] = (unsigned char)((value)& 0x7);
				a_indices[9] = (unsigned char)((value >> 3) & 0x7);
				a_indices[10] = (unsigned char)((value >> 6) & 0x7);
				a_indices[11] = (unsigned char)((value >> 9) & 0x7);
				a_indices[12] = (unsigned char)((value >> 12) & 0x7);
				a_indices[13] = (unsigned char)((value >> 15) & 0x7);
				a_indices[14] = (unsigned char)((value >> 18) & 0x7);
				a_indices[15] = (unsigned char)((value >> 21) & 0x7);

				// decode color
				color0 = Unpack565(sourceBlock + 8, codes);
				color1 = Unpack565(sourceBlock + 10, codes + 1);
				block = sourceBlock + 12;

				// generate the midpoints
				c = codes[0].m_Texel.r;
				d = codes[1].m_Texel.r;
				codes[2].m_Texel.r = (unsigned char)((2 * c + d) / 3);
				codes[3].m_Texel.r = (unsigned char)((c + 2 * d) / 3);
				c = codes[0].m_Texel.g;
				d = codes[1].m_Texel.g;
				codes[2].m_Texel.g = (unsigned char)((2 * c + d) / 3);
				codes[3].m_Texel.g = (unsigned char)((c + 2 * d) / 3);
				c = codes[0].m_Texel.b;
				d = codes[1].m_Texel.b;
				codes[2].m_Texel.b = (unsigned char)((2 * c + d) / 3);
				codes[3].m_Texel.b = (unsigned char)((c + 2 * d) / 3);

				int idx2 = 0;
				for (int py = 0; py < 4; ++py)
				{
					packed = block[py];
					sy = y + py;
					if ((x + 3) < width && sy < height)
					{
						idx1 = (width*sy + x);
						out_rgb[idx1].m_UIColor = codes[(packed & 0x3)].m_UIColor;
						out_rgb[idx1].m_Texel.a = a_codes[a_indices[idx2]];
						idx1 = (width*sy + (x + 1));
						out_rgb[idx1].m_UIColor = codes[((packed >> 2) & 0x3)].m_UIColor;
						out_rgb[idx1].m_Texel.a = a_codes[a_indices[idx2 + 1]];
						idx1 = (width*sy + (x + 2));
						out_rgb[idx1].m_UIColor = codes[((packed >> 4) & 0x3)].m_UIColor;
						out_rgb[idx1].m_Texel.a = a_codes[a_indices[idx2 + 2]];
						idx1 = (width*sy + (x + 3));
						out_rgb[idx1].m_UIColor = codes[((packed >> 6) & 0x3)].m_UIColor;
						out_rgb[idx1].m_Texel.a = a_codes[a_indices[idx2 + 3]];
					}
					idx2 += 4;
				}
				sourceBlock += 16;
			}
		}
	}
}

void drgDibLayer::WriteToTex(const char *filename)
{
	drgFile ofile;
	if(!ofile.Open(filename,drgFile::DRG_MODE_WRITE_BIN))
		return;

	bool use_alpha = HasAlpha();
	bool use_cut = IsCutOut();

	unsigned int datasize = m_Width*m_Height*4;
	unsigned char* destTexture = (unsigned char*)drgMemAlloc(datasize);
	if(!use_alpha)
	{
		datasize = m_Width*m_Height*2;
		unsigned short* pixptr = (unsigned short*)destTexture;
		int curpixel = 0;
		for (int y=0;y<m_Height;y++)
		{
			for (int x=0;x<m_Width;x++)
			{
				curpixel = (y*m_Width)+x;
				pixptr[curpixel] = drgRGB1555(m_pBits[curpixel].m_Texel.b, m_pBits[curpixel].m_Texel.g, m_pBits[curpixel].m_Texel.r);
			}
		}
	}
	else if(use_cut)
	{
		datasize = m_Width*m_Height*2;
		unsigned short* pixptr = (unsigned short*)destTexture;
		int curpixel = 0;
		for (int y=0;y<m_Height;y++)
		{
			for (int x=0;x<m_Width;x++)
			{
				curpixel = (y*m_Width)+x;
				pixptr[curpixel] = drgRGBA1555(m_pBits[curpixel].m_Texel.b, m_pBits[curpixel].m_Texel.g, m_pBits[curpixel].m_Texel.r, m_pBits[curpixel].m_Texel.a);
			}
		}
	}
	else
	{
		datasize = m_Width*m_Height*2;
		unsigned short* pixptr = (unsigned short*)destTexture;
		int curpixel = 0;
		for (int y=0;y<m_Height;y++)
		{
			for (int x=0;x<m_Width;x++)
			{
				curpixel = (y*m_Width)+x;
				pixptr[curpixel] = drgRGBA4444(m_pBits[curpixel].m_Texel.a, m_pBits[curpixel].m_Texel.b, m_pBits[curpixel].m_Texel.g, m_pBits[curpixel].m_Texel.r);
			}
		}
	}

	DRG_DDS_HEADER ddsd;
	memset(&ddsd, 0, sizeof(DRG_DDS_HEADER));
	ddsd.m_Magic[0] = 'D';
	ddsd.m_Magic[1] = 'D';
	ddsd.m_Magic[2] = 'S';
	ddsd.m_Magic[3] = ' ';
	ddsd.m_Size = sizeof(DRG_DDS_HEADER);
	ddsd.m_Flags |= DRG_DDSD_LINEARSIZE | DRG_DDSD_MIPMAPCOUNT | DRG_DDSD_WIDTH | DRG_DDSD_HEIGHT | DRG_DDSD_CAPS | DRG_DDSD_PIXELFORMAT;
	ddsd.m_Width = m_Width;
	ddsd.m_Height = m_Height;
	ddsd.m_MipMapCount = 1;

	ddsd.m_SizeOfPixelFormat = 32;
	ddsd.m_FlagsOfPixelFormat = DRG_DDPF_FOURCC;
	ddsd.m_FourCharIdOfPixelFormat[0] = 'T';
	ddsd.m_FourCharIdOfPixelFormat[1] = 'E';
	ddsd.m_FourCharIdOfPixelFormat[2] = 'X';
	if(!use_alpha)
		ddsd.m_FourCharIdOfPixelFormat[3] = '0';
	else if(use_cut)
		ddsd.m_FourCharIdOfPixelFormat[3] = '1';
	else
		ddsd.m_FourCharIdOfPixelFormat[3] = '2';
	ddsd.m_PitchOrLinearSize = datasize;

	ofile.Write(&ddsd, sizeof(DRG_DDS_HEADER));
	ofile.Write(destTexture, datasize);

	drgMemFree(destTexture);
	ofile.Close();
}

void drgDibInfo::WriteToTga(unsigned int layer, char *filename)
{
	assert(layer<m_Layers);
	bool has_alpha = m_LayerInfo[layer]->HasAlpha();
	m_LayerInfo[layer]->WriteToTga(filename, !has_alpha);
}

void drgDibInfo::ReadFromTga(const char *filename)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromTga(filename);
}

void drgDibInfo::ReadFromTga(void* data, int datalen)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromTga(data, datalen);
}

void drgDibInfo::WriteToBmp(unsigned int layer, char *filename)
{
	assert(layer<m_Layers);
	m_LayerInfo[layer]->WriteToBmp(filename);
}

void drgDibInfo::ReadFromBmp(const char *filename)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromBmp(filename);
}

void drgDibInfo::ReadFromBmp(void* data, int datalen)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromBmp(data, datalen);
}

void drgDibInfo::WriteToPng(unsigned int layer, char *filename)
{
	assert(layer<m_Layers);
	m_LayerInfo[layer]->WriteToPng(filename);
}

void drgDibInfo::ReadFromPng(const char *filename)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromPng(filename);
}

void drgDibInfo::ReadFromPng(void* data, int datalen)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromPng(data, datalen);
}

void drgDibInfo::WriteToJpg(unsigned int layer, char *filename)
{
	assert(layer<m_Layers);
	m_LayerInfo[layer]->WriteToJpg(filename);
}

void drgDibInfo::ReadFromJpg(const char *filename)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromJpg(filename);
}

void drgDibInfo::ReadFromJpg(void* data, int datalen)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromJpg(data, datalen);
}

void drgDibInfo::WriteToDxt(unsigned int layer, char *filename)
{
	assert(layer<m_Layers);
	m_LayerInfo[layer]->WriteToDxt(filename);
}

void drgDibInfo::ReadFromDxt(const char *filename)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromDxt(filename);
}

void drgDibInfo::ReadFromDxt(void* data, int datalen)
{
	CleanUp();
	m_Layers = 1;
	m_LayerInfo = (drgDibLayer**)drgMemAlloc(sizeof(drgDibLayer*));
	MEM_INFO_SET_NEW;
	m_LayerInfo[0] = new drgDibLayer(m_Description);
	m_LayerInfo[0]->ReadFromDxt(data, datalen);
}

void drgDibInfo::WriteToTex(unsigned int layer, char *filename)
{
	assert(layer<m_Layers);
	m_LayerInfo[layer]->WriteToTex(filename);
}

void drgDibInfo::WriteToImageFile(unsigned int layer, char *filename)
{ // read from bmp,tga or jpg depending on the file name
	if(!drgString::CompareNoCase("tga", filename+drgString::Length(filename)-3))
		WriteToTga(layer, filename);
	else if(!drgString::CompareNoCase("bmp", filename+drgString::Length(filename)-3))
		WriteToBmp(layer, filename);
	else if(!drgString::CompareNoCase("png", filename+drgString::Length(filename)-3))
		WriteToPng(layer, filename);
	else if(!drgString::CompareNoCase("jpg", filename+drgString::Length(filename)-3))
		WriteToJpg(layer, filename);
	else if(!drgString::CompareNoCase("dds", filename+drgString::Length(filename)-3))
		WriteToDxt(layer, filename);
	else if(!drgString::CompareNoCase("tex", filename+drgString::Length(filename)-3))
		WriteToTex(layer, filename);
}

void drgDibInfo::ReadFromImageFile(const char *filename)
{ // read from bmp,tga or jpg depending on the file name
	if(!drgString::CompareNoCase("tga", filename+drgString::Length(filename)-3))
		ReadFromTga(filename);
	else if(!drgString::CompareNoCase("bmp", filename+drgString::Length(filename)-3))
		ReadFromBmp(filename);
	else if(!drgString::CompareNoCase("png", filename+drgString::Length(filename)-3))
		ReadFromPng(filename);
	else if (!drgString::CompareNoCase("jpg", filename + drgString::Length(filename) - 3))
		ReadFromJpg(filename);
	else if (!drgString::CompareNoCase("dds", filename + drgString::Length(filename) - 3))
		ReadFromDxt(filename);
}

void drgDibInfo::ReadFromImageInMemory(void* buf, int buflen)
{
	const char szJPEGCommonHeader[] = { (char)0xFF, (char)0xD8, (char)0xFF };
	const char szTiffHeaderForMotorola[] = "MM*";
	const char szTiffHeaderForIntel[] = "II*";
	const char szDDSHeader[] = "DDS ";
	const char szRawTGA[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	const char szCmpTGA[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header
	const char szPNGHeader[] = "\x89PNG\r\n\x1a\n";
	const char szBMPHeader[] = { 0x42, 0x4D };
	
	if (!memcmp(buf, szJPEGCommonHeader, 3))
		ReadFromJpg(buf, buflen);
	else if ((!memcmp(buf, szTiffHeaderForIntel, 3)) || (!memcmp(buf, szTiffHeaderForMotorola, 3)) || (!memcmp(buf, szRawTGA, 12)) || (!memcmp(buf, szCmpTGA, 12)))
		ReadFromTga(buf, buflen);
	else if( !memcmp(buf, szPNGHeader, 8))
		ReadFromPng(buf, buflen);
	else if (!memcmp(buf, szBMPHeader, 2))
		ReadFromBmp(buf, buflen);
	else if (!memcmp(buf, szDDSHeader, 4))
		ReadFromDxt(buf, buflen);
}
