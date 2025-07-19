
#include "system/global.h"
#if USE_WEBGPU

#include "render/vertexbuff.h"
#include "render/material_manager.h"
#include "render/camera.h"
#include "system/extern_inc.h"

unsigned int drgVertBuffer::m_LastRenderFrame = 9999;
unsigned int drgVertBuffer::m_NumBuffers = 0;
unsigned int drgVertBuffer::m_NumBindingChanges = 0;
drgVertBufferRenderNode* drgVertBuffer::m_RenderHead = NULL;
drgVertBufferRenderNode drgVertBuffer::m_RenderList[DRG_VERT_BUFFER_NUM_RENDER_NODES];

extern unsigned int g_VertBufferMemorySize;


//!Constructor
drgIndexBuffer::drgIndexBuffer(void) {
	m_pIB = NULL;
	m_NumIndex = 0;
}

//!Destructor
drgIndexBuffer::~drgIndexBuffer(void) {
	CleanUp();
}

//!Function that cleans up the vertex index buffer
void drgIndexBuffer::CleanUp() {
	if (m_pIB) {
		drgMemFree(m_pIB);
		m_pIB = NULL;
	}

	m_NumIndex = 0;
}

//!Function that creates a new vertex index buffer
//! \param numIndex index to create
void drgIndexBuffer::Create(unsigned int numIndex) {
	if (numIndex == 0) {
		return;
	}

	CleanUp();

	m_NumIndex = numIndex;

	m_pIB = (unsigned short*)drgMemAlign(128, m_NumIndex * sizeof(short));
}

//!Function that locks the vertex index buffer.
//! \returns the index number if the lock was successful or NULL if not.
unsigned short* drgIndexBuffer::Lock() {
	if (!m_pIB) {
		return NULL;
	}

	unsigned short* retIndx = m_pIB;
	return retIndx;
}

//!Function that unlocks the vertex index buffer
void drgIndexBuffer::UnLock() {
}


//!Constructor
drgVertBuffer::drgVertBuffer(void) {
	m_pVB = NULL;
	m_VertSize = 0;
	m_NumVerts = 0;
	m_NumPrimitives = 0;
	m_Indexed = false;
	m_RenderFlags = 0;
	m_Flags = 0;

	m_VertBufferObject = 0;
	m_IndexBufferObject = 0;

	m_Context = NULL;
	m_PrimitiveType = (DRG_PRIMITIVE_TYPE)0;
	m_Material = NULL;
	m_VertexAttributes = 0;
	m_fAdjustmentColor[0] = 0.0f;
	m_fAdjustmentColor[1] = 0.0f;
	m_fAdjustmentColor[2] = 0.0f;
	m_fAdjustmentAlpha = 0.0f;
	m_pVertColors = NULL;

	m_AnimType = 0;
	m_AnimVal1 = 0;
	m_AnimVal2 = 0;
	m_Offset.Set(0.0f, 0.0f, 0.0f);

	m_bDisableVertColors = false;
	m_bIsDynamic = false;
}

//!Destructor
drgVertBuffer::~drgVertBuffer(void) {
	CleanUp();
}

//!Function that cleans up the vertex buffer
void drgVertBuffer::CleanUp() {
	if (m_pVB) {
		drgMemFree(m_pVB);
		m_pVB = NULL;
	}

	m_VertSize = 0;
	m_NumVerts = 0;
	m_NumPrimitives = 0;
	m_Indexed = false;
	m_Index.CleanUp();
	m_RenderFlags = 0;

	ReleaseGLResources();
}

