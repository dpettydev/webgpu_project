#ifndef __DRG_VERTEXBUFF_H__
#define __DRG_VERTEXBUFF_H__

#include "verts.h"

//! Vertigo's number of render nodes.
#if (defined _WII)
#define DRG_VERT_BUFFER_NUM_RENDER_NODES (128)
#elif (defined _IPHONE) || defined(_ANDROID)
#define DRG_VERT_BUFFER_NUM_RENDER_NODES (256)
#elif (defined _WINMO)
#define DRG_VERT_BUFFER_NUM_RENDER_NODES (128)
#else
#define DRG_VERT_BUFFER_NUM_RENDER_NODES (512)
#endif

//! Class describing a Vertigo vertex index buffer
class drgIndexBuffer
{
public:
	drgIndexBuffer(void);
	~drgIndexBuffer(void);

	void Create(unsigned int numIndex);
	void CleanUp();
	unsigned short *Lock();
	void UnLock();

	//! Function that returns the index.
	//!\returns the index
	inline unsigned short *GetIndex() { return m_pIB; }

	//! Function that returns the number index.
	//!\returns the number index
	inline unsigned int GetNumIndex() { return m_NumIndex; }

private:
	unsigned short *m_pIB;
	unsigned int m_NumIndex;
};

class drgVertBuffer;

//! Class that handles vert buffer rendering
class drgVertBufferRenderNode
{
protected:
	//! Constructor.
	drgVertBufferRenderNode();

	//! Destructor.
	~drgVertBufferRenderNode();

	//! Clear the list of all render nodes.
	void Clear();

	unsigned short m_Flags;

	//! Buffer render node matrix.
	drgMat44 m_Matrix;
	//! Ambient color.
	drgColor m_Ambient;
	//! Buffer pointer.
	drgVertBuffer *m_VertBuffer;
	//! buffer material
	drgMaterial *m_Material;

	//! Next model node.
	drgVertBufferRenderNode *m_Next;

	friend class drgVertBuffer;
};

//! Class describing a Vertigo vertex buffer.
class drgVertBuffer
{
public:
	drgVertBuffer(void);
	~drgVertBuffer(void);

	enum VERTEX_RENDER_FLAGS
	{
		VERTEX_RENDER_FLAG_ALPHABLEND = (1 << (0)),
		VERTEX_RENDER_FLAG_AMBIENT = (1 << (1))
	};

	void Create(drgRenderContext *context, unsigned int numVerts, drgMaterial *material, DRG_PRIMITIVE_TYPE prim, unsigned int numIndex = 0, bool isDynamic = false);
	bool IsCreated();
	bool IsTextured();
	void CleanUp();
	void *Lock();
	void UnLock(int numVerts = 0);
	unsigned short *LockIndex();
	void UnLockIndex();
	void Render(unsigned int start, unsigned int numprims, bool useeffect = true, bool setup = true, drgMaterial *material = NULL);

	inline DRG_PRIMITIVE_TYPE GetPrimType() { return m_PrimitiveType; };
	inline unsigned int GetRenderFlags() { return m_RenderFlags; };
	inline void *GetBuffer() { return m_pVB; };

	//! Function that sets the render effect that this vertex buffer uses
	//!\param effect type of render effect to use
	inline void SetRenderMaterial(drgMaterial *mat) { m_Material = mat; };

	//! Function that returns the type of render effect that this vertex buffer uses
	//!\returns the type of render effect that this vertex buffer is set to use
	inline drgMaterial *GetRenderMaterial() { return m_Material; }
	inline unsigned int GetNumVerts() { return m_NumVerts; }
	inline unsigned int GetNumIndicies() { return m_Index.GetNumIndex(); }
	inline drgRenderContext *GetContext() { return m_Context; }

	inline void SetContext(drgRenderContext *context) { m_Context = context; }

	void AddToRenderList(drgMat44 *matrix, drgMaterial *material, drgColor *ambient = NULL, unsigned short flags = 0);
	static void RenderList(bool clear);

	int m_Flags;
	char m_AnimType;
	float m_AnimVal1;
	float m_AnimVal2;
	drgVec3 m_Offset;

	bool m_bDisableVertColors; // used by font so we don't modify vert colors
	bool m_bIsDynamic;
	void *m_VertBufferObject;
	void *m_IndexBufferObject;
	static unsigned int m_NumBindingChanges;

	drgVertBuffer *CreateIndicies();
	void ReleaseGLResources();
	void InitResources();

private:
	//! Find an open node within the rendernode list.
	static drgVertBufferRenderNode *FindOpenNode();

	void *m_pVB;
	DRG_PRIMITIVE_TYPE m_PrimitiveType;
	drgMaterial *m_Material;
	unsigned int m_NumVerts;
	unsigned int m_NumPrimitives;
	unsigned int m_VertSize;
	bool m_Indexed;
	unsigned int m_RenderFlags;
	drgIndexBuffer m_Index;
	drgRenderContext *m_Context;
	unsigned int m_VertexAttributes;

	float m_fAdjustmentColor[3];
	float m_fAdjustmentAlpha;
	unsigned char *m_pVertColors;

	inline void BindAttributeArrays(unsigned int vertexAttribute, void *vertbuf, drgMaterial *material);

	enum VERTEX_ATTRIB
	{
		VERT_ATTRIB_COLOR = 0,
		VERT_ATTRIB_VERTPOS,
		VERT_ATTRIB_NORMAL,
		VERT_ATTRIB_TEXCOORD,
		VERT_ATTRIB_TANGENT,
		// VERT_ATTRIB_BINORMAL,
		VERT_ATTRIB_WEIGHT0,
		VERT_ATTRIB_WEIGHT1
	};
	void SetVertAttribute(int _attrib, unsigned int size, unsigned int type, bool normalized, unsigned int stride, void *data);
	void SetNumPrimitives();

	// stuff for the vert buffer render list
	static unsigned int m_LastRenderFrame;
	static unsigned int m_NumBuffers;
	static drgVertBufferRenderNode *m_RenderHead;
	static drgVertBufferRenderNode m_RenderList[DRG_VERT_BUFFER_NUM_RENDER_NODES];
};

#endif // __DRG_VERTEXBUFF_H__
