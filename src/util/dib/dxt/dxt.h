#pragma once

#include "system/global.h"
#include "util/math_def.h"


// dxt flags
#define DRG_DDSD_CAPS			0x00000001
#define DRG_DDSD_HEIGHT			0x00000002
#define DRG_DDSD_WIDTH			0x00000004
#define DRG_DDSD_PITCH			0x00000008
#define DRG_DDSD_PIXELFORMAT	0x00001000
#define DRG_DDSD_MIPMAPCOUNT	0x00020000
#define DRG_DDSD_LINEARSIZE		0x00080000
#define DRG_DDSD_DEPTH			0x00800000
#define DRG_DDPF_ALPHAPIXELS	0x00000001
#define DRG_DDPF_LUMINANCE		0x00020000
#define DRG_DDPF_FOURCC			0x00000004
#define DRG_DDPF_RGB			0x00000040
#define DRG_DDSCAPS_COMPLEX		0x00000008
#define DRG_DDSCAPS_TEXTURE		0x00001000
#define DRG_DDSCAPS_MIPMAP		0x00400000
#define DRG_DDSCAPS_LUMINANCE	0x00000002


//! Typedef a quantity that is a single unsigned byte.
typedef unsigned char u8;
enum
{
	kDxt1 = ( 1 << 0 ), 
	kDxt3 = ( 1 << 1 ), 
	kDxt5 = ( 1 << 2 ), 
	
	//! Use a very slow but very high quality colour compressor.
	kColourIterativeClusterFit = ( 1 << 8 ),	
	//! Use a slow but high quality colour compressor (the default).
	kColourClusterFit = ( 1 << 3 ),	
	//! Use a fast but low quality colour compressor.
	kColourRangeFit	= ( 1 << 4 ),
	//! Use a perceptual metric for colour error (the default).
	kColourMetricPerceptual = ( 1 << 5 ),
	//! Use a uniform metric for colour error.
	kColourMetricUniform = ( 1 << 6 ),
	//! Weight the colour by alpha during cluster fit (disabled by default).
	kWeightColourByAlpha = ( 1 << 7 )
};

class dxtVec3
{
public:
	typedef dxtVec3 const& Arg;

	dxtVec3()
	{
	}

	explicit dxtVec3( float s )
	{
		m_x = s;
		m_y = s;
		m_z = s;
	}

	dxtVec3( float x, float y, float z )
	{
		m_x = x;
		m_y = y;
		m_z = z;
	}
	
	float X() const { return m_x; }
	float Y() const { return m_y; }
	float Z() const { return m_z; }
	
	dxtVec3 operator-() const
	{
		return dxtVec3( -m_x, -m_y, -m_z );
	}
	
	dxtVec3& operator+=( Arg v )
	{
		m_x += v.m_x;
		m_y += v.m_y;
		m_z += v.m_z;
		return *this;
	}
	
	dxtVec3& operator-=( Arg v )
	{
		m_x -= v.m_x;
		m_y -= v.m_y;
		m_z -= v.m_z;
		return *this;
	}
	
	dxtVec3& operator*=( Arg v )
	{
		m_x *= v.m_x;
		m_y *= v.m_y;
		m_z *= v.m_z;
		return *this;
	}
	
	dxtVec3& operator*=( float s )
	{
		m_x *= s;
		m_y *= s;
		m_z *= s;
		return *this;
	}
	
	dxtVec3& operator/=( Arg v )
	{
		m_x /= v.m_x;
		m_y /= v.m_y;
		m_z /= v.m_z;
		return *this;
	}
	
	dxtVec3& operator/=( float s )
	{
		float t = 1.0f/s;
		m_x *= t;
		m_y *= t;
		m_z *= t;
		return *this;
	}
	
	friend dxtVec3 operator+( Arg left, Arg right )
	{
		dxtVec3 copy( left );
		return copy += right;
	}
	
	friend dxtVec3 operator-( Arg left, Arg right )
	{
		dxtVec3 copy( left );
		return copy -= right;
	}
	
	friend dxtVec3 operator*( Arg left, Arg right )
	{
		dxtVec3 copy( left );
		return copy *= right;
	}
	
	friend dxtVec3 operator*( Arg left, float right )
	{
		dxtVec3 copy( left );
		return copy *= right;
	}
	
	friend dxtVec3 operator*( float left, Arg right )
	{
		dxtVec3 copy( right );
		return copy *= left;
	}
	
	friend dxtVec3 operator/( Arg left, Arg right )
	{
		dxtVec3 copy( left );
		return copy /= right;
	}
	
	friend dxtVec3 operator/( Arg left, float right )
	{
		dxtVec3 copy( left );
		return copy /= right;
	}
	
	friend float Dot( Arg left, Arg right )
	{
		return left.m_x*right.m_x + left.m_y*right.m_y + left.m_z*right.m_z;
	}
	
