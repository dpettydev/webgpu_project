
#include "system/global.h"

#if USE_OPEN_GL

#include "render/context_ogl.h"
#include "render/material.h"
#include "system/extern_inc.h"
#include "render/vertexbuff.h"
#include "render/material_manager.h"
#include "render/render_target.h"
#include "util/stringn.h"

// Helper Functions
void HandleGLError( GLenum error, const char* message )
{
	char text[ 128 ];
	int length = drgString::Length( message );
	drgString::CopySafe( text, message, DRG_MIN( length, 128 ) );
	text[ length ] = '\0';

	drgDebug::Print( text );
	drgDebug::Print( "\n" );

	switch( error )
	{
	case GL_INVALID_ENUM:
		drgDebug::Error( "OpenGL has returned the error: GL_INVALID_ENUM. Please refer to the documentation for why the previous OGL function would return this error\r\n" );
		break;
	case GL_INVALID_VALUE:
		drgDebug::Error( "OpenGL has returned the error: GL_INVALID_VALUE. Please refer to the documentation for why the previous OGL function would return this error\r\n" );
		break;
	case GL_INVALID_OPERATION:
		drgDebug::Error( "OpenGL has returned the error: GL_INVALID_OPERATION. Please refer to the documentation for why the previous OGL function would return this error\r\n" );
		break;
	case GL_OUT_OF_MEMORY:
		drgDebug::Error( "OpenGL has returned the error: GL_OUT_OF_MEMORY. Please refer to the documentation for why the previous OGL function would return this error\r\n" );
		break;
#if !defined(USE_GLES)
	case GL_STACK_UNDERFLOW:
		drgDebug::Error( "OpenGL has returned the error: GL_STACK_UNDERFLOW. Please refer to the documentation for why the previous OGL function would return this error\r\n" );
		break;
	case GL_STACK_OVERFLOW:
		drgDebug::Error( "OpenGL has returned the error: GL_STACK_OVERFLOW. Please refer to the documentation for why the previous OGL function would return this error\r\n" );
		break;
#endif
	default:
		drgDebug::Error( "OpenGL has returned an unknown error %d. Please refer to the documentation for why the previous OGL function would return this error\r\n", (int)error );		
		break;
	}
}

GLenum GetPlatformColorBlend( DRG_BLEND_FUNCTION func )
{
	switch( func )
	{
		case DRG_BLEND_FUNC_SOURCE:
			return GL_SRC_COLOR;
		case DRG_BLEND_FUNC_DEST:
			return GL_DST_COLOR;
		case DRG_BLEND_FUNC_INV_SOURCE:
			return GL_ONE_MINUS_SRC_COLOR;
		case DRG_BLEND_FUNC_INV_DEST:
			return GL_ONE_MINUS_DST_COLOR;
		case DRG_BLEND_FUNC_ONE:
			return GL_ONE;
		case DRG_BLEND_FUNC_ZERO:
			return GL_ZERO;
		default:
			assert( "Tried to get an unknown color blend type" && 0 );
			return GL_SRC_COLOR;
	}
}

GLenum GetPlatformAlphaBlend( DRG_BLEND_FUNCTION func )
{
	switch( func )
	{
		case DRG_BLEND_FUNC_SOURCE:
			return GL_SRC_ALPHA;
		case DRG_BLEND_FUNC_DEST:
			return GL_DST_ALPHA;
		case DRG_BLEND_FUNC_INV_SOURCE:
			return GL_ONE_MINUS_SRC_ALPHA;
		case DRG_BLEND_FUNC_INV_DEST:
			return GL_ONE_MINUS_DST_ALPHA;
		case DRG_BLEND_FUNC_ONE:
			return GL_ONE;
		case DRG_BLEND_FUNC_ZERO:
			return GL_ZERO;
		default:
			assert( "Tried to get an unknown alpha blend type" && 0 );
			return GL_SRC_ALPHA;
	}
}

GLenum GetPlatformCompareFunc( DRG_COMPARE_FUNC func )
{
	switch( func )
	{
		case DRG_COMPARE_LESS:
			return GL_LESS;
		case DRG_COMPARE_LEQUAL:
			return GL_LEQUAL;
		case DRG_COMPARE_GREATER:
			return GL_GREATER;
		case DRG_COMPARE_GEQUAL:
			return GL_GEQUAL;
		case DRG_COMPARE_EQUAL:
			return GL_EQUAL;
		case DRG_COMPARE_NOTEQUAL:
			return GL_NOTEQUAL;
		case DRG_COMPARE_ALWAYS:
			return GL_ALWAYS;
		case DRG_COMPARE_NEVER:
			return GL_NEVER;
		default:
			assert( "Provided an unknown comparison function type" && 0 );
			return GL_LESS;
	}
}

