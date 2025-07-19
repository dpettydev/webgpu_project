#ifndef __DRG_COLOR_H__
#define __DRG_COLOR_H__

#define DRG_COLOR_MAX(a)				(((a) < (255)) ? (a) : (255))
#define DRG_COLOR_MIN(a)				(((a) > (0))   ? (a) : (0))
#define DRG_COLOR_CLAMP(a)				(((a) < (255)) ? ((((a) > (0))   ? (a) : (0))) : (255))

#define DRG_5TO8R(val)						(((val>>11)&0x001F)*8)
#define DRG_6TO8G(val)						(((val>>5)&0x003F)*4)
#define DRG_5TO8B(val)						((val&0x001F)*8)
#define DRG_COLOR_FROM_565(col,val)			{ col.r=DRG_5TO8R(val); col.g=DRG_6TO8G(val); col.b=DRG_5TO8B(val); }
#define DRG_COLORP_FROM_565(col,val)		{ col->r=DRG_5TO8R(val); col->g=DRG_6TO8G(val); col->b=DRG_5TO8B(val); }
#define DRG_COLORP_MIX_1_565(col,val1,val2)	{ col->r = ((DRG_5TO8R(val1)+DRG_5TO8R(val2))/2); col->g = ((DRG_6TO8G(val1)+DRG_6TO8G(val2))/2); col->b = ((DRG_5TO8B(val1)+DRG_5TO8B(val2))/2); }
#define DRG_COLORP_MIX_2_565(col,val1,val2)	{ col->r = ((DRG_5TO8R(val1)*2+DRG_5TO8R(val2))/3); col->g = ((DRG_6TO8G(val1)*2+DRG_6TO8G(val2))/3); col->b = ((DRG_5TO8B(val1)*2+DRG_5TO8B(val2))/3); }

//!Class containing red,green,blue and alpha color information as well as methods to operate on them
class drgColor
{
public:
	drgColor();
	drgColor( unsigned int _color );
	drgColor( unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a=255 );
	~drgColor();

	void Set( unsigned int _color );
	void Set( drgColor* _color );
	void Set( unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a=255 );
	void SetRandom( unsigned char _min=0, unsigned char _max=255 );
	void Set565( unsigned short _color );
	unsigned short	Get4444( unsigned short* outcolor=0 );
	unsigned int	Get8888( unsigned int* outcolor=0 );
	unsigned int	GetABGR( unsigned int* outcolor=0 );
	unsigned int	GetRGB( unsigned int* outcolor=0 );

	void Lerp(drgColor *color0, drgColor *color1, float ammount);

	inline drgColor operator=(drgColor color);
	inline drgColor operator*(float s) const;
	inline drgColor operator/(float s) const;
	inline drgColor operator+(const drgColor &c) const;
	inline drgColor operator-(const drgColor &c) const;
	inline drgColor operator+=(const drgColor &c);

	unsigned char	
		//!alpha value
		a,
		//!red value
		r,
		//!green value
		g,
		//!blue value
		b;
};

//!Overloaded == operator
inline bool operator==( const drgColor &lhs, const drgColor &rhs )
{
	return ((lhs.r==rhs.r) && (lhs.g==rhs.g) && (lhs.b==rhs.b) && (lhs.a==rhs.a));
}

//!Overloaded = operator
inline drgColor drgColor::operator=(drgColor color)
{
	r=color.r;
	g=color.g;
	b=color.b;
	a=color.a;
	return (*this);
}

//!Overloaded * operator for scaling
inline drgColor drgColor::operator*(float s) const
{
	return drgColor( (unsigned char)DRG_COLOR_CLAMP(r*s), (unsigned char)DRG_COLOR_CLAMP(g*s), (unsigned char)DRG_COLOR_CLAMP(b*s));
}

//!Overloaded / operator for scaling
inline drgColor drgColor::operator/(float s) const
{
	return drgColor( (unsigned char)DRG_COLOR_CLAMP(r/s), (unsigned char)DRG_COLOR_CLAMP(g/s), (unsigned char)DRG_COLOR_CLAMP(b/s));
}

//!Overloaded + operator for color addition
inline drgColor drgColor::operator+(const drgColor &p) const
{
	return drgColor( (unsigned char)DRG_COLOR_CLAMP(r+p.r), (unsigned char)DRG_COLOR_CLAMP(g+p.g), (unsigned char)DRG_COLOR_CLAMP(b+p.b) );
}

//!Overloaded - operator for color addition
inline drgColor drgColor::operator-(const drgColor &p) const
{
	return drgColor( (unsigned char)DRG_COLOR_CLAMP(r-p.r), (unsigned char)DRG_COLOR_CLAMP(g-p.g), (unsigned char)DRG_COLOR_CLAMP(b-p.b) );
}

