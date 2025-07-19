#include "verts.h"

// Static Initialization
const unsigned int DRG_VERTEX_COLORED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_COLOR0;
const unsigned int DRG_VERTEX_NORMAL_COLORED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_TEX0;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0 | DRG_VERTEX_ATTRIB_TEX0;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0 | DRG_VERTEX_ATTRIB_TEX0 | DRG_VERTEX_ATTRIB_WEIGHT0 | DRG_VERTEX_ATTRIB_WEIGHT1;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0 | DRG_VERTEX_ATTRIB_TEX0 | DRG_VERTEX_ATTRIB_TANGENT;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0 | DRG_VERTEX_ATTRIB_TEX0 | DRG_VERTEX_ATTRIB_WEIGHT0 | DRG_VERTEX_ATTRIB_WEIGHT1 | DRG_VERTEX_ATTRIB_TANGENT;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0 | DRG_VERTEX_ATTRIB_COLOR1;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_COLOR0 | DRG_VERTEX_ATTRIB_COLOR1 | DRG_VERTEX_ATTRIB_TANGENT;
const unsigned int DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_NORMAL | DRG_VERTEX_ATTRIB_TANGENT | DRG_VERTEX_ATTRIB_POS_MORPH | DRG_VERTEX_ATTRIB_NORMAL_MORPH;
const unsigned int DRG_VERTEX_TEXTURED_COLORED::attribs = DRG_VERTEX_ATTRIB_POSITION | DRG_VERTEX_ATTRIB_TEX0 | DRG_VERTEX_ATTRIB_COLOR0;

// GetVertexSize
int drgVertHelper::GetVertexSize(unsigned int vertexAttribs)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return sizeof(DRG_VERTEX_COLORED);
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return sizeof(DRG_VERTEX_TEXTURED_COLORED);
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_COLORED);
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_COLORED);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT);
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return sizeof(DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH);
	}
	default:
		drgDebug::Error("Tried to get the size of a vertex with no matching vertex attribs");
		return 0;
	}
}

drgVec3 *drgVertHelper::GetVertexPosition(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return &(((DRG_VERTEX_COLORED *)vertices)[index]).position;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_TEXTURED_COLORED *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_COLORED *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT *)vertices)[index]).position;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).position;
	}
	default:
		drgDebug::Error("Tried to get the vertex position pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

drgVec3 *
drgVertHelper::GetVertexNormal(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_COLORED *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT *)vertices)[index]).normal;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).normal;
	}
	default:
		drgDebug::Error("Tried to get the vertex normal pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

drgVec3 *
drgVertHelper::GetVertexTangent(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT *)vertices)[index]).tangent;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).tangent;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT *)vertices)[index]).tangent;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).tangent;
	}
	default:
		drgDebug::Error("Tried to get the vertex tangent pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

drgVec2 *
drgVertHelper::GetVertexTexCoord0(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_TEXTURED_COLORED *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT *)vertices)[index]).uv;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).uv;
	}
	default:
		drgDebug::Error("Tried to get the vertex uv coord pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

drgVec2 *
drgVertHelper::GetVertexTexCoord1(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return NULL;
	}
	default:
		drgDebug::Error("Tried to get the vertex tex coord1 pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

unsigned int *
drgVertHelper::GetVertexColor0(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return &(((DRG_VERTEX_COLORED *)vertices)[index]).color;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_TEXTURED_COLORED *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_COLORED *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT *)vertices)[index]).color;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).color;
	}
	default:
		drgDebug::Error("Tried to get the vertex color pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

unsigned int *
drgVertHelper::GetVertexColor1(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED *)vertices)[index]).color2;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT *)vertices)[index]).color2;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return NULL;
	}
	default:
		drgDebug::Error("Tried to get the vertex color1 pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

drgVec4 *
drgVertHelper::GetVertexWeight0(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).weight1;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).weight1;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return NULL;
	}
	default:
		drgDebug::Error("Tried to get the vertex weight0 pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

drgVec4 *
drgVertHelper::GetVertexWeight1(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).weight2;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return &(((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED *)vertices)[index]).weight2;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return NULL;
	}
	default:
		drgDebug::Error("Tried to get the vertex weight1 pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

unsigned char *
drgVertHelper::GetVertexPosOffset(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return (((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).posOffset;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return (((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).posOffset;
	}
	default:
		drgDebug::Error("Tried to get the vertex weight1 pointer for a set of unmatched vertex attributes");
		return 0;
	}
}

unsigned char *
drgVertHelper::GetVertexNormalOffset(unsigned int vertexAttribs, void *vertices, int index)
{
	switch (vertexAttribs)
	{
	case DRG_VERTEX_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH::attribs:
	{
		return (((DRG_VERTEX_NORMAL_TEXTURED_COLORED_SKINNED_TWOWEIGHTED_MORPH *)vertices)[index]).normOffset;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_SKINNED_TWOWEIGHTED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_TWOCOLORED_TANGENT::attribs:
	{
		return NULL;
	}
	case DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH::attribs:
	{
		return (((DRG_VERTEX_NORMAL_TEXTURED_COLORED_TANGENT_MORPH *)vertices)[index]).normOffset;
	}
	default:
		drgDebug::Error("Tried to get the vertex weight1 pointer for a set of unmatched vertex attributes");
		return 0;
	}
}