	friend dxtVec3 Min( Arg left, Arg right )
	{
		return dxtVec3(
			DRG_MIN( left.m_x, right.m_x ), 
			DRG_MIN( left.m_y, right.m_y ), 
			DRG_MIN( left.m_z, right.m_z )
		);
	}

	friend dxtVec3 Max( Arg left, Arg right )
	{
		return dxtVec3(
			DRG_MAX( left.m_x, right.m_x ), 
			DRG_MAX( left.m_y, right.m_y ), 
			DRG_MAX( left.m_z, right.m_z )
		);
	}

	friend dxtVec3 Truncate( Arg v )
	{
		return dxtVec3(
			v.m_x > 0.0f ? DRG_FLOOR( v.m_x ) : DRG_CEIL( v.m_x ), 
			v.m_y > 0.0f ? DRG_FLOOR( v.m_y ) : DRG_CEIL( v.m_y ), 
			v.m_z > 0.0f ? DRG_FLOOR( v.m_z ) : DRG_CEIL( v.m_z )
		);
	}

private:
	float m_x;
	float m_y;
	float m_z;
};

#define VEC4_CONST( X ) dxtVec4( X )

class dxtVec4
{
public:
	typedef dxtVec4 const& Arg;

	dxtVec4() {}
		
	explicit dxtVec4( float s )
	  : m_x( s ),
		m_y( s ),
		m_z( s ),
		m_w( s )
	{
	}
	
	dxtVec4( float x, float y, float z, float w )
	  : m_x( x ),
		m_y( y ),
		m_z( z ),
		m_w( w )
	{
	}
	
	dxtVec3 GetVec3() const
	{
		return dxtVec3( m_x, m_y, m_z );
	}
	
	dxtVec4 SplatX() const { return dxtVec4( m_x ); }
	dxtVec4 SplatY() const { return dxtVec4( m_y ); }
	dxtVec4 SplatZ() const { return dxtVec4( m_z ); }
	dxtVec4 SplatW() const { return dxtVec4( m_w ); }

	dxtVec4& operator+=( Arg v )
	{
		m_x += v.m_x;
		m_y += v.m_y;
		m_z += v.m_z;
		m_w += v.m_w;
		return *this;
	}
	
	dxtVec4& operator-=( Arg v )
	{
		m_x -= v.m_x;
		m_y -= v.m_y;
		m_z -= v.m_z;
		m_w -= v.m_w;
		return *this;
	}
	
	dxtVec4& operator*=( Arg v )
	{
		m_x *= v.m_x;
		m_y *= v.m_y;
		m_z *= v.m_z;
		m_w *= v.m_w;
		return *this;
	}
	
	friend dxtVec4 operator+(dxtVec4::Arg left, dxtVec4::Arg right  )
	{
		dxtVec4 copy( left );
		return copy += right;
	}
	
	friend dxtVec4 operator-(dxtVec4::Arg left, dxtVec4::Arg right  )
	{
		dxtVec4 copy( left );
		return copy -= right;
	}
	
	friend dxtVec4 operator*(dxtVec4::Arg left, dxtVec4::Arg right  )
	{
		dxtVec4 copy( left );
		return copy *= right;
	}
	
	//! Returns a*b + c
	friend dxtVec4 MultiplyAdd(dxtVec4::Arg a, dxtVec4::Arg b, dxtVec4::Arg c )
	{
		return a*b + c;
	}
	
	//! Returns -( a*b - c )
	friend dxtVec4 NegativeMultiplySubtract(dxtVec4::Arg a, dxtVec4::Arg b, dxtVec4::Arg c )
	{
		return c - a*b;
	}
	
	friend dxtVec4 Reciprocal(dxtVec4::Arg v )
	{
		return dxtVec4(
			1.0f/v.m_x, 
			1.0f/v.m_y, 
			1.0f/v.m_z, 
			1.0f/v.m_w 
		);
	}
	
	friend dxtVec4 Min(dxtVec4::Arg left, dxtVec4::Arg right )
	{
		return dxtVec4(
			DRG_MIN( left.m_x, right.m_x ), 
			DRG_MIN( left.m_y, right.m_y ), 
			DRG_MIN( left.m_z, right.m_z ), 
			DRG_MIN( left.m_w, right.m_w ) 
		);
	}
	
	friend dxtVec4 Max(dxtVec4::Arg left, dxtVec4::Arg right )
	{
		return dxtVec4(
			DRG_MAX( left.m_x, right.m_x ), 
			DRG_MAX( left.m_y, right.m_y ), 
			DRG_MAX( left.m_z, right.m_z ), 
			DRG_MAX( left.m_w, right.m_w ) 
		);
	}
	
