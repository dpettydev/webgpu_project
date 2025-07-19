
#include "render/dynamic_draw_command_buffer.h"
#include "render/context.h"
#include "render/verts.h"
#include "render/vertexbuff.h"
#include "render/material_manager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// _DynamicVertices functionality
//////////////////////////////////////////////////////////////////////////////////////////////////////

drgDynamicDrawCommandBuffer::_DynamicVertices::_DynamicVertices()
{
	m_Material = NULL;
	for (int currentBuffer = 0; currentBuffer < DRG_NUM_DRAW_BUFFERS; ++currentBuffer)
	{
		m_VertBuffer[currentBuffer] = NULL;
		m_Verts[currentBuffer] = NULL;
		m_VertBufferSize[currentBuffer] = 0;
		m_VertStartOffset[currentBuffer] = 0;
	}
}

drgDynamicDrawCommandBuffer::_DynamicVertices::~_DynamicVertices()
{
	CleanUp();
}

void drgDynamicDrawCommandBuffer::_DynamicVertices::CleanUp()
{
	SAFE_DELETE(m_Material);
	for (int currentBuffer = 0; currentBuffer < DRG_NUM_DRAW_BUFFERS; ++currentBuffer)
	{
		SAFE_FREE(m_Verts[currentBuffer]);
		SAFE_DELETE(m_VertBuffer[currentBuffer]);
	}
}

void drgDynamicDrawCommandBuffer::_DynamicVertices::UpdateVertices(int currentDrawBuffer)
{
	int newNumVerts = m_VertBufferSize[currentDrawBuffer] / drgVertHelper::GetVertexSize(m_Material->GetVertexAttributes());
	void *buffer = m_VertBuffer[currentDrawBuffer]->Lock();
	drgMemory::MemCopy(buffer, m_Verts[currentDrawBuffer], m_VertBufferSize[currentDrawBuffer]);
	m_VertBuffer[currentDrawBuffer]->UnLock(newNumVerts);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// drgDynamicDrawList functionality
//////////////////////////////////////////////////////////////////////////////////////////////////////

drgDynamicDrawCommandBuffer::drgDynamicDrawCommandBuffer(drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type, drgRenderCache *cache)
	: drgDrawCommandBuffer(context)
{
	assert(context && "Can't create dynamic draw list without a context");
	m_Context = context;
	m_CommandBufferType = type;
	m_Cache = cache;
	m_ListID = 0;

	for (int currentBuffer = 0; currentBuffer < DRG_NUM_DRAW_BUFFERS; ++currentBuffer)
	{
		m_DataBuffer[currentBuffer] = NULL;
		m_NumCommands[currentBuffer] = 0;
	}

	m_CurrentDataOffset = 0;
	m_CurrentDrawBuffer = 0;
}

drgDynamicDrawCommandBuffer::~drgDynamicDrawCommandBuffer()
{
	Destroy();
}

void drgDynamicDrawCommandBuffer::Init()
{
	Destroy();

	// Initialize our textured quad material
	InitializeDynamicVerts(&m_TexturedQuadVertices, "VS_UI", PRIM_TRIANGLELIST, DRG_MAX_VERT_SIZE);
	InitializeDynamicVerts(&m_LineVertices, "VS_UI", PRIM_LINELIST, DRG_MAX_LINE_SIZE);

	for (int currentBuffer = 0; currentBuffer < DRG_NUM_DRAW_BUFFERS; ++currentBuffer)
	{
		m_NumCommands[currentBuffer] = 0;
		m_DataBuffer[currentBuffer] = (unsigned char *)drgMemAlign(128, DRG_MAX_COMMAND_SIZE);
	}
}

void drgDynamicDrawCommandBuffer::InitializeDynamicVerts(drgDynamicDrawCommandBuffer::_DynamicVertices *verts, const char *materialName, DRG_PRIMITIVE_TYPE type, int maxNumVerts)
{
	verts->CleanUp();

	MEM_INFO_SET_NEW;
	verts->m_Material = new drgMaterial;
	verts->m_Material->InitMaterial();
	verts->m_Material->SetEffectByName(materialName);

	for (int currentBuffer = 0; currentBuffer < DRG_NUM_DRAW_BUFFERS; ++currentBuffer)
	{
		// This could potentially change as we add new things to the dynamic draw buffer.
		int numVerts = maxNumVerts / drgVertHelper::GetVertexSize(verts->m_Material->GetVertexAttributes());
		MEM_INFO_SET_NEW;
		verts->m_VertBuffer[currentBuffer] = new drgVertBuffer;
		verts->m_VertBuffer[currentBuffer]->Create(m_Context, numVerts, verts->m_Material, type, 0, true);
		verts->m_Verts[currentBuffer] = (unsigned char *)drgMemAlign(128, maxNumVerts);
		verts->m_VertBufferSize[currentBuffer] = 0;
		verts->m_VertStartOffset[currentBuffer] = 0;
	}
}

void drgDynamicDrawCommandBuffer::Destroy()
{
	for (int currentBuffer = 0; currentBuffer < DRG_NUM_DRAW_BUFFERS; ++currentBuffer)
	{
		SAFE_FREE(m_DataBuffer[currentBuffer])
	}
}

void drgDynamicDrawCommandBuffer::Begin()
{
	m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] = 0;
	m_LineVertices.m_VertBufferSize[m_CurrentDrawBuffer] = 0;

	drgDrawCommandBuffer::Begin();
}