//!Overloaded + operator for color addition
inline drgColor drgColor::operator+=(const drgColor &p)
{
	r = (unsigned char)DRG_COLOR_CLAMP(r+p.r);
	g = (unsigned char)DRG_COLOR_CLAMP(g+p.g);
	b = (unsigned char)DRG_COLOR_CLAMP(b+p.b);
	a = (unsigned char)DRG_COLOR_CLAMP(a+p.a);
	return (*this);
}

#ifdef _PSP
#define drgARGB(a,r,g,b) ((int)((((int)(b))<<0)|(((int)(g))<<8)|(((int)(r))<<16)|(((int)(a))<<24)))
#define drgRGBA(r,g,b,a) ((int)((((int)(r))<<0)|(((int)(g))<<8)|(((int)(b))<<16)|(((int)(a))<<24)))
#elif defined _WII
#define drgRGBA(r,g,b,a) ((int)((((int)(a))<<0)|(((int)(b))<<8)|(((int)(g))<<16)|(((int)(r))<<24)))
#define drgARGB(a,r,g,b) ((int)((((int)(a))<<0)|(((int)(r))<<8)|(((int)(g))<<16)|(((int)(b))<<24)))
#elif defined _PS3
#define drgRGBA(r,g,b,a) ((int)((((int)(a))<<0)|(((int)(b))<<8)|(((int)(g))<<16)|(((int)(r))<<24)))
#define drgARGB(a,r,g,b) ((int)((((int)(a))<<0)|(((int)(r))<<8)|(((int)(g))<<16)|(((int)(b))<<24)))
#elif  (defined(_IPHONE) || defined(_MAC))
#define drgRGBA(r,g,b,a) ((int)((((int)(r))<<0)|(((int)(g))<<8)|(((int)(b))<<16)|(((int)(a))<<24)))
#define drgARGB(a,r,g,b) ((int)((((int)(r))<<0)|(((int)(g))<<8)|(((int)(b))<<16)|(((int)(a))<<24)))
#else
#define drgRGBA(r,g,b,a) ((int)((((int)(b))<<0)|(((int)(g))<<8)|(((int)(r))<<16)|(((int)(a))<<24)))
#define drgARGB(a,r,g,b) ((int)((((int)(r))<<0)|(((int)(g))<<8)|(((int)(b))<<16)|(((int)(a))<<24)))
#endif

#define drgRGB(r,g,b) drgRGBA(r,g,b,255)
#define drgRGBA4444(r,g,b,a) (((unsigned char)DRG_MIN(15,a/16)<<12) | ((unsigned char)DRG_MIN(15,b/16)<<8) | ((unsigned char)DRG_MIN(15,g/16)<<4) | (unsigned char)DRG_MIN(15,r/16))
#define drgRGB565(r,g,b) (((unsigned char)DRG_MIN(31,r/8)<<11) | ((unsigned char)DRG_MIN(63,g/4)<<5) | (unsigned char)DRG_MIN(30,b/8))
#define drgRGB5551(r,g,b) ((unsigned short)(1<<15) | ((unsigned char)DRG_MIN(31,b/8)<<10) | ((unsigned char)DRG_MIN(31,g/8)<<5) | (unsigned char)DRG_MIN(31,r/8))
#define drgRGB1555(r,g,b) (((unsigned char)DRG_MIN(31,b/8)<<11) | ((unsigned char)DRG_MIN(31,g/8)<<6) | ((unsigned char)DRG_MIN(31,r/8)<<1) | ((unsigned char)1))
#define drgRGBA5551(r,g,b,a) ((unsigned short)((!!a)<<15) | ((unsigned char)DRG_MIN(31,b/8)<<10) | ((unsigned char)DRG_MIN(31,g/8)<<5) | (unsigned char)DRG_MIN(31,r/8))
#define drgRGBA1555(r,g,b,a) (((unsigned char)DRG_MIN(31,b/8)<<11) | ((unsigned char)DRG_MIN(31,g/8)<<6) | ((unsigned char)DRG_MIN(31,r/8)<<1) | ((unsigned char)(!!a)))
#define drgC8888_2_C5551(c) drgRGBA5551((c&0x000000FF), ((c&0x0000FF00)>>8), ((c&0x00FF0000)>>16), ((c&0xFF000000)>>24))

//Basic colors
extern drgColor drg_Red;
extern drgColor drg_Green;
extern drgColor drg_Blue;
extern drgColor drg_Yellow;
extern drgColor drg_Cyan;
extern drgColor drg_Magenta;
extern drgColor drg_Black;
extern drgColor drg_White;
extern drgColor drg_Grey;
extern drgColor drg_BrightOrange;

#endif // __DRG_COLOR_H__