//!Function that creates a new vertex buffer
/*!
	\param context: The render context that should be used by this vert buffer to render.
	\param numVerts number of vertices to store in the vertex buffer
	\param material: The material that should be used to draw this vertex buffer
	\param prim type of primitive to store in the vertex buffer
	\param numIndex bool to determine whether or not to index the vertices
	\param effect type of rendering effect to use
*/
void drgVertBuffer::Create(drgRenderContext* context, unsigned int numVerts, drgMaterial* material, DRG_PRIMITIVE_TYPE prim, unsigned int numIndex, bool isDynamic) {
	if (numVerts == 0)
		return;

	CleanUp();
	m_Context = context;

	m_PrimitiveType = prim;
	m_Material = material;
	m_NumVerts = numVerts;

	// Create our indices if necessary
	m_Indexed = false;
	if (numIndex) {
		m_Indexed = true;
		m_Index.Create(numIndex);
	}

	// Verify material information
	assert(m_Material); // Need to get m_VertexAttributes from the material provided
	m_VertexAttributes = m_Material->GetVertexAttributes();
	assert(m_VertexAttributes);

	SetNumPrimitives();
	m_VertSize = drgVertHelper::GetVertexSize(m_VertexAttributes);
	m_pVB = (short*)drgMemAlign(128, m_NumVerts * m_VertSize);

	m_bIsDynamic = isDynamic;

	// Create our buffer objects
	InitResources();
}

void drgVertBuffer::SetNumPrimitives() {
	if (m_Indexed) {
		m_NumPrimitives = m_Index.GetNumIndex();
	}
	else {
		m_NumPrimitives = m_NumVerts;
	}

	switch (m_PrimitiveType) {
	case PRIM_LINELIST:
		m_NumPrimitives /= 2;
		break;

	case PRIM_LINESTRIP:
		m_NumPrimitives -= 1;
		break;

	case PRIM_TRIANGLELIST:
		m_NumPrimitives /= 3;
		break;

	case PRIM_TRIANGLESTRIP:
	case PRIM_TRIANGLEFAN:
		m_NumPrimitives -= 2;
		break;

	case PRIM_POINTLIST:
	default:
		break;
	}

}

//!Function that returns whether or not the vertex buffer has been created.
//! \Returns true if the vertex buffer has been created and false if the buffer has not been created.
bool drgVertBuffer::IsCreated()
{
	if (!m_pVB)
		return false;
	return true;
}

//!Function that returns whether of not the vertex buffer is textured.
//! \returns true if the vertex buffer is textured and false if the buffer is not textured.
bool drgVertBuffer::IsTextured()
{
	return drgVertHelper::GetVertexTexCoord0(m_VertexAttributes, m_pVB, 0) != NULL;
}

//!Function that locks the vertex buffer.
//! \returns a pointer to the list of vertices if the lock was successful and NULL if the lock was unsuccessful.
void* drgVertBuffer::Lock()
{
	if (!m_pVB)
		return NULL;

	void* retVerts = m_pVB;
	return retVerts;
}

//!Function that unlocks the vertex buffer
void drgVertBuffer::UnLock(int numVerts)
{
	if (numVerts > 0) {
		m_NumVerts = numVerts;
	}
	else {
		return;
	}

	SetNumPrimitives();

	if (m_bIsDynamic == false) {
		ReleaseGLResources();
		InitResources();
	}
	else {
		drgRenderContext_WEBGPU* context = (drgRenderContext_WEBGPU*)m_Context;
		wgpuQueueWriteBuffer(context->m_Queue, (WGPUBuffer)m_VertBufferObject, 0, m_pVB, m_NumVerts * m_VertSize);
		//pglBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(&offset), m_NumVerts * m_VertSize, (GLvoid*)m_pVB );	GL_ERROR( "Failed to set the dynamic VBO data" );
	}
}

//!Function that locks the vertex index buffer.
/*!
	\returns the index number of the vertex index buffer if the lock was successful or NULL if not.
*/
unsigned short* drgVertBuffer::LockIndex()
{
	if (!m_Indexed)
		return NULL;
	return m_Index.Lock();
}

//!Function that unlocks the vertex index buffer.
void drgVertBuffer::UnLockIndex()
{
	if (!m_Indexed)
		return;
	m_Index.UnLock();
}

//#define OGL_QUERY(x)	(x)
#define OGL_QUERY(x)	(true)

