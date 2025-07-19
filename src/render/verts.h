#ifndef _DRG_VERTS_H_
#define _DRG_VERTS_H_

#include "render/context.h"
#include "system/extern_inc.h"

// structures for our custom vertex type.
struct DRG_VERTEX_COLORED
{
    drgVec3		position; //!< The position of the vertex
    unsigned int	color;    //!< The color of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_COLORED
{
	drgVec3		position;	//!< The position of the vertex
	drgVec3		normal;		//!< The normal of the vertex
    unsigned int	color;		//!< The color of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_TEXTURED_COLORED
{
	drgVec3		position;	//!< The position of the vertex
	drgVec2		uv;			//!< The texcoord of the vertex
    unsigned int	color;		//!< The color of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED
{
    drgVec3		position;		//!< The position of the vertex
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_COLORED
{
	drgVec3		position;		//!< The position of the vertex
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates
    unsigned int	color;			//!< The color of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED
{
	drgVec3		position;		//!< The position of the vertex
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates
    unsigned int	color;			//!< The color of the vertex
    unsigned int	color2;			//!< The color of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT
{
	drgVec3		position;		//!< The position of the vertex
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates
    unsigned int	color;			//!< The color of the vertex
	drgVec3		tangent;		//!< The tangent of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT
{
	drgVec3		position;		//!< The position of the vertex
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates
    unsigned int	color;			//!< The color of the vertex
    unsigned int	color2;			//!< The color of the vertex
	drgVec3		tangent;		//!< The tangent of the vertex

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH
{
    drgVec3		position;		//!< The position of the vertex
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates
    unsigned int	color;			//!< The color of the vertex
	drgVec3		tangent;		//!< The tangent of the vertex
	unsigned char	posOffset[4];	//!< The offset position for the morph
	unsigned char	normOffset[4];	//!< The offset normal for the morph

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED
{
    drgVec3		position;   //!< The position
    drgVec3		normal;		//!< The normal of the vertex
    drgVec2		uv;         //!< The texture coordinates
	unsigned int	color;      //!< The color
	drgVec3		tangent;	//!< The tangent of the vertex
	drgVec4		weight1;	//!< vert weights
	drgVec4		weight2;	//!< vert weights

	static const unsigned int attribs;
};

struct DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH
{
    drgVec3		position;		//!< The position
    drgVec3		normal;			//!< The normal of the vertex
	drgVec2		uv;				//!< The texture coordinates
    unsigned int	color;			//!< The color
	drgVec4		weight1;		//!< vert weights
	drgVec4		weight2;		//!< vert weights
	unsigned char	posOffset[4];	//!< The offset position for the morph
	unsigned char	normOffset[4];	//!< The offset normal for the morph

	static const unsigned int attribs;
};

//
// Vertex Attributes
//

#define DRG_VERTEX_ATTRIB_WEIGHT0		0x00000001
#define	DRG_VERTEX_ATTRIB_WEIGHT1		0x00000002
#define DRG_VERTEX_ATTRIB_POSITION		0x00000004
#define	DRG_VERTEX_ATTRIB_NORMAL		0x00000008
#define DRG_VERTEX_ATTRIB_TEX0			0x00000010
#define DRG_VERTEX_ATTRIB_TEX1			0x00000020
#define	DRG_VERTEX_ATTRIB_TANGENT		0x00000040
#define DRG_VERTEX_ATTRIB_BINORMAL		0x00000080
#define	DRG_VERTEX_ATTRIB_COLOR0		0x00000100
#define DRG_VERTEX_ATTRIB_COLOR1		0x00000200
#define	DRG_VERTEX_ATTRIB_POS_MORPH		0x00000400
#define DRG_VERTEX_ATTRIB_NORMAL_MORPH	0x00000800

#if defined _IPHONE
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>
	#import <OpenGLES/ES2/gl.h>
	#import <OpenGLES/ES2/glext.h>
#elif defined _ANDROID
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#elif defined _WINDOWS
#if defined _OPEN_GL
	#include <gl/GL.h>
	#include <gl/GLU.h>
#endif
#elif defined _MAC
    #import <OpenGL/gl.h>
    #import <OpenGL/glext.h>
#endif

	// tile file defines
#define GEOM_FLAG_PHYSICS			(1<<0)
#define GEOM_FLAG_PHYSICS_ONLY		(1<<1)
#define GEOM_FLAG_WALK				(1<<2)
#define GEOM_FLAG_SHADOW_NO_CAST	(1<<3)
#define GEOM_FLAG_SHADOW_ONLY		(1<<4)
#define GEOM_FLAG_SHADOW_NO_RECIVE	(1<<5)
#define GEOM_FLAG_MOBILE			(1<<6)
#define GEOM_FLAG_DOOR				(1<<7)
#define GEOM_FLAG_WATER				(1<<8)
#define GEOM_FLAG_PROJECTION_NONE	(1<<9)
#define GEOM_FLAG_CUTOUT			(1<<10)
#define GEOM_FLAG_ALPHA				(1<<11)
#define GEOM_FLAG_LOD_FADE			(1<<12)
#define GEOM_FLAG_SELFILLUM			(1<<13)
#define GEOM_FLAG_SWAY				(1<<14)
#define GEOM_FLAG_SECOND_UV			(1<<15)
#define GEOM_FLAG_PROP				(1<<16)
#define GEOM_FLAG_ANIMATE			(1<<17)
#define GEOM_FLAG_DECAL				(1<<18)
#define GEOM_FLAG_BILLBOARD			(1<<19)
#define GEOM_FLAG_REFLECT			(1<<20)
#define GEOM_FLAG_VALPHA			(1<<21)
#define GEOM_FLAG_TWOSIDE			(1<<22)
#define GEOM_FLAG_DISTANT			(1<<23)
#define GEOM_FLAG_HEIGHT			(1<<24)
#define GEOM_FLAG_SAT				(1<<25)
#define GEOM_FLAG_DYNAMIC_TEX		(1<<29)
#define GEOM_FLAG_DEFORM			(1<<30)
#define GEOM_FLAG_POST_PROCESS		(1<<31)


#define POLYSETFLAGS_SIMPLEORIENTATION	(1<<0)
#define POLYSETFLAGS_NOTFUNKYTRANS		(1<<1)
#define POLYSETFLAGS_COMPLEXFLIP		(1<<2)
#define POLYSETFLAGS_SHADOW_NO_CAST		(1<<3)
#define POLYSETFLAGS_SHADOW_NO_RECIVE	(1<<5)
#define POLYSETFLAGS_MOBILE				(1<<6)
#define POLYSETFLAGS_WATER				(1<<7)
#define POLYSETFLAGS_FLASH				(1<<8)
#define POLYSETFLAGS_TRANSLUCENT		(1<<9)
#define POLYSETFLAGS_CUTOUT				(1<<10)
#define POLYSETFLAGS_OBJECT				(1<<11)
#define POLYSETFLAGS_ALTERNATEWATER		(1<<12)
#define POLYSETFLAGS_SELFILLUM			(1<<13)
#define POLYSETFLAGS_SELFILLUM_OLD		(1<<19)
#define POLYSETFLAGS_SWAY				(1<<14)
#define POLYSETFLAGS_SECOND_UV			(1<<15)
#define POLYSETFLAGS_SIMPLE_VERT		(1<<16)
#define POLYSETFLAGS_NOSOLID			(1<<17)
#define POLYSETFLAGS_DECAL				(1<<18)
#define POLYSETFLAGS_BILLBOARD			(1<<19)
#define POLYSETFLAGS_REFLECT			(1<<20)
#define POLYSETFLAGS_ANIMATE			(1<<21)
#define POLYSETFLAGS_FADEDIN			(1<<22)
#define POLYSETFLAGS_LOD_FADE			(1<<24)
#define POLYSETFLAGS_VTRANSLUCENT		(1<<25)
#define POLYSETFLAGS_TWOSIDE			(1<<26)
#define POLYSETFLAGS_DISTANT			(1<<27)
#define POLYSETFLAGS_HEIGHT				(1<<28)
#define POLYSETFLAGS_DYNAMIC_TEX		(1<<29)
#define POLYSETFLAGS_DEFORM				(1<<30)
#define POLYSETFLAGS_POST_PROCESS		(1<<31)

enum DRG_PRIMITIVE_TYPE
{
#if (defined(_WINMO))
	PRIM_POINTLIST				= D3DMPT_POINTLIST,
	PRIM_LINELIST				= D3DMPT_LINELIST,
	PRIM_LINESTRIP				= D3DMPT_LINESTRIP,
	PRIM_TRIANGLELIST			= D3DMPT_TRIANGLELIST,
	PRIM_TRIANGLESTRIP			= D3DMPT_TRIANGLESTRIP,
	PRIM_TRIANGLEFAN			= D3DMPT_TRIANGLEFAN,
#elif (defined(_XBOX360))
	PRIM_POINTLIST				= D3DPT_POINTLIST,
	PRIM_LINELIST				= D3DPT_LINELIST,
	PRIM_LINESTRIP				= D3DPT_LINESTRIP,
	PRIM_TRIANGLELIST			= D3DPT_TRIANGLELIST,
	PRIM_TRIANGLESTRIP			= D3DPT_TRIANGLESTRIP,
	PRIM_TRIANGLEFAN			= D3DPT_TRIANGLEFAN,
#elif (defined(_WINDOWS))
#if USE_OPEN_GL
	PRIM_POINTLIST				= GL_POINTS,
	PRIM_LINELIST				= GL_LINES,
	PRIM_LINESTRIP				= GL_LINE_STRIP,
	PRIM_TRIANGLELIST			= GL_TRIANGLES,
	PRIM_TRIANGLESTRIP			= GL_TRIANGLE_STRIP,
	PRIM_TRIANGLEFAN			= GL_TRIANGLE_FAN,
#elif USE_WEBGPU
	PRIM_POINTLIST				= WGPUPrimitiveTopology_PointList,
	PRIM_LINELIST				= WGPUPrimitiveTopology_LineList,
	PRIM_LINESTRIP				= WGPUPrimitiveTopology_LineStrip,
	PRIM_TRIANGLELIST			= WGPUPrimitiveTopology_TriangleList,
	PRIM_TRIANGLESTRIP			= WGPUPrimitiveTopology_TriangleStrip,
	PRIM_TRIANGLEFAN			= 0xBAD,
#endif
#elif defined _PSP
	PRIM_POINTLIST				= PSPGE_PRIM_POINTS,
	PRIM_LINELIST				= PSPGE_PRIM_LINES,
	PRIM_LINESTRIP				= PSPGE_PRIM_LINE_STRIP,
	PRIM_TRIANGLELIST			= PSPGE_PRIM_TRIANGLES,
	PRIM_TRIANGLESTRIP			= PSPGE_PRIM_TRIANGLE_STRIP,
	PRIM_TRIANGLEFAN			= PSPGE_PRIM_TRIANGLE_FAN,
#elif defined _PS3
	PRIM_POINTLIST				= CELL_GCM_PRIMITIVE_POINTS,
	PRIM_LINELIST				= CELL_GCM_PRIMITIVE_LINES,
	PRIM_LINESTRIP				= CELL_GCM_PRIMITIVE_LINE_STRIP,
	PRIM_TRIANGLELIST			= CELL_GCM_PRIMITIVE_TRIANGLES,
	PRIM_TRIANGLESTRIP			= CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,
	PRIM_TRIANGLEFAN			= CELL_GCM_PRIMITIVE_TRIANGLE_FAN,
#elif (defined _IPHONE) || (defined _MAC) || defined(_ANDROID)
	PRIM_POINTLIST				= GL_POINTS,
	PRIM_LINELIST				= GL_LINES,
	PRIM_LINESTRIP				= GL_LINE_STRIP,
	PRIM_TRIANGLELIST			= GL_TRIANGLES,
	PRIM_TRIANGLESTRIP			= GL_TRIANGLE_STRIP,
	PRIM_TRIANGLEFAN			= GL_TRIANGLE_FAN,
#else
	PRIM_POINTLIST				= 0,
	PRIM_LINELIST				= 1,
	PRIM_LINESTRIP				= 2,
	PRIM_TRIANGLELIST			= 3,
	PRIM_TRIANGLESTRIP			= 4,
	PRIM_TRIANGLEFAN			= 5,
#endif
	PRIM_NUM_TYPES
};

class drgVertHelper
{
	public:
		static int				GetVertexSize( unsigned int vertexAttribs );
		static drgVec3*		GetVertexPosition( unsigned int vertexAttribs, void* vertices, int index );
		static drgVec3*		GetVertexNormal( unsigned int vertexAttribs, void* vertices, int index );
		static drgVec3*		GetVertexTangent( unsigned int vertexAttribs, void* vertices, int index );
		static drgVec2*		GetVertexTexCoord0( unsigned int vertexAttribs, void* vertices, int index );
		static drgVec2*		GetVertexTexCoord1( unsigned int vertexAttribs, void* vertices, int index );
		static unsigned int*	GetVertexColor0( unsigned int vertexAttribs, void* vertices, int index );
		static unsigned int*	GetVertexColor1( unsigned int vertexAttribs, void* vertices, int index );
		static drgVec4*		GetVertexWeight0( unsigned int vertexAttribs, void* vertices, int index );
		static drgVec4*		GetVertexWeight1( unsigned int vertexAttribs, void* vertices, int index );
		static unsigned char*	GetVertexPosOffset( unsigned int vertexAttribs, void* vertices, int index );
		static unsigned char*	GetVertexNormalOffset( unsigned int vertexAttribs, void* vertices, int index );
};

#endif