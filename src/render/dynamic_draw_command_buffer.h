#ifndef __DRG_DYNAMIC_DRAW_LIST__
#define __DRG_DYNAMIC_DRAW_LIST__

#include "render/texture.h"
#include "render/draw_command_buffer.h"
#include "render/command_nodes.h"
#include "render/verts.h"

#define DRG_DRAW_PRIORITY_FIRST 1
#define DRG_DRAW_PRIORITY_LAST 255
#define DRG_DRAW_PRIORITY_NORMAL 64
#define DRG_DRAW_PRIORITY_ALPHA 164

class drgRenderContext;
class drgMaterial;
class drgVertBuffer;

class drgDynamicDrawCommandBuffer : public drgDrawCommandBuffer
{
public: // Methods
	drgDynamicDrawCommandBuffer(drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type = DRG_COMMAND_BUFFER_DYNAMIC, drgRenderCache *renderCache = NULL);
	virtual ~drgDynamicDrawCommandBuffer();

	virtual void Init();
	virtual void Destroy();

	virtual void Begin();
	virtual void Draw(bool flushBuffer = true);
	virtual void End();

	void AddRect(drgBBox *rect, drgBBox *trect, drgColor *col);
	void AddRectHom(drgVec3 *v1, drgVec3 *v2, drgVec3 *v3, drgVec3 *v4, drgBBox *trect, drgColor *col);

	void AddDynamicTexturedVertDraw(drgTexture *texture, int numVerts);

	void AddVert2D(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void AddVert2D(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float u, float v);
	void AddVert2DHom(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float u, float v);

	void DrawLine(drgVec3 *point, drgVec3 *point2, drgColor color, drgColor color2);
	void AddLineVert(float x, float y, float z, drgColor color);

	void DrawRect(drgBBox *rect, drgColor *col);
	void DrawRect(drgBBox *rect, drgBBox *trect, drgColor *col, drgTexture *tex);
	void DrawRect(drgVec3 *corners, drgColor *col);									 // Assumes four corners in topleft, topright, bottomleft, bottom right order.
	void DrawRect(drgVec3 *corners, drgColor *col, drgBBox *trect, drgTexture *tex); // Assumes four corners in topleft, topright, bottomleft, bottom right order.

	void DrawRectTransformed(drgBBox *rect, drgColor *col, drgBBox *trect, drgTexture *tex, drgMat44 *transform);

protected: // Members
	struct _DynamicVertices
	{
		drgMaterial *m_Material;
		drgVertBuffer *m_VertBuffer[DRG_NUM_DRAW_BUFFERS];
		unsigned int m_VertBufferSize[DRG_NUM_DRAW_BUFFERS];
		unsigned int m_VertStartOffset[DRG_NUM_DRAW_BUFFERS];
		unsigned char *m_Verts[DRG_NUM_DRAW_BUFFERS];

		_DynamicVertices();
		virtual ~_DynamicVertices();
		void CleanUp();

		void UpdateVertices(int currentDrawBuffer);
	};

	drgRenderContext *m_Context;
	_DynamicVertices m_TexturedQuadVertices;
	_DynamicVertices m_LineVertices;

protected: // Methods
	virtual void AddNode(DRG_DRAW_COMMAND_TYPE type, void *data, int dataSize);
	virtual void DispatchCommand(DRG_DRAW_COMMAND_TYPE command, int &currentOffset);

	virtual void ProcessDynamicDraw(int &currentOffset);
	virtual void ProcessDynamicLineDraw(int &currentOffset);

	void InitializeDynamicVerts(_DynamicVertices *verts, const char *materialName, DRG_PRIMITIVE_TYPE prim, int maxNumVerts);
};

#endif