drgVertBuffer* drgVertBuffer::CreateIndicies()
{
	if (m_Indexed) return NULL;						// already indexed

	unsigned short* indexbuffer = (unsigned short*)malloc(sizeof(unsigned short) * m_NumVerts);
	memset(indexbuffer, 0xff, sizeof(unsigned short) * m_NumVerts);

	unsigned short* uniquebuffer = (unsigned short*)malloc(sizeof(unsigned short) * m_NumVerts);   // worst case we have all unique verts
	memset(uniquebuffer, 0xff, sizeof(unsigned short) * m_NumVerts);

	int numUniques = 0;

	for (unsigned short i = 0; i < m_NumVerts; i++)
	{
		drgVec3* p1 = drgVertHelper::GetVertexPosition(m_VertexAttributes, m_pVB, i);
		assert(p1);

		unsigned short j = 0;
		for (; j < numUniques; j++)
		{
			drgVec3* p2 = drgVertHelper::GetVertexPosition(m_VertexAttributes, m_pVB, j);
			assert(p2);

			if ((p1->x == p2->x) &&
				(p1->y == p2->y) &&
				(p1->z == p2->z))			// We have a match
				break;
		}

		indexbuffer[i] = j;				// add index record;
		if (j == numUniques) {				// add a new unique record
			uniquebuffer[numUniques] = i;
			numUniques++;
		}
	}


	// Now make a new vert buffer to replace the old one
	drgVertBuffer* tmpVertBuffer = new drgVertBuffer();
	drgMemory::MemSet(tmpVertBuffer, 0, sizeof(drgVertBuffer));
	tmpVertBuffer->Create(m_Context, numUniques, NULL, PRIM_TRIANGLELIST, m_NumVerts);
	tmpVertBuffer->m_bDisableVertColors = true;
	tmpVertBuffer->m_bIsDynamic = false;

	// Copy the verts which are unique
	void* pVertices = tmpVertBuffer->Lock();
	drgMemory::MemCopy(pVertices, m_pVB, drgVertHelper::GetVertexSize(m_VertexAttributes) * m_NumVerts);
	tmpVertBuffer->UnLock();

	// Copy the index we generated
	unsigned short* pIndices = (unsigned short*)tmpVertBuffer->LockIndex();
	if (pIndices != NULL)
	{
		memcpy(pIndices, indexbuffer, sizeof(unsigned short) * m_NumVerts);
		tmpVertBuffer->UnLockIndex();
	}

	free(indexbuffer);
	free(uniquebuffer);

	return tmpVertBuffer;
}

void drgVertBuffer::InitResources()
{
	drgRenderContext_WEBGPU* context = (drgRenderContext_WEBGPU * )m_Context;
	WGPUBufferDescriptor desc_vbuff;
	drgMemory::MemSet(&desc_vbuff, 0, sizeof(WGPUBufferDescriptor));
	desc_vbuff.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
	desc_vbuff.size = m_NumVerts * m_VertSize;
	m_VertBufferObject = wgpuDeviceCreateBuffer(context->m_Device, &desc_vbuff);


	//pglGenBuffers(1, &m_VertBufferObject);														GL_ERROR("Failed to generate vertex buffer object");

	//pglBindBuffer(GL_ARRAY_BUFFER, m_VertBufferObject);											GL_ERROR("Failed to bind the VBO for setting the data");

	//if (!m_bIsDynamic)
	//{
	//	pglBufferData(GL_ARRAY_BUFFER, m_NumVerts * m_VertSize, (GLvoid*)m_pVB, GL_STATIC_DRAW);	GL_ERROR("Failed to set the VBO data");
	//}
	//else
	//{
	//	pglBufferData(GL_ARRAY_BUFFER, m_NumVerts * m_VertSize, (GLvoid*)m_pVB, GL_DYNAMIC_DRAW);	GL_ERROR("Failed to set the dynamic VBO data");
	//}
	//if (m_Indexed)
	//{
	//	pglGenBuffers(1, &m_IndexBufferObject);
	//	pglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);
	//	pglBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Index.GetNumIndex() * sizeof(unsigned short), (GLvoid*)m_Index.GetIndex(), GL_STATIC_DRAW);
	//}
}