GLenum GetPlatformCullMode( DRG_CULL_MODE mode )
{
	switch( mode )
	{
		case DRG_CULL_FRONT:
			return GL_FRONT;
		case DRG_CULL_BACK:
			return GL_BACK;
		case DRG_CULL_NONE:
			return GL_NONE;
		default:
			assert( "Provided an unknown cull mode" && 0 );
			return GL_NONE;
	}
}

#if !defined(USE_GLES)
GLenum GetPlatformFillMode( DRG_FILL_MODE mode )
{
	switch( mode )
	{
		case DRG_FILL_POLYGON:
			return GL_FILL;
		case DRG_FILL_WIREFRAME:
			return GL_LINE;
		default:
			assert( "Provided an unknown fill mode" && 0 );
			return GL_FILL;
	}
}
#endif

GLenum GetPlatformWindingOrder( DRG_WINDING_ORDER order )
{
	switch( order )
	{
		case DRG_WINDING_CLOCKWISE:
			return GL_CW;
		case DRG_WINDING_COUNTER_CLOCKWISE:
			return GL_CCW;
		default:
			assert( "Provided an unknown winding order" && 0 );
			return GL_CW;
	}
}

unsigned int GetPlatformTextureFormat( DRG_TEX_FORMAT format )
{
	switch( format )
	{
		case DRG_FMT_A8R8G8B8:
			return GL_RGBA8;
		case DRG_FMT_R8G8B8:
			return GL_RGB8;
		case DRG_FMT_D24S8:
			return GL_DEPTH24_STENCIL8;
		default:
			assert( "Provided an unknown texture format" && 0 );
			return GL_RGBA8;
	}
}

void CheckForCompleteness()
{
    GLenum result = glCheckFramebufferStatus( GL_FRAMEBUFFER );	GL_ERROR( "Failed to check frame buffer status" );
	if( result != GL_FRAMEBUFFER_COMPLETE )
	{
		switch( result )
		{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				assert( "Framebuffer has an incomplete attachment" && 0 );
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				assert( "Framebuffer is missing an attachment, are any render targets bound?" && 0 );
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				assert( "Framebuffer incomplete: One or more render buffers are set to attachment GL_NONE" && 0 );
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				assert( "Framebuffer incomplete: One or more read buffer are set to attacment GL_NONE" && 0 );
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				assert( "Framebuffer unsupported: The frame buffer has one or more render buffers attached with an unsupported format" && 0 );
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				assert( "Framebuffer incomplete: The frame buffer currently has one more more render buffers with incompatible multisample formats, or there are a combination of textures and render buffers attached" && 0 );
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				assert( "Framebuffer incomplete: The frame buffer currently has improperly formated layer targets" && 0 );
				break;
			default:
				assert( "Framebuffer incomplete: Unknown error!" && 0 );
				break;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// drgRenderContext_OGL
////////////////////////////////////////////////////////////////////////////////////////////////////

drgRenderContext_OGL::drgRenderContext_OGL()
{
	m_Platform = DRG_RENDER_PLATFORM_OPENGL;
	//m_RenderCache->SetupCache();

	for( int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget )
	{
		m_BoundRenderTargets[ currentTarget ] = NULL;
	}

	m_CurrentFrameBufferObject = 0;
	m_FrameBufferObject = -1;
	m_BoundDepthTarget = NULL;
}

drgRenderContext_OGL::~drgRenderContext_OGL()
{
	drgEngine::NativeContextSetMain();
}

#if defined(_JS_2D)
void drgRenderContext_OGL::Init()
{
}

void drgRenderContext_OGL::Begin()
{
	drgRenderContext::Begin();
}

void drgRenderContext_OGL::End()
{
	int itridx = -1;
	drgDrawNode* curnode;

	drgRenderContext::End();

	for(int bucket=0; bucket<DRG_MAX_NUM_MATERIALS; ++bucket)
	{
		itridx = m_DrawBuckets[m_CurrDraw][bucket].GetFirst();
		while(itridx >= 0)
		{
			curnode = &drgRenderContext::m_DrawNodes[itridx];
			if(curnode->m_Type == DRG_DRAW_VERT_LIST)
			{
			}
			else if(curnode->m_Type == DRG_DRAW_VERT_LIST_TEX)
			{
			}
			else if(curnode->m_Type == DRG_DRAW_CLIP_RECT)
			{
			}
			itridx = curnode->m_Next;
			curnode->m_Next = -1;
		}
		drgRenderContext::m_DrawBuckets[m_CurrDraw][bucket].Reset();
	}
	m_CurrDraw = !m_CurrDraw;
}

#elif USE_OPEN_GL
extern GLuint g_Shader;
void drgRenderContext_OGL::Init()
{
	//drgEngine::NativeContextSetMain();

	int width, height;
	drgEngine::GetScreenSize( &width, &height );

	drgPrintOut( "We got the screen size: %d, %d\r\n", width, height );

	//
	// Bind our FBO
	//
#if !(defined(_ANDROID) || defined(_IPHONE))
	glGenFramebuffers( 1, &m_FrameBufferObject );												GL_ERROR( "Failed to generate the frame buffer object" );
	glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferObject );									GL_ERROR( "Failed to bind the framebuffer object" );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0 );		GL_ERROR( "Failed to bind the default back buffer to the framebuffer object" );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
#endif
	m_CurrentFrameBufferObject = 0;
}

void drgRenderContext_OGL::Begin()
{
	// Bind the back buffer
#if !(defined(_ANDROID) || defined(_IPHONE))
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );	GL_ERROR( "Failed to bind the default frame buffer object" );
	m_CurrentFrameBufferObject = 0;
#endif

	drgRenderContext::Begin();
}

