
#include "util/dib/dib.h"
#include "system/file.h"
#include "util/stringn.h"
#include "util/math_def.h"

#define Mitchell_support (2.0)
#define Lanczos3_support (3.0)

#define FILTER_DIM 7
float dibGuassianFilter[FILTER_DIM][FILTER_DIM] = {
	{1, 4, 8, 10, 8, 4, 1},
	{4, 12, 25, 29, 25, 12, 4},
	{8, 25, 49, 58, 49, 25, 8},
	{10, 29, 58, 67, 58, 29, 10},
	{8, 25, 49, 58, 49, 25, 8},
	{4, 12, 25, 29, 25, 12, 4},
	{1, 4, 8, 10, 8, 4, 1},

	//{1, 4, 	8, 	10, 8, 	4, 	1},
	//{4,	12, 25, 29, 25, 12, 4},
	//{8,	25, 49, 58, 49, 25, 8},
	//{10, 29, 58, 67, 58, 29, 10},
	//{8,	25, 49, 58, 49, 25, 8},
	//{4,	12, 25, 29, 25, 12, 4},
	//{1,	4, 	8, 	10, 8, 	4, 	1},
};

float dibBlurFilter[FILTER_DIM][FILTER_DIM] = {
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 0, 1, 1, 1},
	{1, 1, 0, 0, 0, 1, 1},
	{1, 1, 1, 0, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1},
};

typedef struct
{
	int pixel;
	double weight;
} CONTRIB;

typedef struct
{
	int n;		// number of contributors
	CONTRIB *p; // pointer to list of contributions
} CLIST;

static double Mitchell_filter(double t)
{
	const double B = 1.0 / 3.0;
	const double C = 1.0 / 3.0;

	double tt;

	tt = t * t;
	if (t < 0)
		t = -t;
	if (t < 1.0)
	{
		t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt)) + ((-18.0 + 12.0 * B + 6.0 * C) * tt) + (6.0 - 2 * B));
		return t / 6.0;
	}
	else if (t < 2.0)
	{
		t = (((-1.0 * B - 6.0 * C) * (t * tt)) + ((6.0 * B + 30.0 * C) * tt) + ((-12.0 * B - 48.0 * C) * t) + (8.0 * B + 24 * C));
		return t / 6.0;
	}
	return 0.0;
}

static double sinc(double x)
{
	x *= 3.1415926f;
	if (x != 0)
		return (sin(x) / x);
	return (1.0);
}

static double Lanczos3_filter(double t)
{
	if (t < 0)
		t = -t;
	if (t < 3.0)
		return (sinc(t) * sinc(t / 3.0));
	return (0.0);
}

// calc_x_contrib()
//
// Calculates the filter weights for a single target column.
// contribX->p must be freed afterwards.
//
// Returns -1 if error, 0 otherwise.
static int calc_x_contrib(CLIST *contribX, double xscale, double fwidth, int dstwidth, int srcwidth, double (*filterf)(double), int i)
{
	double width;
	double fscale;
	double center, left, right;
	double weight;
	int j, k, n;

	if (xscale < 1.0)
	{
		// Shrinking Image
		width = fwidth / xscale;
		fscale = 1.0 / xscale;

		contribX->n = 0;
		contribX->p = (CONTRIB *)calloc((int)(width * 2 + 1), sizeof(CONTRIB));
		if (contribX->p == NULL)
			return -1;

		center = (double)i / xscale;
		left = DRG_CEIL((float)(center - width));
		right = DRG_FLOOR((float)(center + width));
		for (j = (int)left; j <= right; ++j)
		{
			weight = center - (double)j;
			weight = (*filterf)(weight / fscale) / fscale;
			if (j < 0)
				n = -j;
			else if (j >= srcwidth)
				n = (srcwidth - j) + srcwidth - 1;
			else
				n = j;

			n = DRG_MAX(0, n);
			n = DRG_MIN(srcwidth - 1, n);

			k = contribX->n++;
			assert(k < width * 2 + 1);
			contribX->p[k].pixel = n;
			contribX->p[k].weight = weight;
		}
	}
	else
	{
		// Expanding Image
		contribX->n = 0;
		contribX->p = (CONTRIB *)calloc((int)(fwidth * 2 + 1), sizeof(CONTRIB));
		if (contribX->p == NULL)
			return -1;
		center = (double)i / xscale;
		left = DRG_CEIL((float)(center - fwidth));
		right = DRG_FLOOR((float)(center + fwidth));

		for (j = (int)left; j <= right; ++j)
		{
			weight = center - (double)j;
			weight = (*filterf)(weight);
			if (j < 0)
			{
				n = -j;
			}
			else if (j >= srcwidth)
			{
				n = (srcwidth - j) + srcwidth - 1;
			}
			else
			{
				n = j;
			}

			n = DRG_MAX(0, n);
			n = DRG_MIN(srcwidth - 1, n);

			k = contribX->n++;
			contribX->p[k].pixel = n;
			contribX->p[k].weight = weight;
		}
	}
	return 0;
} // calc_x_contrib

