

#include "util/math_def.h"
#include "util/rand.h"
#include "system/global.h"

/////////////////////////////////
//  drgRand
/////////////////////////////////

unsigned int drgRand::m_RandState = 0;
void drgRand::SetSeedByTime()
{
	SetSeed(drgGetTime());
}

void drgVec2::Print(const char *name) const
{
	if (name != NULL)
		printf("Vector3 %s: {%f, %f}\r\n", name, x, y);
	else
		printf("Vector3: {%f, %f}\r\n", x, y);
}

void drgVec3::Print(const char *name) const
{
	if (name != NULL)
		printf("Vector3 %s: {%f, %f, %f}\r\n", name, x, y, z);
	else
		printf("Vector3: {%f, %f, %f}\r\n", x, y, z);
}

void drgMat44::Print(const char *name) const
{
	if (name != NULL)
		drgPrintOut("Matrix %s:\n", name);
	else
		drgPrintOut("Matrix:\n");
	for (int i = 0; i < 4; ++i)
		drgPrintOut("[%f, %f, %f, %f]\n", m[i][0], m[i][1], m[i][2], m[i][3]);
}



/////////////////////////////////
//  drgBitArray
/////////////////////////////////

drgBitArray::drgBitArray(unsigned int size)
{
	m_Size = (size / 32) + 1;
	m_Array = (unsigned int*)drgMemAlloc(sizeof(unsigned int) * m_Size);
}

drgBitArray::~drgBitArray()
{
	drgMemFree(m_Array);
}

void drgBitArray::SetBit(unsigned int bit)
{
	assert((bit / 32U) < m_Size);
	m_Array[bit / 32U] |= 1U << (bit % 32U);
}

void drgBitArray::ClearBit(unsigned int bit)
{
	assert((bit / 32U) < m_Size);
	m_Array[bit / 32U] &= ~(1U << (bit % 32U));
}

bool drgBitArray::TestBit(unsigned int bit)
{
	return ((m_Array[bit / 32U] & (1U << (bit % 32U))) != 0);
}