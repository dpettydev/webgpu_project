
#ifndef _DRG_RENDER_CONTEXT_
#define _DRG_RENDER_CONTEXT_

#include "util/math_def.h"
#include "render/material.h"
#include "render/texture.h"
#include "render/verts.h"
#include "render/render_target.h"
#include "render/model.h"
#include "render/command_nodes.h"


#define NUM_BONE_MATRICES			8
#define DRG_GUI_COMMAND_BUFFER_ID	(1<<(2))

class drgDynamicDrawCommandList;
class drgVertBuffer;

enum DRG_RENDER_PLATFORM
{
	DRG_RENDER_PLATFORM_OPENGL,
	DRG_RENDER_PLATFORM_GLES1,
	DRG_RENDER_PLATFORM_GLES2,
	DRG_RENDER_PLATFORM_DX9,
	DRG_RENDER_PLATFORM_XB360,
	DRG_RENDER_PLATFORM_WEBGPU
};

#define DRG_CLEAR_ALL (DRG_CLEAR_COLOR | DRG_CLEAR_DEPTH | DRG_CLEAR_STENCIL)


class drgRenderContext
{
public:
	drgRenderContext();
	virtual ~drgRenderContext();

	virtual void Init();
	virtual void Begin();
	virtual void End();
	
	virtual void SetAmbientColor( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255 );
	virtual void SetWorldViewProjectionMatrix( drgMat44* world, drgMat44* projection = NULL );
	virtual void SetWorldMatrix( drgMat44* world );
	virtual void SetBoneMatrix( drgMat44* boneMat, int index );
	virtual void SetSpecular( drgColor* spec );
	virtual void SetDiffuseColor( drgColor* color );
	virtual void SetTexCoordOffset( drgVec4* offset );
	virtual void SetViewPosition( drgVec3* position );
	virtual void SetViewDirection( drgVec3* direction );
	virtual void SetupDPI( float xDPI, float yDPI, float baseXDPI, float baseYDPI );

	inline	drgColor		GetAmbientColor()					{ return m_AmbientColor; }
	inline	drgMat44*	GetWorldViewProjectionMatrix()		{ return &m_WorldViewProjectionMatrix; }
	inline	drgMat44*	GetWorldMatrix()					{ return &m_WorldMatrix; }
	inline	drgMat44*	GetBoneMatrix( int index )			{ assert( index >= 0 && index < NUM_BONE_MATRICES ); return &m_BoneMatrices[ index ]; }
	inline	drgColor		GetSpecularColor()					{ return m_SpecularColor; }
	inline	drgColor		GetDiffuseColor()					{ return m_DiffuseColor; }
	inline	drgVec4		GetTexCoordOffset()					{ return m_TexCoordOffset; }
	inline	drgVec3		GetViewPosition()					{ return m_ViewPosition; }
	inline	drgVec3		GetViewDirection()					{ return m_ViewDirection; }

	virtual void RenderPrimitive( unsigned int renderFlags,
		int primType,
		int numPrims,
		int numVertsTot,
		void* data,
		int vertSize,
		void* indices,
		bool setStream = true,
		int startVert = 0,
		void* startData = NULL, 
		int numVerts = 0,
		int streamOffset = 0 );

	inline void ResetAmbient() 
	{ 
		SetAmbientColor( m_AmbientColor.r, m_AmbientColor.g, m_AmbientColor.b ); 
	}

	inline void SetDrawSize(float w, float h)
	{
		m_DrawWidth = w;
		m_DrawHeight = h;
	}

	inline float GetDrawWidth()
	{
		return m_DrawWidth;
	}

	inline float GetDrawHeight()
	{
		return m_DrawHeight;
	}

	inline DRG_RENDER_PLATFORM GetCurrentPlatform()
	{
		return m_Platform;
	}

	inline float GetXDPI()
	{
		return m_XDPI;
	}

	inline float GetYDPI()
	{
		return m_YDPI;
	}

	inline float GetBaseXDPI()
	{
		return m_BaseXDPI;
	}

	inline float GetBaseYDPI()
	{
		return m_BaseYDPI;
	}

	inline float GetXDPIScalingFactor()
	{
		return m_XDPIScalingFactor;
	}