void drgDynamicDrawCommandBuffer::End()
{
	drgDrawCommandBuffer::End();
}

void drgDynamicDrawCommandBuffer::AddDynamicTexturedVertDraw(drgTexture *texture, int numVerts)
{
	int startOffset = m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] / sizeof(DRG_VERTEX_TEXTURED_COLORED);
	drgDynamicTexturedVertNode texturedNode(startOffset, numVerts, texture);
	AddNode(DRG_DRAW_COMMAND_DYNAMIC_DRAW_VERT_LIST_TEX, &texturedNode, sizeof(drgDynamicTexturedVertNode));
}

void drgDynamicDrawCommandBuffer::AddRect(drgBBox *rect, drgBBox *trect, drgColor *col)
{
	AddVert2D(rect->minv.x, rect->minv.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->minv.y);
	AddVert2D(rect->maxv.x, rect->minv.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(rect->minv.x, rect->maxv.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(rect->minv.x, rect->maxv.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(rect->maxv.x, rect->minv.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(rect->maxv.x, rect->maxv.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->maxv.y);
}

void drgDynamicDrawCommandBuffer::AddRectHom(drgVec3 *v1, drgVec3 *v2, drgVec3 *v3, drgVec3 *v4, drgBBox *trect, drgColor *col)
{
	AddVert2D(v1->x, v1->y, col->r, col->g, col->b, col->a, trect->minv.x, trect->minv.y);
	AddVert2D(v3->x, v3->y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(v2->x, v2->y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(v2->x, v2->y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(v3->x, v3->y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(v4->x, v4->y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->maxv.y);
}

void drgDynamicDrawCommandBuffer::AddVert2D(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	int currentOffset = m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer];
	DRG_VERTEX_TEXTURED_COLORED *vert = (DRG_VERTEX_TEXTURED_COLORED *)&m_TexturedQuadVertices.m_Verts[m_CurrentDrawBuffer][currentOffset];
	drgColor color = drgColor(r, g, b, a);

	vert->position.x = x / m_Context->GetDrawWidth();
	vert->position.y = y / m_Context->GetDrawHeight();
	vert->position.z = 0.0f;
	vert->color = color.Get8888();
	vert->uv.Set(0.0f, 0.0f); // No UV, but it'll still look up from the texture

	m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] += sizeof(DRG_VERTEX_TEXTURED_COLORED);
	assert(m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] < DRG_MAX_VERT_SIZE);
}

void drgDynamicDrawCommandBuffer::AddVert2D(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float u, float v)
{
	int currentOffset = m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer];
	DRG_VERTEX_TEXTURED_COLORED *vert = (DRG_VERTEX_TEXTURED_COLORED *)&m_TexturedQuadVertices.m_Verts[m_CurrentDrawBuffer][currentOffset];
	drgColor color = drgColor(r, g, b, a);

	vert->position.x = x / m_Context->GetDrawWidth();
	vert->position.y = y / m_Context->GetDrawHeight();
	vert->position.z = 0.0f;
	vert->color = color.Get8888();
	vert->uv.Set(u, v);

	// drgPrintOut("VTX  IN-%i: %f %f\r\n", currentOffset/ sizeof(DRG_VERTEX_TEXTURED_COLORED), vert->position.x, vert->position.y);

	m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] += sizeof(DRG_VERTEX_TEXTURED_COLORED);
	assert(m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] < DRG_MAX_VERT_SIZE);
}

void drgDynamicDrawCommandBuffer::AddVert2DHom(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float u, float v)
{
	int currentOffset = m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer];
	DRG_VERTEX_TEXTURED_COLORED *vert = (DRG_VERTEX_TEXTURED_COLORED *)&m_TexturedQuadVertices.m_Verts[m_CurrentDrawBuffer][currentOffset];
	drgColor color = drgColor(r, g, b, a);

	vert->position.x = x;
	vert->position.y = y;
	vert->position.z = 0.0f;
	vert->color = color.Get8888();
	vert->uv.Set(u, v);

	m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] += sizeof(DRG_VERTEX_TEXTURED_COLORED);
	assert(m_TexturedQuadVertices.m_VertBufferSize[m_CurrentDrawBuffer] < DRG_MAX_VERT_SIZE);
}

