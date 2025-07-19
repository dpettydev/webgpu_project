#ifndef __DRG_MATH_RAND_H__
#define __DRG_MATH_RAND_H__

#include "util/math_def.h"

//!Class that can randomly generate shorts, floats, and 2 and 3 dimensional vectors
class drgRand
{
public:
	static void SetSeedByTime();
	static inline void SetSeed( unsigned int seed );
	static inline unsigned int GetState();
	static inline unsigned short Rand();
	static inline unsigned short Rand(unsigned short RangeLow, unsigned short RangeHigh);
	static inline unsigned short Rand(unsigned int RandState);
	static inline float Randf();
	static inline float Randf(float RangeLow, float RangeHigh);
	static inline float Randf(unsigned int RandState);
	static inline void Randv(drgVec3 *v);
	static inline void Randv(drgVec3 *v, int angle);
	static inline drgVec3 Randv();
	static inline drgVec2 Randv2();

private:
	static unsigned int m_RandState;
};



////////////////////////////////////////////////////////////////
// drgRand stuff
////////////////////////////////////////////////////////////////

//!Function that sets the seed for the random number generator
/*!
	\param seed new seed for the random number generator
*/
void drgRand::SetSeed( unsigned int seed )
{
	m_RandState=seed;
}

//!Function that returns the current random state.
/*!
	\returns the current state.
*/
unsigned int drgRand::GetState()
{
	return m_RandState;
}

//TODO:accurate?
//!Function that generates a random short
unsigned short drgRand::Rand()
{
	m_RandState=m_RandState*1664525+1013904223;
	return (unsigned short)(m_RandState>>16);
}

//!Function that generates a random unsigned short within a given range
unsigned short drgRand::Rand(unsigned short RangeLow, unsigned short RangeHigh)
{
	m_RandState=m_RandState*1664525+1013904223;
	//float retval = (float) (m_RandState>>16);
	//return (unsigned short) fmod(retval, ((float)RangeHigh-(float)RangeLow)+0.000001f) + RangeLow; 
	return (unsigned short) (((float) Randf() * (float)((RangeHigh-RangeLow)+1)) + (float)RangeLow);
	//return (unsigned short)(fmod(retval, ((float)RangeHigh-(float)RangeLow)+0.000001f) + RangeLow); 
}

//!Function that generates a random short given a new state
unsigned short drgRand::Rand(unsigned int RandState)
{
	RandState=RandState*1664525+1013904223;
	RandState=RandState*1664525+1013904223;
	RandState=RandState*1664525+1013904223;
	return (unsigned short)(RandState>>16);
}

//!Function that generates a random float within a given range
float drgRand::Randf(float RangeLow, float RangeHigh)
{
	return (float) (((float) Randf() * (float)(RangeHigh-RangeLow)) + (float)RangeLow);
	//return (((RangeHigh-RangeLow)+0.000001f) * (Rand()*(1.f/65536.f))) + RangeLow;
}

//!Function that generates a random float
float drgRand::Randf()
{
	return Rand()*(1.f/65536.f);
}

//!Function that generates a random float given a new state
float drgRand::Randf(unsigned int RandState)
{
	return Rand(RandState)*(1.f/65536.f);
}


//!Function that generates a random drgVec3
void drgRand::Randv(drgVec3 *v)
{
	do
	{
		v->x=Randf()*2.0f-1.0f;
		v->y=Randf()*2.0f-1.0f;
		v->z=Randf()*2.0f-1.0f;
	} while (v->x*v->x+v->y*v->y+v->z*v->z>1);
}
//!Function that generates a random drgVec3
/*!
	Stores the newly generated drgVec3 in the provided v.
	\param *v initial vector and storage of new vector
	\param angle the max angle from initial vector to generate the new one
*/
void drgRand::Randv(drgVec3 *v, int angle)
{
	int rTheta = drgRand::Rand(0, (unsigned short)angle);
	int rFee = drgRand::Rand(0, (unsigned short)angle);
	rTheta -= (angle>>1);
	rFee -= (angle>>1);
	float sT = isin(rTheta);
	float sF = isin(rFee);
	float cT = icos(rTheta);
	float cF = icos(rFee);
	v->x = (v->x + (cT * sF)) * 0.5f;
	v->y = (v->y + (sT * sF)) * 0.5f;
	v->z = (v->z + cF) * 0.5f;
}

//!Function that generates a random drgVec3
drgVec3 drgRand::Randv()
{
	drgVec3 v;
	do
	{
		v.x=Randf()*2.0f-1.0f;
		v.y=Randf()*2.0f-1.0f;
		v.z=Randf()*2.0f-1.0f;
	} while (v.x*v.x+v.y*v.y+v.z*v.z>1);
	return v;
}

//!Function that generates a random drgVec2
drgVec2 drgRand::Randv2()
{
	drgVec2 v;
	do
	{
		v.x=Randf()*2.0f-1.0f;
		v.y=Randf()*2.0f-1.0f;
	} while (v.x*v.x+v.y*v.y>1);
	return v;
}

#endif // __DRG_MATH_RAND_H__



/* 
// Javascript version
const double norm32 = 2.3283064365386963e-10;   // 2^-32

static uint32_t x = 123456789;
static uint32_t y = 362436069;
static uint32_t z = 21288629;
static uint32_t w = 14921776;
static uint32_t c = 0;

static inline double kiss07(void) {
  uint32_t t;

  x += 545925293;

  y ^= y << 13;
  y ^= y >> 17;
  y ^= y << 5;

  t = z + w + c;
  z = w;
  c = t >> 31;
  w = t & 0x7fffffff;

  return (x + y + w) * norm32;  // 2^-32;
}

int main(int argc, unsigned char *argv[]) {
  double buffer[256];
  int i;

  for (i = 1; i < argc; i++) {
	x ^= (uint32_t) (mash(argv[i]) * 0x100000000ULL);   // 2^32
	y ^= (uint32_t) (mash(argv[i]) * 0x100000000ULL);   // 2^32
	z ^= (uint32_t) (mash(argv[i]) * 0x100000000ULL);   // 2^32
	w ^= (uint32_t) (mash(argv[i]) * 0x100000000ULL);   // 2^32
  }
  if (y == 0) {
	y = 1;
  }
  c ^= z >> 31;
  z &= 0x7fffffff;
  if ((z % 7559) == 0) {
	z++;
  }
  w &= 0x7fffffff;
  if ((w % 7559) == 0) {
	w++;
  }

  while (1) {
	for (i = 0; i < 256; i++) {
	  buffer[i] = kiss07();
	}
	fwrite(buffer, sizeof buffer, 1, stdout);
  }
}
*/