// roundcloser()
// Round an FP value to its closest int representation.
// General routine; ideally belongs in general math lib file.
static int roundcloser(double d)
{
	// Version that uses no function calls at all.
	int n = (int)d;
	double diff = d - (double)n;
	if (diff < 0)
		diff = -diff;
	if (diff >= 0.5)
	{
		if (d < 0)
			n--;
		else
			n++;
	}
	return n;
} // roundcloser

drgDibLayer::drgDibLayer(const char *desc)
{
	m_pBits = NULL;
	m_Width = 0;
	m_Height = 0;
	m_DeltaScan = 0;
	m_NumPixels = 0;
	m_HalfWidth = 0;
	m_HalfHeight = 0;
	m_Flags = DRG_DIB_FLAG_DEFAULT;
	m_Description = desc;
}

drgDibLayer::~drgDibLayer()
{
	CleanUp();
}

void drgDibLayer::CleanUp()
{
	if ((m_pBits))
		drgMemFree(m_pBits);
	m_pBits = NULL;
	m_Width = 0;
	m_Height = 0;
	m_DeltaScan = 0;
	m_NumPixels = 0;
	m_HalfWidth = 0;
	m_HalfHeight = 0;
	m_Flags = DRG_DIB_FLAG_DEFAULT;
}

void drgDibLayer::Init(unsigned int width, unsigned int height, unsigned int flags)
{
	CleanUp();
	m_Flags = flags;
	if ((width > 0) && (height > 0))
	{
		m_Width = width;
		m_Height = height;
		m_DeltaScan = width;
		m_HalfWidth = ((float)width) / 2.0f;
		m_HalfHeight = ((float)height) / 2.0f;
		m_NumPixels = width * height;
#if MEM_ENABLE_DEBUG
		char memdesc[128] = __FILE__;
		drgString::Concatenate(memdesc, ":");
		drgString::Concatenate(memdesc, m_Description);
		m_pBits = (drgDibColor *)g_MainBlocks.ReserveZero(m_Width * m_Height * sizeof(drgDibColor), 32, memdesc, __LINE__);
#else
		m_pBits = (drgDibColor *)drgMemAlloc(m_Width * m_Height * sizeof(drgDibColor));
#endif
	}
	else
	{
		m_Width = 0;
		m_Height = 0;
		m_DeltaScan = 0;
		m_HalfWidth = 0;
		m_HalfHeight = 0;
		m_NumPixels = 0;
		m_pBits = NULL;
	}
}

void drgDibLayer::Fill(void *data)
{
	drgMemory::MemCopy(m_pBits, data, m_NumPixels * sizeof(unsigned int));
}

void drgDibLayer::FillRectColor(drgDibColor color)
{
	unsigned int val = color.m_UIColor;
	unsigned int *pixels = (unsigned int *)m_pBits;
	for (unsigned int count = 0; count < m_NumPixels; ++count)
		pixels[count] = val;
}

bool drgDibLayer::HasAlpha()
{
	for (unsigned int count = 0; count < m_NumPixels; ++count)
		if (m_pBits[count].m_Texel.a <= 250)
			return true;
	return false;
}

bool drgDibLayer::IsCutOut()
{
	for (unsigned int count = 0; count < m_NumPixels; ++count)
		if ((m_pBits[count].m_Texel.a <= 250) && (m_pBits[count].m_Texel.a >= 5))
			return false;
	return true;
}