void drgDynamicDrawCommandBuffer::AddLineVert(float x, float y, float z, drgColor color)
{
	int currentOffset = m_LineVertices.m_VertBufferSize[m_CurrentDrawBuffer];
	DRG_VERTEX_COLORED *vert = (DRG_VERTEX_COLORED *)&m_LineVertices.m_Verts[m_CurrentDrawBuffer][currentOffset];

	vert->position.x = x;
	vert->position.y = y;
	vert->position.z = z;
	vert->color = color.Get8888();

	m_LineVertices.m_VertBufferSize[m_CurrentDrawBuffer] += sizeof(DRG_VERTEX_COLORED);
	assert(m_LineVertices.m_VertBufferSize[m_CurrentDrawBuffer] < DRG_MAX_LINE_SIZE);
}

void drgDynamicDrawCommandBuffer::DrawRect(drgBBox *rect, drgColor *col)
{
	AddDynamicTexturedVertDraw(drgTexture::GetNullTexture(), 6);

	AddVert2D(rect->minv.x, rect->minv.y, col->r, col->g, col->b, col->a, 0.0f, 0.0f);
	AddVert2D(rect->maxv.x, rect->minv.y, col->r, col->g, col->b, col->a, 1.0f, 0.0f);
	AddVert2D(rect->minv.x, rect->maxv.y, col->r, col->g, col->b, col->a, 0.0f, 1.0f);
	AddVert2D(rect->minv.x, rect->maxv.y, col->r, col->g, col->b, col->a, 0.0f, 1.0f);
	AddVert2D(rect->maxv.x, rect->minv.y, col->r, col->g, col->b, col->a, 1.0f, 0.0f);
	AddVert2D(rect->maxv.x, rect->maxv.y, col->r, col->g, col->b, col->a, 1.0f, 1.0f);
}