#include "system/extern_inc.h"
void drgRenderContext_OGL::End()
{
}

void drgRenderContext_OGL::SetWorldViewProjectionMatrix( drgMat44* world, drgMat44* viewProjection )
{
	if(viewProjection)
	{
		if(world)
		{
			m_WorldViewProjectionMatrix = (*viewProjection) * (*world);
		}
		else
			m_WorldViewProjectionMatrix = (*viewProjection);
		//m_WorldViewProjectionMatrix.Transpose();
	}
	else
	{
		m_WorldViewProjectionMatrix = (*world);
	}
}

void drgRenderContext_OGL::SetAmbientColor( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha )
{
	m_AmbientColor = drgColor( red, green, blue, alpha );
}

void drgRenderContext_OGL::SetBoneMatrix( drgMat44* matrix, int index )
{
	assert( index >= 0 && index < NUM_BONE_MATRICES );

	drgMat44 mat = *matrix;
	m_BoneMatrices[ index ] = mat;
}

void drgRenderContext_OGL::SetWorldMatrix( drgMat44* world )
{
	m_WorldMatrix = *world;
}

void drgRenderContext_OGL::SetSpecular( drgColor* spec )
{
	m_SpecularColor = *spec;
}

void drgRenderContext_OGL::SetDiffuseColor( drgColor* color )
{
	m_DiffuseColor = *color;
}

void drgRenderContext_OGL::SetTexCoordOffset( drgVec4* offset )
{
	m_TexCoordOffset = *offset;
}

void drgRenderContext_OGL::SetViewPosition( drgVec3* position )
{
	m_ViewPosition = *position;
}

void drgRenderContext_OGL::SetViewDirection( drgVec3* direction )
{
	m_ViewDirection = *direction;
}

void drgRenderContext_OGL::RenderPrimitive( unsigned int renderFlags, int primType, int numPrims, int numVertsTot, void* data, int vertSize, void* indices, bool setStream, int startVert, void* startData,  int numVerts, int streamOffset )
{
	numVerts = numPrims;
	switch( primType )
	{
		case PRIM_LINELIST:
			startVert *= 2;
			numVerts *= 2;
			break;
		case PRIM_LINESTRIP:
			numVerts += 1;
			break;
		case PRIM_TRIANGLELIST:
			startVert *= 3;
			numVerts *= 3;
			break;
		case PRIM_TRIANGLESTRIP:
		case PRIM_TRIANGLEFAN:
			numVerts += 2;
			break;
		case PRIM_POINTLIST:
		default:
			break;
	}	
	
	if( indices != NULL)		// this is an indexed array
	{
		glDrawElements( primType, numVerts, GL_UNSIGNED_SHORT, 0 );	GL_ERROR( "Failed to draw an indexed mesh" );
	}
	else
	{
		glDrawArrays( primType, startVert, numVerts );				GL_ERROR( "Failed to draw a mesh" );
	}
}

void drgRenderContext_OGL::ApplyMakeCurrent( void* renderContext, void* deviceContext )
{
// #if defined(_WINDOWS)
// 	wglMakeCurrent( (HDC)deviceContext, (HGLRC)renderContext );		assert( GetLastError() == 0 );
// #endif
}

void drgRenderContext_OGL::ApplySwapBuffers( GUI_Window* window )
{
	drgEngine::NativeWindowEnd( window );	// There are different swaps for different OGL versions. 
}

