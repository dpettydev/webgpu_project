
#include "core/memmgr.h"
#include "render/context.h"
#include "util/stringn.h"
#include "render/vertexbuff.h"
#include "render/model.h"

#if !(defined(EMSCRIPTEN) && defined(_JS_2D))

drgRenderContext::drgRenderContext()
{
	m_WorldViewProjectionMatrix.Identity();
	m_WorldMatrix.Identity();
	m_AmbientColor = drgColor( 255, 255, 255, 255 );
	m_SpecularColor = drgColor( 0, 0, 0, 0 );
	m_DiffuseColor = drgColor( 255, 255, 255, 255 );
	m_TexCoordOffset.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	m_ViewPosition.Set( 0.0f, 0.0f, 0.0f );
	m_ViewDirection.Set( 0.0f, 0.0f, 1.0f );

	for( int currentBone = 0; currentBone < NUM_BONE_MATRICES; ++currentBone )
	{
		m_BoneMatrices[ currentBone ].Identity();
	}

	//m_RenderCache = new drgRenderCache;
	//m_RenderCache->SetCommandBuffer( m_DynamicDrawList );
}

drgRenderContext::~drgRenderContext() {
}

void drgRenderContext::Init() {
}

void drgRenderContext::Begin() {
	m_WorldViewProjectionMatrix.Identity();
}

void drgRenderContext::End() {
}