void drgDibLayer::Scale(unsigned int width_in, unsigned int height_in, FILTER_TYPE filter)
{
	if (width_in == 0 && height_in == 0 && m_Width == 0 && m_Height == 0)
		return;

	if (width_in == m_Width && height_in == m_Height)
		return;

	drgDibColor *buffer;
	buffer = (drgDibColor *)drgMemAlloc(m_Width * m_Height * sizeof(drgDibColor));
	drgMemory::MemCopy(buffer, m_pBits, m_Width * m_Height * sizeof(drgDibColor));

	double (*filterf)(double) = Mitchell_filter;
	double fwidth = Mitchell_support;

	switch (filter)
	{
	case FILTER_MITCHELL:
		filterf = Mitchell_filter;
		fwidth = Mitchell_support;
		break;
	case FILTER_LANCZOS:
		filterf = Lanczos3_filter;
		fwidth = Lanczos3_support;
		break;
	}

	drgDibTexel *tmp;
	double xscale, yscale; // zoom scale factors
	int xx;
	int i, j, k;				  // loop variables
	int n;						  // pixel number
	double center, left, right;	  // filter calculation variables
	double width, fscale, weight; // filter calculation variables
	// drgDibTexel pel, pel2;
	// int bPelDelta;
	CLIST *contribY; // array of contribution lists
	CLIST contribX;
	int nRet = -1;

	int srcwidth = (int)m_Width;
	int srcheight = (int)m_Height;
	int dstwidth = (int)width_in;
	int dstheight = (int)height_in;

	Init(width_in, height_in, DRG_DIB_FLAG_RGBA);

	// create intermediate column to hold horizontal dst column zoom
	tmp = (drgDibTexel *)drgMemAlloc(srcheight * sizeof(drgDibTexel));
	assert(tmp);

	xscale = (double)dstwidth / (double)srcwidth;

	// Build y weights
	// pre-calculate filter contributions for a column
	contribY = (CLIST *)drgMemCAlloc(dstheight, sizeof(CLIST));
	assert(contribY);

	yscale = (double)dstheight / (double)srcheight;

	if (yscale < 1.0)
	{
		width = fwidth / yscale;
		fscale = 1.0 / yscale;
		for (i = 0; i < dstheight; ++i)
		{
			contribY[i].n = 0;
			contribY[i].p = (CONTRIB *)drgMemCAlloc((int)(width * 2 + 1), sizeof(CONTRIB));

			assert(contribY[i].p);
			center = (double)i / yscale;
			left = DRG_CEIL((float)(center - width));
			right = DRG_FLOOR((float)(center + width));
			for (j = (int)left; j <= right; ++j)
			{
				weight = center - (double)j;
				weight = (*filterf)(weight / fscale) / fscale;
				if (j < 0)
				{
					n = -j;
				}
				else if (j >= srcheight)
				{
					n = (srcheight - j) + srcheight - 1;
				}
				else
				{
					n = j;
				}

				n = DRG_MAX(0, n);
				n = DRG_MIN(srcheight - 1, n);

				assert(i >= 0);
				assert(i < dstheight);
				k = contribY[i].n++;
				assert(k >= 0);
				assert(k < (width * 2 + 1));
				contribY[i].p[k].pixel = n;
				contribY[i].p[k].weight = weight;
			}
		}
	}
	else
	{
		for (i = 0; i < dstheight; ++i)
		{
			contribY[i].n = 0;
			contribY[i].p = (CONTRIB *)drgMemCAlloc((int)(fwidth * 2 + 1), sizeof(CONTRIB));

			assert(contribY[i].p);
			center = (double)i / yscale;
			left = DRG_CEIL((float)(center - fwidth));
			right = DRG_FLOOR((float)(center + fwidth));
			for (j = (int)left; j <= right; ++j)
			{
				weight = center - (double)j;
				weight = (*filterf)(weight);
				if (j < 0)
				{
					n = -j;
				}
				else if (j >= srcheight)
				{
					n = (srcheight - j) + srcheight - 1;
				}
				else
				{
					n = j;
				}

				n = DRG_MAX(0, n);
				n = DRG_MIN(srcheight - 1, n);

				k = contribY[i].n++;
				contribY[i].p[k].pixel = n;
				contribY[i].p[k].weight = weight;
			}
		}
	}

	int curpixel;
	for (xx = 0; xx < dstwidth; xx++)
	{
		calc_x_contrib(&contribX, xscale, fwidth, dstwidth, srcwidth, filterf, xx);
		// Apply horz filter to make dst column in tmp.
		for (k = 0; k < srcheight; ++k)
		{
			double r = 0, g = 0, b = 0, a = 0;
			weight = 0.0;

			for (j = 0; j < contribX.n; ++j)
			{
				assert(contribX.p[j].pixel < srcwidth);
				curpixel = (k * srcwidth) + contribX.p[j].pixel;
				drgDibTexel p = buffer[curpixel].m_Texel;
				double pw = contribX.p[j].weight;

				if (p.r == 0 && p.g == 255 && p.b == 0)
					continue;

				r += p.r * pw;
				g += p.g * pw;
				b += p.b * pw;
				a += p.a * pw;
				weight += pw;
			}

			if (weight == 0)
			{
				r = 0;
				g = 255;
				b = 0;
				a = 0;
			}
			else
			{
				double oow = 1.f / weight;
				r *= oow;
				g *= oow;
				b *= oow;
				a *= oow;
			}

			r = roundcloser(r);
			r = DRG_MAX(r, 0.);
			r = DRG_MIN(r, 255.);
			g = roundcloser(g);
			g = DRG_MAX(g, 0.);
			g = DRG_MIN(g, 255.);
			b = roundcloser(b);
			b = DRG_MAX(b, 0.);
			b = DRG_MIN(b, 255.);
			a = roundcloser(a);
			a = DRG_MAX(a, 0.);
			a = DRG_MIN(a, 255.);

			tmp[k].r = (unsigned int)r;
			tmp[k].g = (unsigned int)g;
			tmp[k].b = (unsigned int)b;
			tmp[k].a = (unsigned int)a;
		} // next row in temp column

		drgMemFree(contribX.p);

		// The temp column has been built. Now stretch it
		// vertically into dst column.
		for (i = 0; i < dstheight; ++i)
		{
			double r = 0, g = 0, b = 0, a = 0;
			weight = 0.0;

			for (j = 0; j < contribY[i].n; ++j)
			{
				assert(contribY[i].p[j].pixel < srcheight);
				drgDibTexel p = tmp[contribY[i].p[j].pixel];
				double pw = contribY[i].p[j].weight;

				if (p.r == 0 && p.g == 255 && p.b == 0)
					continue;

				r += p.r * pw;
				g += p.g * pw;
				b += p.b * pw;
				a += p.a * pw;
				weight += pw;
			}

			if (weight == 0)
			{
				r = 0;
				g = 255;
				b = 0;
				a = 0;
			}
			else
			{
				double oow = 1.f / weight;
				r *= oow;
				g *= oow;
				b *= oow;
				a *= oow;
			}

			r = roundcloser(r);
			r = DRG_MAX(r, 0.);
			r = DRG_MIN(r, 255.);
			g = roundcloser(g);
			g = DRG_MAX(g, 0.);
			g = DRG_MIN(g, 255.);
			b = roundcloser(b);
			b = DRG_MAX(b, 0.);
			b = DRG_MIN(b, 255.);
			a = roundcloser(a);
			a = DRG_MAX(a, 0.);
			a = DRG_MIN(a, 255.);

			assert(xx < dstwidth);
			assert(i < dstheight);
			curpixel = (i * dstwidth) + xx;
			m_pBits[curpixel].m_Texel.r = (unsigned char)r;
			m_pBits[curpixel].m_Texel.g = (unsigned char)g;
			m_pBits[curpixel].m_Texel.b = (unsigned char)b;
			m_pBits[curpixel].m_Texel.a = (unsigned char)a;
		} // next dst row
	} // next dst column

	drgMemFree(buffer);
	drgMemFree(tmp);

	// free the memory allocated for vertical filter weights
	for (i = 0; i < dstheight; ++i)
		drgMemFree(contribY[i].p);
	drgMemFree(contribY);
}