void drgRenderContext_OGL::ApplyClearTarget( unsigned int clearFlags, drgColor color, float depth, unsigned int stencil )
{
	GLbitfield flags = 0;
	if( clearFlags & DRG_CLEAR_COLOR )
	{
		glClearColor( (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f );
		GL_ERROR( "Failed to set the clear color" );

		flags |= GL_COLOR_BUFFER_BIT;
	}
	
	if( clearFlags & DRG_CLEAR_DEPTH )
	{
		glClearDepth( depth );
		GL_ERROR( "Failed to set the depth clear value" );
		flags |= GL_DEPTH_BUFFER_BIT;
	}
	
	if( clearFlags & DRG_CLEAR_STENCIL )
	{
		glClearStencil( stencil );
		GL_ERROR( "Failed to set the stencil clear value" );
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear( flags ); GL_ERROR( "Failed to clear" );
}

void drgRenderContext_OGL::ApplyScissorEnable( bool enable )
{
	if( enable )
	{
		glEnable( GL_SCISSOR_TEST );			GL_ERROR( "Failed to enable scissor testing" );
	}
	else
	{
		glDisable( GL_SCISSOR_TEST );			GL_ERROR( "Failed to disable scissor testing" );
	}
}

void drgRenderContext_OGL::ApplyClipRect( short top, short bottom, short left, short right )
{	
	glScissor( left, (short)m_DrawHeight - bottom, right - left, bottom - top );		GL_ERROR( "Failed to apply the clip rect" );
}

void drgRenderContext_OGL::ApplyScissorState( bool enable, short top, short bottom, short left, short right )
{
	ApplyScissorEnable( enable );
	ApplyClipRect( top, bottom, left, right );
}

void drgRenderContext_OGL::ApplyViewport( short top, short bottom, short left, short right )
{
	//
	// This is a bit of hack for coordinate spaces. We can change this around if we want to.
	// But basically we assume that the screen orgin is at the top left corner of the screen.
	// But OpenGL assumes it's in the lower left corner of the screen. So we need to move our Y accordingly.
	//

	top = (short)m_DrawHeight - top - bottom;

	glViewport( left, top, right, bottom );		GL_ERROR( "Failed to apply the viewport" );
}

void drgRenderContext_OGL::ApplyColorBlend( bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op )
{
	if( enable )
	{

		GLenum sourceBlend = GetPlatformColorBlend( source );
		GLenum destBlend = GetPlatformColorBlend( dest );
	
		// Blend Operations are not available on low end specs, so I'm ignoring them here until we have a better
		// way of determining the capabilities of a platform. We can always add it in later.
		if( glIsEnabled( GL_BLEND ) == GL_FALSE )
		{
			glEnable( GL_BLEND );				GL_ERROR( "Failed to enable color blending" );
		}

		glBlendFunc( sourceBlend, destBlend );	GL_ERROR( "Failed to set the color blending" );
	}
	else
	{
		glDisable( GL_BLEND );					GL_ERROR( "Failed to disable color blending" );
	}
}

void drgRenderContext_OGL::ApplyAlphaBlend( bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op )
{
	if( enable )
	{

		GLenum sourceBlend = GetPlatformAlphaBlend( source );
		GLenum destBlend = GetPlatformAlphaBlend( dest );
	
		// Blend Operations are not available on low end specs, so I'm ignoring them here until we have a better
		// way of determining the capabilities of a platform. We can always add it in later.
		if( glIsEnabled( GL_BLEND ) == GL_FALSE )
		{
			glEnable( GL_BLEND );				GL_ERROR( "Failed to enable alpha blending" );
		}

		glBlendFunc( sourceBlend, destBlend );	GL_ERROR( "Failed to set alpha blending" );
	}
	else
	{
		glDisable( GL_BLEND );					GL_ERROR( "Failed to disable alpha blending" );
	}
}

void drgRenderContext_OGL::ApplyColorBlendEnable( bool enable )
{
	if( enable )
	{
		assert( "Color Blending is not supported currently. OpenGL only allows one blend mode at a time. Need to cache this correctly.");
		if( glIsEnabled( GL_BLEND ) == GL_FALSE )
		{
			glEnable( GL_BLEND );				GL_ERROR( "Failed to enable color blending" );
		}
	}
	else
	{
		glDisable( GL_BLEND );					GL_ERROR( "Failed to disable color blending" );
	}
}

void drgRenderContext_OGL::ApplyColorSourceBlend( DRG_BLEND_FUNCTION func )
{
#if defined(USE_GLES)
	GLenum oldDest = GL_SRC_COLOR;
#else
	GLenum oldDest = GL_ZERO;
	glGetIntegerv( GL_BLEND_DST, (GLint*)&oldDest );			GL_ERROR( "Failed to get the dest color blend" );
#endif
	glBlendFunc( GetPlatformColorBlend( func ), oldDest );		GL_ERROR( "Failed to set the src color blend" );
}

void drgRenderContext_OGL::ApplyColorDestBlend( DRG_BLEND_FUNCTION func )
{
#if defined(USE_GLES)
	GLenum oldSrc = GL_DST_COLOR;
#else
	GLenum oldSrc = GL_ONE;
	glGetIntegerv( GL_BLEND_SRC, (GLint*)&oldSrc );				GL_ERROR( "Failed to get the previous src color blend" );
#endif
	glBlendFunc( oldSrc, GetPlatformColorBlend( func ) );		GL_ERROR( "Failed to set the dest color blend" );
}

void drgRenderContext_OGL::ApplyColorOperation( DRG_BLEND_OPERATION op )
{
	// This isn't supported on low end platforms
}

void drgRenderContext_OGL::ApplyAlphaBlendEnable( bool enable )
{
	ApplyColorBlendEnable( enable );	// Same code as color blend
}

void drgRenderContext_OGL::ApplyAlphaSourceBlend( DRG_BLEND_FUNCTION func )
{
#if defined(USE_GLES)
	GLenum oldDest = GL_DST_ALPHA;
#else
	GLint oldDest = GL_ONE_MINUS_SRC_ALPHA;
	glGetIntegerv( GL_BLEND_DST, &oldDest );			GL_ERROR( "Failed to get the dest alpha blend" );
#endif
	GLenum newSrc = GetPlatformAlphaBlend( func );
	glBlendFunc( newSrc, oldDest );						GL_ERROR( "Failed to set the src alpha blend" );
}

void drgRenderContext_OGL::ApplyAlphaDestBlend( DRG_BLEND_FUNCTION func )
{
#if defined(USE_GLES)
	GLenum oldSrc = GL_SRC_ALPHA;
#else
	GLenum oldSrc = GL_SRC_ALPHA;
	glGetIntegerv( GL_BLEND_SRC, (GLint*)&oldSrc );				GL_ERROR( "Failed to get the previous src alpha blend" );
#endif
	GLenum newDst = GetPlatformAlphaBlend( func );
	glBlendFunc( oldSrc, newDst );								GL_ERROR( "Failed to set the dest alpha blend" );
}

void drgRenderContext_OGL::ApplyAlphaOperation( DRG_BLEND_OPERATION op )
{
	// Currently not supported
}

void drgRenderContext_OGL::ApplyDepthState( bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func )
{
	ApplyDepthTestEnable( enableTest );
	ApplyDepthWriteEnable( enableWrite );
	ApplyDepthCompareFunc( func );
}

void drgRenderContext_OGL::ApplyStencilState( bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask )
{
	ApplyStencilTestEnable( enableTest );
	ApplyStencilWriteEnable( enableWrite );
	ApplyStencilCompareFunc( func );
	ApplyStencilRef( ref );
	ApplyStencilMask( mask );
}

void drgRenderContext_OGL::ApplyDepthTestEnable( bool enable )
{
	if( enable )
	{
		glEnable( GL_DEPTH_TEST );		GL_ERROR( "Failed to enable depth testing" );
	}
	else
	{
		glDisable( GL_DEPTH_TEST );		GL_ERROR( "Failed to disable depth testing" );
	}
}

void drgRenderContext_OGL::ApplyDepthWriteEnable( bool enable )
{
	if( enable )
	{
		glDepthMask( GL_TRUE );		GL_ERROR( "Failed to enable depth testing" );
	}
	else
	{
		glDepthMask( GL_FALSE );	GL_ERROR( "Failed to disable depth testing" );	// Remember that this must be enabled to clear the depth buffer properly.
	}
}

void drgRenderContext_OGL::ApplyDepthCompareFunc( DRG_COMPARE_FUNC func )
{
	glDepthFunc( GetPlatformCompareFunc( func ) );		GL_ERROR( "Failed to set the depth comparison function" );
}

void drgRenderContext_OGL::ApplyStencilTestEnable( bool enable )
{
	if( enable )
	{
		glEnable( GL_STENCIL_TEST );		GL_ERROR( "Failed to enable stencil testing" );
	}
	else
	{
		glDisable( GL_STENCIL_TEST );		GL_ERROR( "Failed to disable stencil testing" );
	}
}

void drgRenderContext_OGL::ApplyStencilWriteEnable( bool enable )
{
	// Always writes.
}

void drgRenderContext_OGL::ApplyStencilCompareFunc( DRG_COMPARE_FUNC func )
{
	int stencilRef, stencilMask;
	glGetIntegerv( GL_STENCIL_REF, &stencilRef );				GL_ERROR( "Failed to get the stencil ref" );
	glGetIntegerv( GL_STENCIL_VALUE_MASK, &stencilMask );		GL_ERROR( "Failed to get the stencil mask" );

	glStencilFunc( GetPlatformCompareFunc( func ), stencilRef, stencilMask );	GL_ERROR( "Failed to set the stencil comparison function" );
}

void drgRenderContext_OGL::ApplyStencilRef( int ref )
{
	int stencilMask;
	GLenum stencilFunc;

	glGetIntegerv( GL_STENCIL_VALUE_MASK, &stencilMask );		GL_ERROR( "Failed to get the previous stencil mask" );
	glGetIntegerv( GL_STENCIL_FUNC, (int*)&stencilFunc );		GL_ERROR( "Failed to get the previous stencil func" );

	glStencilFunc( stencilFunc, ref, stencilMask );				GL_ERROR( "Failed to set the stencil ref" );
}

void drgRenderContext_OGL::ApplyStencilMask( int mask )
{
	int stencilRef;
	GLenum stencilFunc;

	glGetIntegerv( GL_STENCIL_REF, &stencilRef );				GL_ERROR( "Failed to get the previous stencil ref" );
	glGetIntegerv( GL_STENCIL_FUNC, (int*)&stencilFunc );		GL_ERROR( "Failed to get the previous stencil function" );

	glStencilFunc( stencilFunc, stencilRef, mask );				GL_ERROR( "Failed to set the stencil mask" );
}

void drgRenderContext_OGL::ApplyCullMode( DRG_CULL_MODE mode )
{
	GLenum cull = GetPlatformCullMode( mode );
	
	// A bit hacky, but if we passed in DRG_CULL_NONE, we need to disable culling
	if( cull == GL_NONE )
	{
		glDisable( GL_CULL_FACE );	GL_ERROR( "Failed to disable culling" );
	}
	else
	{
		// Make sure we enable it here if it's disabled
		if( glIsEnabled( GL_CULL_FACE ) == GL_FALSE )
		{
			glEnable( GL_CULL_FACE );	GL_ERROR( "Failed to enable culling" );
		}

		glCullFace( cull );	GL_ERROR( "Failed to set the cull face" );
	}
}

void drgRenderContext_OGL::ApplyFrontFace( DRG_WINDING_ORDER order )
{
	GLenum face = GetPlatformWindingOrder( order );
	glFrontFace( face );
}

void drgRenderContext_OGL::ApplyFillMode( DRG_FILL_MODE mode )
{
#if !defined(USE_GLES)
	GLenum fill = GetPlatformFillMode( mode );
	glPolygonMode( GL_FRONT_AND_BACK, fill );	GL_ERROR( "Failed to set the polygon mode" );
#endif
}

void drgRenderContext_OGL::ApplyBindRenderTarget( drgRenderTarget* target, int index )
{
	if( index >= 0 )
	{
		m_BoundRenderTargets[ index ] = target;
	}
	else
	{
		m_BoundDepthTarget = target;
	}
}

void drgRenderContext_OGL::ApplyUnbindRenderTarget( drgRenderTarget* target )
{
	for( int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget )
	{
		if( m_BoundRenderTargets[ currentTarget ] == target )
		{
			target->Unbind();
			m_BoundRenderTargets[ currentTarget ] = NULL;
			break;
		}
	}
}

void drgRenderContext_OGL::ApplyUpdateSurfaces()
{
#if (defined(_ANDROID) || defined(_IPHONE))
	drgPrintError( "This needs to be revisited" );
#endif
	if( m_BoundRenderTargets[ 0 ] == NULL )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );	GL_ERROR( "Failed to bind default frame buffer" );
		m_CurrentFrameBufferObject = 0;
	}
	else
	{
		// First we bind our context frame buffer if necessary.
		if( m_CurrentFrameBufferObject != m_FrameBufferObject )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferObject ); GL_ERROR( "Failed to bind context frame buffer" );
			m_CurrentFrameBufferObject = m_FrameBufferObject;
		}

		// Now we attach our render buffers to the framebuffer
		int numDrawBuffers = 0;
		GLenum drawBuffers[ DRG_MAX_RENDER_TARGETS ] = {GL_COLOR_ATTACHMENT0};
		for( int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget )
		{
			if( m_BoundRenderTargets[ currentTarget ] )
			{
				m_BoundRenderTargets[ currentTarget ]->Bind( currentTarget );
				drawBuffers[ currentTarget ] = GL_COLOR_ATTACHMENT0 + currentTarget;
				numDrawBuffers++;
			}
		}

		// Now we need to specify the draw buffers, (targets whose textures can be written to)
		glDrawBuffers( numDrawBuffers, drawBuffers );	GL_ERROR( "Failed to specify draw buffers" );

		// Bind the depth target if applicable.
		if( m_BoundDepthTarget )
		{
			m_BoundDepthTarget->Bind( -1 );
			//m_BoundDepthTarget->Unbind();
		}

		// Now we need to make sure we are complete.
		CheckForCompleteness();
	}
}