	inline float GetYDPIScalingFactor()
	{
		return m_YDPIScalingFactor;
	}

protected:

	float						m_DrawWidth;
	float						m_DrawHeight;

	drgMat44					m_WorldViewProjectionMatrix;
	drgMat44					m_WorldMatrix;
	drgMat44					m_BoneMatrices[ NUM_BONE_MATRICES ];
	drgColor					m_AmbientColor;
	drgColor					m_SpecularColor;
	drgColor					m_DiffuseColor;
	drgVec4					m_TexCoordOffset;
	drgVec3					m_ViewPosition;
	drgVec3					m_ViewDirection;
	float						m_XDPI;
	float						m_YDPI;
	float						m_BaseXDPI;
	float						m_BaseYDPI;
	float						m_XDPIScalingFactor;
	float						m_YDPIScalingFactor;

	DRG_RENDER_PLATFORM			m_Platform;

	protected:	// Methods

		virtual		void	ApplyClearTarget( unsigned int clearFlags, drgColor color, float depth, unsigned int stencil );
		virtual		void	ApplyDrawMesh( drgVertBuffer* buffer, drgMaterial* material, int startPrim, int numPrims, bool setup, bool useEffect );
		virtual		void	ApplyDrawModel(drgModel* model);
		virtual		void	ApplyScissorEnable( bool enable );
		virtual		void	ApplyScissorState( bool enable, short top, short bottom, short left, short right );
		virtual		void	ApplyClipRect( short top, short bottom, short left, short right );
		virtual		void	ApplyViewport( short top, short bottom, short left, short right );
		virtual		void	ApplyColorBlend( bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op );
		virtual		void	ApplyAlphaBlend( bool enable, DRG_BLEND_FUNCTION source, DRG_BLEND_FUNCTION dest, DRG_BLEND_OPERATION op );
		virtual		void	ApplyColorBlendEnable( bool enable );
		virtual		void	ApplyColorSourceBlend( DRG_BLEND_FUNCTION func );
		virtual		void	ApplyColorDestBlend( DRG_BLEND_FUNCTION func );
		virtual		void	ApplyColorOperation( DRG_BLEND_OPERATION op );
		virtual		void	ApplyAlphaBlendEnable( bool enable );
		virtual		void	ApplyAlphaSourceBlend( DRG_BLEND_FUNCTION func );
		virtual		void	ApplyAlphaDestBlend( DRG_BLEND_FUNCTION func );
		virtual		void	ApplyAlphaOperation( DRG_BLEND_OPERATION op );
		virtual		void	ApplyDepthState( bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func );
		virtual		void	ApplyStencilState( bool enableTest, bool enableWrite, DRG_COMPARE_FUNC func, int ref, int mask );
		virtual		void	ApplyDepthTestEnable( bool enable );
		virtual		void	ApplyDepthWriteEnable( bool enable );
		virtual		void	ApplyDepthCompareFunc( DRG_COMPARE_FUNC func );
		virtual		void	ApplyStencilTestEnable( bool enable );
		virtual		void	ApplyStencilWriteEnable( bool enable );
		virtual		void	ApplyStencilCompareFunc( DRG_COMPARE_FUNC func );
		virtual		void	ApplyStencilRef( int ref );
		virtual		void	ApplyStencilMask( int mask );
		virtual		void	ApplyCullMode( DRG_CULL_MODE mode );
		virtual		void	ApplyFrontFace( DRG_WINDING_ORDER order );
		virtual		void	ApplyFillMode( DRG_FILL_MODE mode );
		virtual		void	ApplySwapBuffers( GUI_Window* window );
		virtual		void	ApplyMakeCurrent( void* platformRenderContext, void* platformDeviceContext );
		virtual		void	ApplyBindRenderTarget( drgRenderTarget* target, int index );
		virtual		void	ApplyUnbindRenderTarget( drgRenderTarget* target );
		virtual		void	ApplyUpdateSurfaces();

		friend class drgDrawCommandBuffer;
		friend class drgDynamicDrawCommandBuffer;
};


#include "render/context_ogl.h"
#include "render/context_wgpu.h"

#endif // _DRG_RENDER_CONTEXT_