void drgDibLayer::GaussianScale(unsigned int width_in, unsigned int height_in, int filter)
{
	if (width_in == 0 && height_in == 0 && m_Width == 0 && m_Height == 0)
		return;

	if (width_in == m_Width && height_in == m_Height)
		return;

	drgDibColor *buffer;
	buffer = (drgDibColor *)drgMemAlloc(m_Width * m_Height * sizeof(drgDibColor));

	drgMemory::MemCopy(buffer, m_pBits, m_Width * m_Height * sizeof(drgDibColor));

	unsigned int i, j, srcX, srcY;
	int k, m, texPos, startX, startY, endX, endY, samples;
	float sumR, sumG, sumB, sumA, mag, sumDiv, filterSum, srcStepX, srcStepY, realSrcX, realSrcY;

	unsigned int width_old = m_Width;
	unsigned int height_old = m_Height;

	srcStepX = (float)m_Width / (float)width_in;
	srcStepY = (float)m_Height / (float)height_in;

	Init(width_in, height_in, DRG_DIB_FLAG_RGBA);

	realSrcY = 0;
	int curpixel;
	for (i = 0; i < height_in; i++)
	{
		realSrcX = 0;
		// Clip the filter scan in Y
		if (i < FILTER_DIM / 2)
		{
			startY = FILTER_DIM / 2 - i;
			endY = FILTER_DIM;
		}
		else
		{
			startY = 0;
			if ((height_in - i) <= FILTER_DIM / 2)
				endY = (height_in - i) + FILTER_DIM / 2;
			else
				endY = FILTER_DIM;
		}
		for (j = 0; j < width_in; j++)
		{
			// Clip the filter scan in X
			if (j < FILTER_DIM / 2)
			{
				startX = FILTER_DIM / 2 - j;
				endX = FILTER_DIM;
			}
			else
			{
				startX = 0;
				if ((width_in - j) <= FILTER_DIM / 2)
					endX = (width_in - j) + FILTER_DIM / 2;
				else
					endX = FILTER_DIM;
			}
			sumR = sumG = sumB = sumA = 0;
			filterSum = 0;
			for (k = startY; k < endY; k++) // Step through the filter at this location.
			{
				for (m = startX; m < endX; m++)
				{
					srcX = DRG_ROUND(realSrcX) + (m - FILTER_DIM / 2);
					srcY = DRG_ROUND(realSrcY) + (k - FILTER_DIM / 2);
					if ((srcX >= 0) && (srcY >= 0) && (srcX < width_old) && (srcY < height_old))
					{
						mag = filter ? dibBlurFilter[k][m] : dibGuassianFilter[k][m];
						filterSum += mag;
						curpixel = (srcY * width_old) + srcX;
						sumR += (float)buffer[curpixel].m_Texel.r * mag;
						sumG += (float)buffer[curpixel].m_Texel.g * mag;
						sumB += (float)buffer[curpixel].m_Texel.b * mag;
						sumA += (float)buffer[curpixel].m_Texel.a * mag;
					}
				}
			}
			samples = (endY - startY) * (endX - startX);
			sumDiv = 1.0f / filterSum;
			texPos = (i * width_in * 4) + j * 4;
			curpixel = (i * width_in) + j;
			m_pBits[curpixel].m_Texel.r = DRG_MIN(255, DRG_ROUND(sumR * sumDiv));
			m_pBits[curpixel].m_Texel.g = DRG_MIN(255, DRG_ROUND(sumG * sumDiv));
			m_pBits[curpixel].m_Texel.b = DRG_MIN(255, DRG_ROUND(sumB * sumDiv));
			m_pBits[curpixel].m_Texel.a = DRG_MIN(255, DRG_ROUND(sumA * sumDiv));

			realSrcX += srcStepX;
		}
		realSrcY += srcStepY;
	}
	drgMemFree(buffer);
}