void drgRenderContext::SetWorldViewProjectionMatrix( drgMat44* world, drgMat44* projection )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetAmbientColor( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetBoneMatrix( drgMat44* matrix, int index )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetWorldMatrix( drgMat44* world )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetSpecular( drgColor* spec )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetDiffuseColor( drgColor* color )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetTexCoordOffset( drgVec4* offset )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetViewPosition( drgVec3* position )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetViewDirection( drgVec3* direction )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::SetupDPI( float xDPI, float yDPI, float xBaseDPI, float yBaseDPI )
{
	m_BaseXDPI = xBaseDPI;
	m_BaseYDPI = yBaseDPI;
	m_XDPI = xDPI;
	m_YDPI = yDPI;

	m_XDPIScalingFactor = m_BaseXDPI / m_XDPI;
	m_YDPIScalingFactor = m_BaseYDPI / m_YDPI;
	drgPrintOut( "[ENGINE] Setup dpi: X DPI: %f, Y DPI: %f\n X Base DPI: %f, Y Base DPI: %f\n X DPI Scaling Factor: %f, Y DPI Scaling Factor: %f\n",
		m_XDPI,
		m_YDPI, 
		m_BaseXDPI,
		m_BaseYDPI,
		m_XDPIScalingFactor,
		m_YDPIScalingFactor );
}

void drgRenderContext::RenderPrimitive( unsigned int renderFlags,
		int primType,
		int numPrims,
		int numVertsTot,
		void* data,
		int vertSize,
		void* indices,
		bool setStream,
		int startVert,
		void* startData, 
		int numVerts,
		int streamOffset )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::ApplyClearTarget( unsigned int clearFlags, drgColor color, float depth, unsigned int stencil )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::ApplyMakeCurrent( void* platformRenderContext, void* platformDeviceContext )
{
	drgPrintError( "Not implemented, this is the platform agnostic layer" );
}

void drgRenderContext::ApplyDrawMesh( drgVertBuffer* buffer, drgMaterial* material, int startPrim, int numPrims, bool setup, bool useEffect )
{
	if( buffer ) {
		buffer->Render( startPrim, numPrims, setup, useEffect, material );
	}
}

void drgRenderContext::ApplyDrawModel( drgModel* model )
{
	if( model ) {
		model->Render();
	}
}

void drgRenderContext::ApplyScissorEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyScissorState( bool enable, short top, short bottom, short left, short right )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyClipRect( short top, short bottom, short left, short right )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyViewport( short top, short bottom, short left, short right )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyColorBlend( bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyAlphaBlend( bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyColorBlendEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyColorSourceBlend( DRG_BLEND_FUNCTION func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyColorDestBlend( DRG_BLEND_FUNCTION func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyColorOperation( DRG_BLEND_OPERATION op )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyAlphaBlendEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyAlphaSourceBlend( DRG_BLEND_FUNCTION func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyAlphaDestBlend( DRG_BLEND_FUNCTION func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyAlphaOperation( DRG_BLEND_OPERATION op )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyDepthState( bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyStencilState( bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyDepthTestEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyDepthWriteEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyDepthCompareFunc( DRG_COMPARE_FUNC func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyStencilTestEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyStencilWriteEnable( bool enable )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyStencilRef( int ref )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyStencilMask( int mask )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyStencilCompareFunc( DRG_COMPARE_FUNC func )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyCullMode( DRG_CULL_MODE mode )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyFrontFace( DRG_WINDING_ORDER order )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyFillMode( DRG_FILL_MODE mode )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplySwapBuffers( GUI_Window* window )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyBindRenderTarget( drgRenderTarget* target, int index )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyUnbindRenderTarget( drgRenderTarget* target )
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

void drgRenderContext::ApplyUpdateSurfaces()
{
	drgPrintError("Not implemented, this is the platform agnostic layer");
}

#else  // EMSCRIPTEN


extern "C" void drgLibDrawRect(float x1, float y1, float x2, float y2);
extern "C" void drgLibDrawRectTex(float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2, void* tex);

drgRenderContext::drgRenderContext()
{
	m_CurrDraw = 0;
}

drgRenderContext::~drgRenderContext()
{
}

void drgRenderContext::Init()
{
}

void drgRenderContext::Begin()
{
	m_CurrVert = 0;
	m_CurrTexIdx = 0;
	m_CurrMaterial = -1;
	m_ClipTop = 0;
	m_ClipLeft = 0;
	m_ClipBot = m_DrawHeight;
	m_ClipRight = m_DrawWidth;
}

void drgRenderContext::End()
{
}

void drgRenderContext::StartDraw(unsigned char material, drgTexture* tex)
{
	m_CurrMaterial = material;
	m_CurrTexIdx = tex->m_TexIdx;
}

void drgRenderContext::EndDraw()
{
}

void drgRenderContext::ClipRect(unsigned char material, short top, short bot, short left, short right)
{
	m_ClipTop = top;
	m_ClipLeft = left;
	m_ClipBot = bot;
	m_ClipRight = right;
}

void drgRenderContext::AddRect(drgBBox* rect, drgBBox* trect, drgColor* col)
{
	drgLibDrawRect(rect->min.x, rect->min.y, rect->max.x, rect->max.y);
}

void drgRenderContext::DrawRect(unsigned char material, drgBBox* rect, drgColor* col)
{
	drgLibDrawRect(rect->min.x, rect->min.y, rect->max.x, rect->max.y);
}

void drgRenderContext::DrawRect(unsigned char material, drgBBox* rect, drgBBox* trect, drgColor* col, drgTexture* tex)
{
	drgLibDrawRectTex(rect->min.x, rect->min.y, rect->max.x, rect->max.y, trect->min.x, trect->min.y, trect->max.x, trect->max.y, (void*)tex->m_Texture);
}

void drgRenderContext::AddVert2D(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
}

void drgRenderContext::AddVert2D(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float u, float v)
{
}



void drgDrawBucket::Reset()
{
}

drgDrawBucket::drgDrawBucket()
{
}

drgDrawBucket::~drgDrawBucket()
{
}

drgDrawNode* drgDrawBucket::Add(unsigned char type, unsigned short texidx, unsigned short data, unsigned short extra, unsigned char priority)
{
	return NULL;
}

drgDrawNode* drgDrawBucket::AddClipRect(short top, short bot, short left, short right)
{
	return NULL;
}




drgDrawNode::drgDrawNode()
{
	m_Data.m_Vert.m_Data = 0;
	m_Data.m_Vert.m_Extra1 = 0;
	m_Data.m_Vert.m_Extra2 = 0;
	m_Data.m_Vert.m_TextureIdx = 0;
	m_Type = DRG_DRAW_NONE;
	m_Priority = DRG_DRAW_PRIORITY_NORMAL;
}

drgDrawNode::~drgDrawNode()
{
}

#endif  // EMSCRIPTEN
