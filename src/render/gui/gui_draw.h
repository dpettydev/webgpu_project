
#ifndef _GUI_DRAWING_H_
#define _GUI_DRAWING_H_

#include "render/context.h"
#include "render/gui/gui.h"
#include "render/gui/font.h"
#include "render/dynamic_draw_command_buffer.h"

#define GUI_ALPHA_OPAQUE	254

class GUI_Style;
class GUI_Window;

class GUI_Drawing
{
public:
	GUI_Drawing();
	~GUI_Drawing();

	void Init(drgRenderContext* ctx);
	void SetStyle(GUI_Style* style = NULL);
	void BeginFrame( GUI_Window* window );
	void EndFrame( GUI_Window* window );
	void DrawRect(GUI_Widget* wid, drgBBox bbox, drgColor* col);
	void DrawRect(GUI_Widget* wid, drgBBox bbox, drgColor* col, drgTexture* tex);
	void DrawRect(GUI_Widget* wid, drgBBox bbox, drgColor* col, drgTexture* tex, drgBBox* tbbox);
	void DrawHorizontallySlicedRect(GUI_Widget* wid, drgBBox bbox, drgColor* col, drgTexture* tex);
	void DrawRectDirect(drgBBox bbox, drgColor* col);
	void DrawRectDirect(drgBBox bbox, drgColor* col, drgTexture* tex);
	void DrawRectDirect(drgBBox bbox, drgColor* col, drgTexture* tex, drgBBox* tbbox);
	void DrawRectStrokeDirect(drgBBox bbox, drgColor* col);
	void DrawBox(GUI_Widget* wid, drgBBox bbox, int z, drgColor* col);
	void DrawFrame(GUI_Widget* wid, drgBBox bbox, int z, drgColor* col);
	void DrawText(GUI_Widget* wid, drgFont* font, const unsigned short* text, drgBBox bbox, drgColor* col, int count=100000);
	void DrawTextDirect(drgBBox bbox, drgFont* font, const unsigned short* text, drgColor* col, int count=100000);

	GUI_Style* GetStyle()
	{
		if(m_Style == NULL)
			SetStyle();
		return m_Style;
	}

	inline drgRenderContext* GetRenderContext()
	{
		return m_CommandBuffer->GetDrawContext();
	}

	inline drgDynamicDrawCommandBuffer* GetCommandBuffer()
	{
		return m_CommandBuffer;
	}

	inline drgRenderCache* GetRenderCache()
	{
		return m_Cache;
	}

	inline void SetCommandBuffer( drgDynamicDrawCommandBuffer* buffer )
	{
		m_CommandBuffer = buffer;
	}

	inline unsigned int GetListID()
	{
		return m_ListID;
	}

	inline drgRenderContext* GetContext()
	{
		return m_Context;
	}

protected:
	void ColorShift(drgColor* col, char* shift);

	GUI_Style*						m_Style;
	unsigned int					m_ListID;
	drgDynamicDrawCommandBuffer*	m_CommandBuffer;
	drgRenderContext*				m_Context;
	drgRenderCache*					m_Cache;
	bool							m_HasRendered;
	friend class GUI_Style;
};

#endif // _GUI_DRAWING_H_