void drgDibLayer::Resize(unsigned int width, unsigned int height)
{
	if (width == 0 && height == 0 && m_Width == 0 && m_Height == 0)
		return;

	if (width == m_Width && height == m_Height)
		return;

	drgDibColor *buffer;
	buffer = (drgDibColor *)drgMemAlloc(m_Width * m_Height * sizeof(drgDibColor));
	drgMemory::MemCopy(buffer, m_pBits, m_Width * m_Height * sizeof(drgDibColor));
	unsigned int i, x, y;
	int curpixel1, curpixel2;
	unsigned int countwidth = DRG_MIN(width, m_Width);
	unsigned int countheight = DRG_MIN(height, m_Height);
	int oldstride = m_Width;
	Init(width, height, DRG_DIB_FLAG_RGBA);
	for (i = 0, y = 0; y < countheight; ++y)
	{
		for (x = 0; x < countwidth; ++x)
		{
			curpixel1 = (y * m_Width) + x;
			curpixel2 = (y * oldstride) + x;
			m_pBits[curpixel1].m_Texel.r = buffer[curpixel2].m_Texel.r;
			m_pBits[curpixel1].m_Texel.g = buffer[curpixel2].m_Texel.g;
			m_pBits[curpixel1].m_Texel.b = buffer[curpixel2].m_Texel.b;
			m_pBits[curpixel1].m_Texel.a = buffer[curpixel2].m_Texel.a;
		}
	}
	drgMemFree(buffer);
}

void drgDibLayer::PadToPowerOf2()
{
	int width_new = RoundUpToPowerOfTwo(m_Width);
	int height_new = RoundUpToPowerOfTwo(m_Height);
	if ((width_new != m_Width) || (height_new != m_Height))
		Resize(width_new, height_new);
	// Scale(width_new, height_new, FILTER_TYPE::FILTER_LANCZOS);
}