void drgVertBuffer::ReleaseGLResources()
{
	if (m_VertBufferObject) {
		if (!drgEngine::IsClosing()) {
			WGPUProcBufferDestroy(m_VertBufferObject);
			//pglDeleteBuffers(1, &m_VertBufferObject);		GL_ERROR("Failed to delete VBO");
			m_VertBufferObject = NULL;
		}
		m_VertBufferObject = 0;
	}

	if (m_IndexBufferObject) {
		if (!drgEngine::IsClosing()) {
			WGPUProcBufferDestroy(m_IndexBufferObject);
			//pglDeleteBuffers(1, &m_IndexBufferObject);	GL_ERROR("Failed to delete IBO");
			m_IndexBufferObject = NULL;
		}
		m_IndexBufferObject = 0;
	}
}

void drgVertBuffer::SetVertAttribute(int _attrib, unsigned int size, unsigned int type, bool normalized, unsigned int stride, void* data) {
	//pglVertexAttribPointer(_attrib, size, type, (int)normalized, stride, data);	GL_ERROR("Failed to set the vertex attribute pointer");
	//pglEnableVertexAttribArray(_attrib);										GL_ERROR("Failed to enable a vertex attribute array");
}

void drgVertBuffer::BindAttributeArrays(unsigned int attributes, void* vertbuf, drgMaterial* material) {
	bool hasVerts = drgVertHelper::GetVertexPosition(attributes, vertbuf, 0) != NULL;
	bool hasTex = drgVertHelper::GetVertexTexCoord0(attributes, vertbuf, 0) != NULL;
	bool hasNormals = drgVertHelper::GetVertexNormal(attributes, vertbuf, 0) != NULL;
	bool hasColor = drgVertHelper::GetVertexColor0(attributes, vertbuf, 0) != NULL;
	bool hasTangent = drgVertHelper::GetVertexTangent(attributes, vertbuf, 0) != NULL;
	bool hasWeights = drgVertHelper::GetVertexWeight0(attributes, vertbuf, 0) != NULL;
	bool useTempColors = false;

	//if( !m_bDisableVertColors)
	//	useTempColors = adjustVertColors(attributes, vertsize, vertCount, vertbuf);

	drgIntPt bufferStart = (drgIntPt)vertbuf;
	drgMaterialDataWEBGPU *materialData = drgMaterialManagerWEBGPU::GetMaterialData(material->GetMaterialNum());
	assert(materialData && "Need material for attribute locations");

	unsigned int vertSize = drgVertHelper::GetVertexSize(attributes);

	// Bind all vertex attributes
	if (hasWeights) {
		int offset = (drgIntPt)drgVertHelper::GetVertexWeight0(attributes, vertbuf, 0) - bufferStart;
		//SetVertAttribute(materialData->m_Weight0Attribute, 4, GL_FLOAT, false, vertSize, (void*)offset);

		offset = (drgIntPt)drgVertHelper::GetVertexWeight1(attributes, vertbuf, 0) - bufferStart;
		//SetVertAttribute(materialData->m_Weight1Attribute, 4, GL_FLOAT, false, vertSize, (void*)offset);
	}

	if (hasVerts) {
		int offset = (drgIntPt)drgVertHelper::GetVertexPosition(attributes, vertbuf, 0) - bufferStart;
		//SetVertAttribute(materialData->m_PositionAttribute, 3, GL_FLOAT, false, vertSize, (void*)offset);
	}

	if (hasNormals) {
		int offset = (drgIntPt)drgVertHelper::GetVertexNormal(attributes, vertbuf, 0) - bufferStart;
		//SetVertAttribute(materialData->m_NormalAttribute, 3, GL_FLOAT, false, vertSize, (void*)offset);
	}

	if (hasTex) {
		int offset = (drgIntPt)drgVertHelper::GetVertexTexCoord0(attributes, vertbuf, 0) - bufferStart;
		//SetVertAttribute(materialData->m_TexCoord0Attribute, 2, GL_FLOAT, false, vertSize, (void*)offset);
	}

	if (m_bDisableVertColors) {
		hasColor = false;
	}
	else if (hasColor) {
		int offset = (drgIntPt)drgVertHelper::GetVertexColor0(attributes, vertbuf, 0) - bufferStart;

		if (useTempColors) {
			//SetVertAttribute(materialData->m_Color0Attribute, 4, GL_UNSIGNED_BYTE, true, 4, m_pVertColors);
		}
		else {
			//SetVertAttribute(materialData->m_Color0Attribute, 4, GL_UNSIGNED_BYTE, true, vertSize, (void*)offset);
		}
	}
}