	friend dxtVec4 Truncate(dxtVec4::Arg v )
	{
		return dxtVec4(
			v.m_x > 0.0f ? DRG_FLOOR( v.m_x ) : DRG_CEIL( v.m_x ), 
			v.m_y > 0.0f ? DRG_FLOOR( v.m_y ) : DRG_CEIL( v.m_y ), 
			v.m_z > 0.0f ? DRG_FLOOR( v.m_z ) : DRG_CEIL( v.m_z ),
			v.m_w > 0.0f ? DRG_FLOOR( v.m_w ) : DRG_CEIL( v.m_w )
		);
	}
	
	friend bool CompareAnyLessThan(dxtVec4::Arg left, dxtVec4::Arg right )
	{
		return left.m_x < right.m_x
			|| left.m_y < right.m_y
			|| left.m_z < right.m_z
			|| left.m_w < right.m_w;
	}
	
private:
	float m_x;
	float m_y;
	float m_z;
	float m_w;
};

inline float LengthSquared(dxtVec3::Arg v )
{
	return Dot( v, v );
}

class Sym3x3
{
public:
	Sym3x3()
	{
	}

	Sym3x3( float s )
	{
		for( int i = 0; i < 6; ++i )
			m_x[i] = s;
	}

	float operator[]( int index ) const
	{
		return m_x[index];
	}

	float& operator[]( int index )
	{
		return m_x[index];
	}

private:
	float m_x[6];
};

Sym3x3 ComputeWeightedCovariance( int n, dxtVec3 const* points, float const* weights );
dxtVec3 ComputePrincipleComponent( Sym3x3 const& matrix );

void CompressAlphaDxt3( u8 const* rgba, int mask, void* block );
void CompressAlphaDxt5( u8 const* rgba, int mask, void* block );

void DecompressAlphaDxt3( u8* rgba, void const* block );
void DecompressAlphaDxt5( u8* rgba, void const* block );

void WriteColourBlock3(dxtVec3::Arg start, dxtVec3::Arg end, u8 const* indices, void* block );
void WriteColourBlock4(dxtVec3::Arg start, dxtVec3::Arg end, u8 const* indices, void* block );

void DecompressColour( u8* rgba, void const* block, bool isDxt1 );

class ColourSet
{
public:
	ColourSet( u8 const* rgba, int mask, int flags );

	int GetCount() const { return m_count; }
	dxtVec3 const* GetPoints() const { return m_points; }
	float const* GetWeights() const { return m_weights; }
	bool IsTransparent() const { return m_transparent; }

	void RemapIndices( u8 const* source, u8* target ) const;

private:
	int m_count;
	dxtVec3 m_points[16];
	float m_weights[16];
	int m_remap[16];
	bool m_transparent;
};

class ColourSet;
struct SingleColourLookup;

class ColourFit
{
public:
	ColourFit( ColourSet const* colours, int flags );

	void Compress( void* block );

protected:
	virtual void Compress3( void* block ) {};
	virtual void Compress4( void* block ) {};

	ColourSet const* m_colours;
	int m_flags;
};

class SingleColourFit : public ColourFit
{
public:
	SingleColourFit( ColourSet const* colours, int flags );
	
private:
	virtual void Compress3( void* block );
	virtual void Compress4( void* block );
	
	void ComputeEndPoints( SingleColourLookup const* const* lookups );
	
	u8 m_colour[3];
	dxtVec3 m_start;
	dxtVec3 m_end;
	u8 m_index;
	int m_error;
	int m_besterror;
};

class RangeFit : public ColourFit
{
public:
	RangeFit( ColourSet const* colours, int flags );
	
private:
	virtual void Compress3( void* block );
	virtual void Compress4( void* block );
	
	dxtVec3 m_metric;
	dxtVec3 m_start;
	dxtVec3 m_end;
	float m_besterror;
};

class ClusterFit : public ColourFit
{
public:
	ClusterFit( ColourSet const* colours, int flags );
	
private:
	bool ConstructOrdering(dxtVec3 const& axis, int iteration );

	virtual void Compress3( void* block );
	virtual void Compress4( void* block );

	enum { kMaxIterations = 8 };

	int m_iterationCount;
	dxtVec3 m_principle;
	u8 m_order[16*kMaxIterations];
	dxtVec4 m_points_weights[16];
	dxtVec4 m_xsum_wsum;
	dxtVec4 m_metric;
	dxtVec4 m_besterror;
};

void Compress( u8 const* rgba, void* block, int flags );
void CompressMasked( u8 const* rgba, int mask, void* block, int flags );
void Decompress( u8* rgba, void const* block, int flags );
int GetStorageRequirements( int width, int height, int flags );
void CompressImage( u8 const* rgba, int width, int height, void* blocks, int flags );
void DecompressImage( u8* rgba, int width, int height, void const* blocks, int flags );
