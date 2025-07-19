#ifndef _DRG_RENDER_TARGET_H_
#define _DRG_RENDER_TARGET_H_


#include "render/texture.h"
#include "render/render_target_ogl.h"
#include "render/render_target_wgpu.h"

#define DRG_MAX_RENDER_TARGETS		4

class drgRenderTarget
{
	public:
				void		Init( int width, int height, DRG_TEX_FORMAT format );
				void		Destroy();
				void		Bind( int index );
				void		Unbind();
				drgTexture*	GetTexture() { return m_Texture; }

				void*		GetPlatformData()	{ return &m_PlatformData; }
				bool		IsDepthStencil()	{ return m_Format == DRG_FMT_D24S8; }

							drgRenderTarget();
		virtual				~drgRenderTarget();

	public:
	protected:

		int								m_Width;
		int								m_Height;
		DRG_TEX_FORMAT					m_Format;
		drgRenderTargetPlatformData		m_PlatformData;
		int                             m_IndexBoundAt;
		drgTexture*						m_Texture;

	protected:

};

#endif