//!Function that renders the contents of the vertex buffer.
//TODO:what do the params do?
void drgVertBuffer::Render(unsigned int start, unsigned int numPrims, bool useEffect, bool setup, drgMaterial* material) {
	if (!m_pVB) {
		return;
	}

	if ((numPrims + start) > m_NumPrimitives) {
		numPrims = (m_NumPrimitives - start);
	}

	void* indeces = NULL;
	if (m_Indexed) {
		indeces = m_Index.GetIndex();
	}

	unsigned int vertAttributes = m_VertexAttributes;
	drgMaterial* renderMaterial = m_Material;
	drgRenderContext_WEBGPU* context = (drgRenderContext_WEBGPU*)m_Context;

	// Are we setting up?
	if (setup) {
		// Setup our material if we need to
		if (useEffect) {
			if (material) {
				renderMaterial = material;
				vertAttributes = renderMaterial->GetVertexAttributes();
			}

			if (renderMaterial && setup && useEffect)
				renderMaterial->StartRender(context);
		}

		// If we are static, then just bind the VAO
		if (!m_bIsDynamic) {
			//if( m_CurrentVertObject != m_VertObject)
			{
				//m_CurrentVertObject = m_VertObject;
				//pglBindVertexArray(m_VertObject);		GL_ERROR();
				wgpuRenderPassEncoderSetVertexBuffer(context->m_RenderPass, 0, (WGPUBuffer)m_VertBufferObject, 0, WGPU_WHOLE_SIZE);
				//pglBindBuffer(GL_ARRAY_BUFFER, m_VertBufferObject);					GL_ERROR("Failed to bind the vertex buffer object");
				if (m_Indexed) {
					//pglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);		GL_ERROR("Failed to bind the index buffer object");
				}
				BindAttributeArrays(vertAttributes, m_pVB, renderMaterial);
				m_NumBindingChanges++;
			}
		}
		else {
			// Regenerate the VAO and bind it.
			wgpuRenderPassEncoderSetVertexBuffer(context->m_RenderPass, 0, (WGPUBuffer)m_VertBufferObject, 0, WGPU_WHOLE_SIZE);
			//pglBindBuffer(GL_ARRAY_BUFFER, m_VertBufferObject);
			if (m_Indexed) {
				//pglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);
			}
			BindAttributeArrays(vertAttributes, m_pVB, renderMaterial);
		}
	}

	// Draw vertex buffer now
	assert(context);
	context->m_VBuffer = (WGPUBuffer)m_VertBufferObject;
	context->RenderPrimitive(m_RenderFlags, m_PrimitiveType, numPrims, m_NumVerts, m_pVB, m_VertSize, indeces, useEffect, start);

	// Unbind the material if needed.
	if (setup) {
		if (useEffect) {
			m_Material->StopRender();
		}
	}
}


