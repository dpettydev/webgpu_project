#include "util/color.h"
#include "util/math_def.h"
#include "util/rand.h"

drgColor drg_Red(255,0,0);
drgColor drg_Green(0,255,0);
drgColor drg_Blue(0,0,255);
drgColor drg_Yellow(255,255,0);
drgColor drg_Cyan(0,255,255);
drgColor drg_Magenta(255,0,255);
drgColor drg_Black(0,0,0);
drgColor drg_White(255,255,255);
drgColor drg_Grey(128,128,128);
drgColor drg_BrightOrange(244,156,28);

//!Default constructor.
//!Sets color at (255,255,255,255)
drgColor::drgColor()
{
	Set(255, 255, 255, 255);
}

//!Copy constructor
/*!
	Sets current color to the provided color
	\param _color color to use
*/
drgColor::drgColor( unsigned int _color )
{
	Set(_color);
}

//!Constructor accepting individual r,g,b, and a values
/*!
	\param _r red valule
	\param _g green value
	\param _b blue value
	\param _a alpha value
*/
drgColor::drgColor( unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a )
{
	Set(_r, _g, _b, _a);
}

//!Destructor
drgColor::~drgColor()
{
}

//!Function that sets the color, accepting individual r,g,b, and a values.
/*!
	\param _r red valule
	\param _g green value
	\param _b blue value
	\param _a alpha value
*/
void drgColor::Set( unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a )
{
	r=_r;
	g=_g;
	b=_b;
	a=_a;
}

void drgColor::Set( drgColor* _color )
{
	r=_color->r;
	g=_color->g;
	b=_color->b;
	a=_color->a;
}

//!Set constructor
/*!
	Sets current color to the provided color
	\param _color color to use
*/
void drgColor::Set( unsigned int _color )
{
	Set( (_color&0x000000FF), ((_color&0x0000FF00)>>8), ((_color&0x00FF0000)>>16), ((_color&0xFF000000)>>24) );
}


//!Function that sets the color to a random value
/*!
	\param _min minimum value to use
	\param _max maximum value to use
*/
void drgColor::SetRandom( unsigned char _min, unsigned char _max )
{
	int diff=_max-_min;
	if(diff<0)
		Set(0,0,0);
	else
		Set( (drgRand::Rand()%diff)+_min, (drgRand::Rand()%diff)+_min, (drgRand::Rand()%diff)+_min );
}

void drgColor::Set565( unsigned short _color )
{
	Set( ((_color>>11)&0x001F)*8, ((_color>>5)&0x003F)*4, (_color&0x001F)*8, 255 );
}

unsigned short drgColor::Get4444( unsigned short* outcolor )
{
	unsigned short	resultColor;
	
	resultColor =	((unsigned char)a/16<<12) |
					((unsigned char)b/16<<8)  |
					((unsigned char)g/16<<4)  |
					(unsigned char)r/16;

	if( outcolor)
		(*outcolor) = resultColor;
		
	return resultColor;					
}

unsigned int drgColor::Get8888(unsigned int * outcolor)
{
	unsigned int	resultColor;
	resultColor= 	((unsigned char)a<<24) |
					((unsigned char)r)     |
					((unsigned char)g<<8)  |
					((unsigned char)b<<16);
	if( outcolor)
		(*outcolor) = resultColor;
		
	return resultColor;
}

unsigned int drgColor::GetABGR(unsigned int * outcolor)
{
	unsigned int	resultColor;
	resultColor= 	((unsigned char)a<<24) |
					((unsigned char)b<<16) |
					((unsigned char)g<<8)  |
					(unsigned char)r;
	if( outcolor)
		(*outcolor) = resultColor;
		
	return resultColor;
}

unsigned int drgColor::GetRGB(unsigned int * outcolor)
{
	unsigned int	resultColor;
	resultColor= 	((unsigned char)r<<16) |
					((unsigned char)g<<8)  |
					(unsigned char)b;
	if( outcolor)
		(*outcolor) = resultColor;
		
	return resultColor;
}

void drgColor::Lerp(drgColor *color0, drgColor *color1, float ammount)
{
	if (!color0 || !color1)
		return;

	r=(char)((float)color0->r*(1-ammount)+(float)color1->r*(ammount));
	g=(char)((float)color0->g*(1-ammount)+(float)color1->g*(ammount));
	b=(char)((float)color0->b*(1-ammount)+(float)color1->b*(ammount));
	a=(char)((float)color0->a*(1-ammount)+(float)color1->a*(ammount));
}