void drgDynamicDrawCommandBuffer::DrawRect(drgBBox *rect, drgBBox *trect, drgColor *col, drgTexture *tex)
{
	AddDynamicTexturedVertDraw(tex, 6);

	AddVert2D(rect->minv.x, rect->minv.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->minv.y);
	AddVert2D(rect->maxv.x, rect->minv.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(rect->minv.x, rect->maxv.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(rect->minv.x, rect->maxv.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(rect->maxv.x, rect->minv.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(rect->maxv.x, rect->maxv.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->maxv.y);
}

void drgDynamicDrawCommandBuffer::DrawRect(drgVec3 *corners, drgColor *col)
{
	// Create the corners of the rect.
	drgVec3 topLeft(corners[0]);
	drgVec3 topRight(corners[1]);
	drgVec3 bottomLeft(corners[2]);
	drgVec3 bottomRight(corners[3]);

	// Add the vertices
	AddDynamicTexturedVertDraw(drgTexture::GetNullTexture(), 6);

	AddVert2D(topLeft.x, topLeft.y, col->r, col->g, col->b, col->a, 0.0f, 0.0f);
	AddVert2D(topRight.x, topRight.y, col->r, col->g, col->b, col->a, 1.0f, 0.0f);
	AddVert2D(bottomLeft.x, bottomLeft.y, col->r, col->g, col->b, col->a, 0.0f, 1.0f);
	AddVert2D(bottomLeft.x, bottomLeft.y, col->r, col->g, col->b, col->a, 0.0f, 1.0f);
	AddVert2D(topRight.x, topRight.y, col->r, col->g, col->b, col->a, 1.0f, 0.0f);
	AddVert2D(bottomRight.x, bottomRight.y, col->r, col->g, col->b, col->a, 1.0f, 1.0f);
}

void drgDynamicDrawCommandBuffer::DrawRect(drgVec3 *corners, drgColor *col, drgBBox *trect, drgTexture *tex)
{
	// Create the corners of the rect.
	drgVec3 topLeft(corners[0]);
	drgVec3 topRight(corners[1]);
	drgVec3 bottomLeft(corners[2]);
	drgVec3 bottomRight(corners[3]);

	// Add the vertices
	AddDynamicTexturedVertDraw(tex, 6);

	AddVert2D(topLeft.x, topLeft.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->minv.y);
	AddVert2D(topRight.x, topRight.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(bottomLeft.x, bottomLeft.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(bottomLeft.x, bottomLeft.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(topRight.x, topRight.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(bottomRight.x, bottomRight.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->maxv.y);
}

void drgDynamicDrawCommandBuffer::DrawRectTransformed(drgBBox *rect, drgColor *col, drgBBox *trect, drgTexture *tex, drgMat44 *transform)
{
	// Create the corners of the rect.
	drgVec3 topLeft(rect->minv.x, rect->minv.y, 0.0f);
	drgVec3 topRight(rect->maxv.x, rect->minv.y, 0.0f);
	drgVec3 bottomLeft(rect->minv.x, rect->maxv.y, 0.0f);
	drgVec3 bottomRight(rect->maxv.x, rect->maxv.y, 0.0f);

	// Transform those corners
	assert(transform);
	transform->Transform(topLeft);
	transform->Transform(topRight);
	transform->Transform(bottomLeft);
	transform->Transform(bottomRight);

	// Add the vertices
	AddDynamicTexturedVertDraw(tex, 6);

	AddVert2D(topLeft.x, topLeft.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->minv.y);
	AddVert2D(topRight.x, topRight.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(bottomLeft.x, bottomLeft.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(bottomLeft.x, bottomLeft.y, col->r, col->g, col->b, col->a, trect->minv.x, trect->maxv.y);
	AddVert2D(topRight.x, topRight.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->minv.y);
	AddVert2D(bottomRight.x, bottomRight.y, col->r, col->g, col->b, col->a, trect->maxv.x, trect->maxv.y);
}

void drgDynamicDrawCommandBuffer::DrawLine(drgVec3 *point1, drgVec3 *point2, drgColor color1, drgColor color2)
{
	int bufferOffset = m_LineVertices.m_VertBufferSize[m_CurrentDrawBuffer] / sizeof(DRG_VERTEX_COLORED);
	drgDynamicLineNode lineNode(bufferOffset, 2);
	AddNode(DRG_DRAW_COMMAND_DYNAMIC_DRAW_LINE, &lineNode, sizeof(drgDynamicLineNode));

	AddLineVert(point1->x, point1->y, point1->z, color1);
	AddLineVert(point2->x, point2->y, point2->z, color2);
}

void drgDynamicDrawCommandBuffer::AddNode(DRG_DRAW_COMMAND_TYPE type, void *data, int dataSize)
{
	drgDrawCommandBuffer::AddNode(type, data, dataSize);
}

void drgDynamicDrawCommandBuffer::DispatchCommand(DRG_DRAW_COMMAND_TYPE command, int &currentOffset)
{
	switch (command)
	{
	case DRG_DRAW_COMMAND_DYNAMIC_DRAW_VERT_LIST:
	case DRG_DRAW_COMMAND_DYNAMIC_DRAW_VERT_LIST_TEX:
		ProcessDynamicDraw(currentOffset);
		break;

	case DRG_DRAW_COMMAND_DYNAMIC_DRAW_LINE:
		ProcessDynamicLineDraw(currentOffset);
		break;

	default:
		drgDrawCommandBuffer::DispatchCommand(command, currentOffset);
		break;
	}
}

void drgDynamicDrawCommandBuffer::ProcessDynamicDraw(int &currentOffset)
{
	drgDynamicTexturedVertNode *vertNode = (drgDynamicTexturedVertNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDynamicTexturedVertNode);

	// Setup our texture sampler. Currently we assume only one texture at sampler zero
	m_TexturedQuadVertices.m_Material->SetTextureAddr(0, vertNode->m_Texture);

	// Draw our vertex buffer
	int startPrimitive = vertNode->m_StartOffset == 0 ? 0 : vertNode->m_StartOffset / 3;
	int numPrimitives = vertNode->m_NumVertices / 3;

	m_TexturedQuadVertices.m_VertBuffer[m_CurrentDrawBuffer]->Render(startPrimitive, numPrimitives);
}

void drgDynamicDrawCommandBuffer::ProcessDynamicLineDraw(int &currentOffset)
{
	drgDynamicLineNode *lineNode = (drgDynamicLineNode *)(m_DataBuffer[m_CurrentDrawBuffer] + currentOffset);
	currentOffset += sizeof(drgDynamicLineNode);

	int startPrimitive = lineNode->m_StartOffset == 0 ? 0 : lineNode->m_StartOffset / 2;
	int numPrimitives = lineNode->m_NumVerts / 2;

	m_LineVertices.m_VertBuffer[m_CurrentDrawBuffer]->Render(startPrimitive, numPrimitives);
}

void drgDynamicDrawCommandBuffer::Draw(bool flushBuffer)
{
	// Update our dynamic vertices.
	m_TexturedQuadVertices.UpdateVertices(m_CurrentDrawBuffer);
	m_LineVertices.UpdateVertices(m_CurrentDrawBuffer);

	// Process our commands.
	drgDrawCommandBuffer::Draw(flushBuffer);
}