drgDibInfo::drgDibInfo(const char *desc)
{
	m_Layers = 0;
	m_LayerInfo = NULL;
	m_Description[0] = '\0';
	if (desc != NULL)
		drgString::Copy(m_Description, desc);
}

drgDibInfo::~drgDibInfo()
{
	CleanUp();
}

void drgDibInfo::CleanUp()
{
	if (m_Layers == 0)
		return;

	while (m_Layers > 0)
	{
		m_Layers--;
		delete m_LayerInfo[m_Layers];
	}
	drgMemFree(m_LayerInfo);
	m_LayerInfo = NULL;
	m_Layers = 0;
}

void drgDibInfo::Init(unsigned int layers, unsigned int width, unsigned int height, unsigned int flags)
{
	CleanUp();
	m_Layers = layers;
	if (m_Layers == 0)
		return;
	m_LayerInfo = (drgDibLayer **)drgMemAlloc(layers * sizeof(drgDibLayer *));
	for (unsigned int layer = 0; layer < m_Layers; layer++)
	{
		MEM_INFO_SET_NEW;
		m_LayerInfo[layer] = new drgDibLayer(m_Description);
		m_LayerInfo[layer]->Init(width, height, flags);
		width /= 2;
		height /= 2;
	}
}

bool drgDibInfo::HasAlpha()
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		if (m_LayerInfo[layer]->HasAlpha())
			return true;
	return false;
}

bool drgDibInfo::IsCutOut()
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		if (m_LayerInfo[layer]->IsCutOut())
			return true;
	return false;
}

void drgDibInfo::PadToPowerOf2(unsigned int _layer)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->PadToPowerOf2();
}

void drgDibInfo::Resize(int width_new, int height_new)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->Resize(width_new, height_new);
}

void drgDibInfo::GaussianScale(int width_new, int height_new, int filter)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->GaussianScale(width_new, height_new, filter);
}

void drgDibInfo::Scale(int width_new, int height_new, drgDibLayer::FILTER_TYPE filter)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->Scale(width_new, height_new, filter);
}

void drgDibInfo::FillRectRGB(unsigned int layer, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	drgDibColor cval;
	cval.m_Texel.r = r;
	cval.m_Texel.g = g;
	cval.m_Texel.b = b;
	cval.m_Texel.a = a;
	FillRectColor(layer, cval);
	FlagsRemove(DRG_DIB_FLAG_FLOAT | DRG_DIB_FLAG_UINT);
	FlagsAdd(DRG_DIB_FLAG_RGBA);
}

void drgDibInfo::FillRectUI(unsigned int layer, unsigned int val)
{
	drgDibColor cval;
	cval.m_UIColor = val;
	FillRectColor(layer, cval);
	FlagsRemove(DRG_DIB_FLAG_FLOAT | DRG_DIB_FLAG_RGBA);
	FlagsAdd(DRG_DIB_FLAG_UINT);
}

void drgDibInfo::FillRectFloat(unsigned int layer, float val)
{
	drgDibColor cval;
	cval.m_FColor = val;
	FillRectColor(layer, cval);
	FlagsRemove(DRG_DIB_FLAG_RGBA | DRG_DIB_FLAG_UINT);
	FlagsAdd(DRG_DIB_FLAG_FLOAT);
}

void drgDibInfo::Fill(unsigned int layer, void *data)
{
	assert(layer < m_Layers);
	m_LayerInfo[layer]->Fill(data);
}

void drgDibInfo::FillRectColor(unsigned int layer, drgDibColor color)
{
	assert(layer < m_Layers);
	m_LayerInfo[layer]->FillRectColor(color);
}

void drgDibInfo::BuildLayers()
{
	assert(0 < m_Layers);
	for (unsigned int layer = 1; layer < m_Layers; layer++)
		m_LayerInfo[layer - 1]->GaussianScale(m_LayerInfo[layer]->m_Width, m_LayerInfo[layer]->m_Height, 0);
}

void drgDibInfo::FlagsSet(unsigned int flag)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->m_Flags = flag;
}

void drgDibInfo::FlagsAdd(unsigned int flag)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->m_Flags |= flag;
}

void drgDibInfo::FlagsRemove(unsigned int flag)
{
	assert(0 < m_Layers);
	for (unsigned int layer = 0; layer < m_Layers; layer++)
		m_LayerInfo[layer]->m_Flags &= (~flag);
}

bool drgDibInfo::HasFlag(unsigned int layer, unsigned int flag)
{
	assert(0 < m_Layers);
	return !!(m_LayerInfo[layer]->m_Flags & flag);
}