#else


void drgRenderContext_OGL::Init()
{
	//drgEngine::NativeContextSetMain();

	int width, height;
	drgEngine::GetScreenSize(&width, &height);

	drgPrintOut("We got the screen size: %d, %d\r\n", width, height);
	m_CurrentFrameBufferObject = 0;
}

void drgRenderContext_OGL::Begin()
{
	//
	// Bind the back buffer
	//
	m_CurrentFrameBufferObject = 0;

	drgRenderContext::Begin();
}

#include "system/wininc.h"
void drgRenderContext_OGL::End()
{
}

void drgRenderContext_OGL::SetWorldViewProjectionMatrix(drgMat44* world, drgMat44* viewProjection)
{
	if (viewProjection)
	{
		if (world)
		{
			m_WorldViewProjectionMatrix = (*viewProjection) * (*world);
		}
		else
			m_WorldViewProjectionMatrix = (*viewProjection);
		m_WorldViewProjectionMatrix.Transpose();
	}
	else
	{
		m_WorldViewProjectionMatrix = (*world);
	}
}

void drgRenderContext_OGL::SetAmbientColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	m_AmbientColor = drgColor(red, green, blue, alpha);
}

void drgRenderContext_OGL::SetBoneMatrix(drgMat44* matrix, int index)
{
	assert(index >= 0 && index < NUM_BONE_MATRICES);

	drgMat44 mat = *matrix;
	m_BoneMatrices[index] = mat;
}

