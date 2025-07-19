
#include "system/global.h"

#if USE_OPEN_GL

#include "render/render_target.h"
#include "render/render_target_ogl.h"
#include "system/extern_inc.h"


//GLenum GetAttachment( int index, bool isDepthStencil )
//{
//	GLenum attachment = GL_COLOR_ATTACHMENT0 + index;
//	if( isDepthStencil )
//	{
//		attachment = GL_DEPTH_STENCIL_ATTACHMENT;
//	}
//
//	return attachment;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
// drgRenderTarget Functionality
////////////////////////////////////////////////////////////////////////////////////////////////////

drgRenderTarget::drgRenderTarget()
{
	m_Width = 0;
	m_Height = 0;
	m_Format = DRG_FMT_A8R8G8B8;

	m_PlatformData.m_TargetHandle = 0;
	m_IndexBoundAt = -1;
}

drgRenderTarget::~drgRenderTarget()
{
	Destroy();
}

void drgRenderTarget::Init( int width, int height, DRG_TEX_FORMAT format )
{
#if (defined(_ANDROID) || defined(_IPHONE))
	drgPrintError("This needs to be reworked to not use frame buffers");
#endif

	Destroy();

	m_Width = width;
	m_Height = height;
	m_Format = format;

	//
	// Initialize our render buffer
	//

	//GLenum platformFormat = (GLenum)GetPlatformTextureFormat( format );

	//
	// Initialize our texture
	//

	if( IsDepthStencil() == false )
	{
		// There are serious limitation for depth stencil textures. If you really need one,
		// consider the following: Use an R32F texture or Depth16 texture. But I'm not supporting that right now.

		m_Texture = drgTexture::CreateRenderTargetTexture( width, height, format );
		assert( m_Texture && "Failed to create a render target texture" );
	}
	else
	{
		//pglGenRenderbuffers( 1, &m_PlatformData.m_TargetHandle );					GL_ERROR( "Failed to generate render buffer handle" );
		//pglBindRenderbuffer( GL_RENDERBUFFER, m_PlatformData.m_TargetHandle );		GL_ERROR( "Failed to bind the render buffer" );
		//pglRenderbufferStorage( GL_RENDERBUFFER, platformFormat, width, height );	GL_ERROR( "Failed to set render buffer storage" );
		//pglBindRenderbuffer( GL_RENDERBUFFER, 0 );									GL_ERROR( "Failed to bind the default back buffer" );
	}
}

void drgRenderTarget::Destroy()
{
	if( m_PlatformData.m_TargetHandle > 0 )
	{
		//pglDeleteRenderbuffers( 1, &m_PlatformData.m_TargetHandle );				GL_ERROR( "Failed to delete a render buffer" );
		m_PlatformData.m_TargetHandle = 0;
	}

	if( m_Texture )
	{
		m_Texture->CleanUp();
	}
}

void drgRenderTarget::Bind( int index )
{	

	//
	// Make sure we bind our texture as well.
	// TODO: We need a way to make sure that our texture isn't being read from while we write to it. That's bad.
	//

	//GLenum attachment = GetAttachment( index, IsDepthStencil() );

	if( m_Texture )
	{
		//glBindTexture( GL_TEXTURE_2D, m_Texture->m_Texture );	GL_ERROR( "Failed to bind the texture" );
		//pglFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Texture->m_Texture, 0 );
		//GL_ERROR( "Failed to set the render target as a texture" );
	}
	else
	{
		//
		// Attach to the currently bound frame buffer object.
		// TODO: Need some way to ensure there is an FBO bound...
		//

		//pglBindRenderbuffer( GL_RENDERBUFFER, m_PlatformData.m_TargetHandle );	GL_ERROR( "Failed to bind the renderbuffer" );
		//pglFramebufferRenderbuffer( GL_FRAMEBUFFER,
		//	attachment,
		//	GL_RENDERBUFFER,
		//	m_PlatformData.m_TargetHandle );
		//GL_ERROR( "Failed to attach the render buffer to the frame buffer" );

	}

	//
	// Setup our cache flags.
	//

	m_IndexBoundAt = index;
}

void drgRenderTarget::Unbind()
{
	//
	// Make sure we are currently bound.
	//

	if( m_IndexBoundAt >= 0 )
	{
		//GLenum attachment = GetAttachment( m_IndexBoundAt, IsDepthStencil() );

		if( m_Texture )
		{
			//pglFramebufferTexture( GL_FRAMEBUFFER, attachment, 0, 0 );
			//GL_ERROR( "Failed to unbind render target texture" );
		}
		else
		{
			//pglFramebufferRenderbuffer( GL_FRAMEBUFFER,
			//	attachment,
			//	GL_RENDERBUFFER,
			//	0 );
			//GL_ERROR( "Failed to unbind render target" );
			//pglBindRenderbuffer( GL_RENDERBUFFER, 0 );
		}
		
		m_IndexBoundAt = -1;
	}
}

#endif  // USE_OPEN_GL