void drgVertBuffer::AddToRenderList(drgMat44* matrix, drgMaterial* material, drgColor* ambient, unsigned short flags)
{
	if (!m_pVB)
		return;

	assert(!(m_LastRenderFrame == drgEngine::GetFrameNum())); // No rendering after the render list has been rendered!!

	drgVertBufferRenderNode* node = drgVertBuffer::FindOpenNode();
	if (!node)
		return;

	node->m_VertBuffer = this;
	node->m_Matrix = (*matrix);
	node->m_Flags = flags;
	node->m_Material = material;

	//if we are trying to override the material, the use that one
	drgMaterial* tmpMaterialMe = m_Material;
	drgMaterial* tmpMaterialCmp = NULL;

	if (material)
		tmpMaterialMe = material;

	if (ambient)
	{
		node->m_Flags |= VERTEX_RENDER_FLAG_AMBIENT;
		node->m_Ambient = (*ambient);
	}
	else
	{
		node->m_Flags &= ~(VERTEX_RENDER_FLAG_AMBIENT);
	}

	drgVertBufferRenderNode* itr = m_RenderHead;
	while (itr)
	{
		//make sure that we are looking at the overridden material if there is one
		tmpMaterialCmp = itr->m_VertBuffer->m_Material;
		if (itr->m_Material)
			tmpMaterialCmp = itr->m_Material;

		if (tmpMaterialCmp->GetTextureAddr(0) == tmpMaterialMe->GetTextureAddr(0))
		{
			node->m_Next = itr->m_Next;
			itr->m_Next = node;
			break;
		}
		itr = itr->m_Next;
	}

	if (!itr)
	{
		node->m_Next = m_RenderHead;
		m_RenderHead = node;
	}
	++m_NumBuffers;
}


void drgVertBuffer::RenderList(bool clear)
{
	m_LastRenderFrame = drgEngine::GetFrameNum();
	drgVertBufferRenderNode* tempnode;

	bool renderingshadows = drgMaterialManager::IsRenderingShadows();

	tempnode = m_RenderHead;

	while (tempnode)
	{
		if (tempnode->m_Flags & VERTEX_RENDER_FLAG_AMBIENT)
			tempnode->m_VertBuffer->GetContext()->SetAmbientColor(tempnode->m_Ambient.r, tempnode->m_Ambient.g, tempnode->m_Ambient.b);

		//if(tempnode->m_Flags & VERTEX_RENDER_FLAG_ALPHABLEND)
		//	tempnode->m_VertBuffer->GetContext()->GetRenderCache()->EnableAlphaBlend(true);

		if (tempnode->m_Material)
			tempnode->m_Material->SetRenderTextures();
		else
			tempnode->m_VertBuffer->m_Material->SetRenderTextures();
		tempnode->m_VertBuffer->GetContext()->SetWorldViewProjectionMatrix(&tempnode->m_Matrix, drgCamera::GetDefaultCamera().GetViewProjectionMatrix());
		tempnode->m_VertBuffer->Render(0, tempnode->m_VertBuffer->m_NumPrimitives, true, true, tempnode->m_Material);

		if (tempnode->m_Flags & VERTEX_RENDER_FLAG_AMBIENT)
			tempnode->m_VertBuffer->GetContext()->ResetAmbient();

		//if(tempnode->m_Flags & VERTEX_RENDER_FLAG_ALPHABLEND)
		//	tempnode->m_VertBuffer->GetContext()->GetRenderCache()->EnableAlphaBlend(false);

		//tempnode->m_Model->Render(tempnode);
		if (clear)
			tempnode->m_VertBuffer = NULL;
		tempnode = tempnode->m_Next;

	}

	if (clear)
	{
		if (m_RenderHead)
		{
			m_RenderHead->Clear();
			m_RenderHead = NULL;
		}
		m_NumBuffers = 0;
	}
}

drgVertBufferRenderNode* drgVertBuffer::FindOpenNode()
{
	if (m_NumBuffers >= DRG_VERT_BUFFER_NUM_RENDER_NODES)
		return NULL;
	for (int count = 0; count < DRG_VERT_BUFFER_NUM_RENDER_NODES; ++count)
		if (!m_RenderList[count].m_VertBuffer)
			return &(m_RenderList[count]);
	return NULL;
}


drgVertBufferRenderNode::drgVertBufferRenderNode()
{
	Clear();
}

drgVertBufferRenderNode::~drgVertBufferRenderNode()
{
	Clear();
}

void drgVertBufferRenderNode::Clear()
{
	m_Next = NULL;
	m_VertBuffer = NULL;
	m_Material = NULL;
	m_Flags = 0;
}

#endif // USE_WEBGPU