void drgRenderContext_OGL::SetWorldMatrix(drgMat44* world)
{
	m_WorldMatrix = *world;
}

void drgRenderContext_OGL::SetSpecular(drgColor* spec)
{
	m_SpecularColor = *spec;
}

void drgRenderContext_OGL::SetDiffuseColor(drgColor* color)
{
	m_DiffuseColor = *color;
}

void drgRenderContext_OGL::SetTexCoordOffset(drgVec4* offset)
{
	m_TexCoordOffset = *offset;
}

void drgRenderContext_OGL::SetViewPosition(drgVec3* position)
{
	m_ViewPosition = *position;
}

void drgRenderContext_OGL::SetViewDirection(drgVec3* direction)
{
	m_ViewDirection = *direction;
}

void drgRenderContext_OGL::RenderPrimitive(unsigned int renderFlags,
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
	int streamOffset)
{
	numVerts = numPrims;
	switch (primType)
	{
	case PRIM_LINELIST:
		startVert *= 2;
		numVerts *= 2;
		break;
	case PRIM_LINESTRIP:
		numVerts += 1;
		break;
	case PRIM_TRIANGLELIST:
		startVert *= 3;
		numVerts *= 3;
		break;
	case PRIM_TRIANGLESTRIP:
	case PRIM_TRIANGLEFAN:
		numVerts += 2;
		break;
	case PRIM_POINTLIST:
	default:
		break;
	}

	if (indices != NULL)		// this is an indexed array
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyMakeCurrent(void* renderContext, void* deviceContext)
{
// #if defined(_WINDOWS)
// #if USE_OPEN_GL
// 	wglMakeCurrent((HDC)deviceContext, (HGLRC)renderContext);		assert(GetLastError() == 0);
// #endif
// #endif
}

void drgRenderContext_OGL::ApplySwapBuffers(GUI_Window* window)
{
	drgEngine::NativeWindowEnd(window);	// There are different swaps for different OGL versions. 
}

void drgRenderContext_OGL::ApplyClearTarget(unsigned int clearFlags, drgColor color, float depth, unsigned int stencil)
{
	GLbitfield flags = 0;
	if (clearFlags & DRG_CLEAR_COLOR)
	{
		//glClearColor((float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f);

		flags |= GL_COLOR_BUFFER_BIT;
	}

	if (clearFlags & DRG_CLEAR_DEPTH)
	{
		//glClearDepth(depth);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (clearFlags & DRG_CLEAR_STENCIL)
	{
		//glClearStencil(stencil);
		flags |= GL_STENCIL_BUFFER_BIT;
	}
}

void drgRenderContext_OGL::ApplyScissorEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyClipRect(short top, short bottom, short left, short right)
{
}

void drgRenderContext_OGL::ApplyScissorState(bool enable, short top, short bottom, short left, short right)
{
	ApplyScissorEnable(enable);
	ApplyClipRect(top, bottom, left, right);
}

void drgRenderContext_OGL::ApplyViewport(short top, short bottom, short left, short right)
{
	//
	// This is a bit of hack for coordinate spaces. We can change this around if we want to.
	// But basically we assume that the screen orgin is at the top left corner of the screen.
	// But OpenGL assumes it's in the lower left corner of the screen. So we need to move our Y accordingly.
	//

	top = (short)m_DrawHeight - top - bottom;
}

void drgRenderContext_OGL::ApplyColorBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyAlphaBlend(bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyColorBlendEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyColorSourceBlend(DRG_BLEND_FUNCTION func)
{
#if defined(USE_GLES)
	GLenum oldDest = GL_SRC_COLOR;
#else
	GLenum oldDest = GL_ZERO;
#endif
}

void drgRenderContext_OGL::ApplyColorDestBlend(DRG_BLEND_FUNCTION func)
{
#if defined(USE_GLES)
	GLenum oldSrc = GL_DST_COLOR;
#else
	GLenum oldSrc = GL_ONE;
#endif
}

void drgRenderContext_OGL::ApplyColorOperation(DRG_BLEND_OPERATION op)
{
	//
	// This isn't supported on low end platforms
	//
}

void drgRenderContext_OGL::ApplyAlphaBlendEnable(bool enable)
{
	ApplyColorBlendEnable(enable);	// Same code as color blend
}

void drgRenderContext_OGL::ApplyAlphaSourceBlend(DRG_BLEND_FUNCTION func)
{
}

void drgRenderContext_OGL::ApplyAlphaDestBlend(DRG_BLEND_FUNCTION func)
{
}

void drgRenderContext_OGL::ApplyAlphaOperation(DRG_BLEND_OPERATION op)
{
	//
	// Currently not supported
	//
}

void drgRenderContext_OGL::ApplyDepthState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func)
{
	ApplyDepthTestEnable(enableTest);
	ApplyDepthWriteEnable(enableWrite);
	ApplyDepthCompareFunc(func);
}

void drgRenderContext_OGL::ApplyStencilState(bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask)
{
	ApplyStencilTestEnable(enableTest);
	ApplyStencilWriteEnable(enableWrite);
	ApplyStencilCompareFunc(func);
	ApplyStencilRef(ref);
	ApplyStencilMask(mask);
}

void drgRenderContext_OGL::ApplyDepthTestEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyDepthWriteEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyDepthCompareFunc(DRG_COMPARE_FUNC func)
{
}

void drgRenderContext_OGL::ApplyStencilTestEnable(bool enable)
{
	if (enable)
	{
	}
	else
	{
	}
}

void drgRenderContext_OGL::ApplyStencilWriteEnable(bool enable)
{
	// Always writes.
}

void drgRenderContext_OGL::ApplyStencilCompareFunc(DRG_COMPARE_FUNC func)
{
}

void drgRenderContext_OGL::ApplyStencilRef(int ref)
{
}

void drgRenderContext_OGL::ApplyStencilMask(int mask)
{
}

void drgRenderContext_OGL::ApplyCullMode(DRG_CULL_MODE mode)
{
}

void drgRenderContext_OGL::ApplyFrontFace(DRG_WINDING_ORDER order)
{
}

void drgRenderContext_OGL::ApplyFillMode(DRG_FILL_MODE mode)
{
}

void drgRenderContext_OGL::ApplyBindRenderTarget(drgRenderTarget* target, int index)
{
	if (index >= 0)
	{
		m_BoundRenderTargets[index] = target;
	}
	else
	{
		m_BoundDepthTarget = target;
	}
}

void drgRenderContext_OGL::ApplyUnbindRenderTarget(drgRenderTarget* target)
{
	for (int currentTarget = 0; currentTarget < DRG_MAX_RENDER_TARGETS; ++currentTarget)
	{
		if (m_BoundRenderTargets[currentTarget] == target)
		{
			target->Unbind();
			m_BoundRenderTargets[currentTarget] = NULL;
			break;
		}
	}
}

void drgRenderContext_OGL::ApplyUpdateSurfaces()
{
#if (defined(_ANDROID) || defined(_IPHONE))
	drgPrintError("This needs to be revisited");
#endif
	if (m_BoundRenderTargets[0] == NULL)
	{
		m_CurrentFrameBufferObject = 0;
	}
	else
	{
		//
		// First we bind our context frame buffer if necessary.
		//

		if (m_CurrentFrameBufferObject != m_FrameBufferObject)
		{
			m_CurrentFrameBufferObject = m_FrameBufferObject;
		}

		if (m_BoundDepthTarget)
		{
			m_BoundDepthTarget->Bind(-1);
			//m_BoundDepthTarget->Unbind();
		}
	}
}

#endif

#endif  // USE_OPEN_GL
