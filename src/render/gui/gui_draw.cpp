
#include "render/gui/gui_draw.h"
#include "render/gui/style.h"
#include "render/material.h"
#include "render/render_cache.h"
#include "render/draw_command_buffer_manager.h"

char g_FocusSunkColorShift[3] = {-10, -10, -20};
char g_FocusRaisedColorShift[3] = {30, 30, 20};
char g_NofocusSunkColorShift[3] = {-20, -20, -20};
char g_NofocusRaisedColorShift[3] = {10, 10, 10};
char g_HighColorShift[3] = {40, 40, 40};
char g_LowColorShift[3] = {-30, -30, -20};

static unsigned int g_GuiDrawListID = DRG_MAX_COMMAND_BUFFERS; // These are generally used to for tool GUI so we want to use as few buffers as possible.

GUI_Drawing::GUI_Drawing()
{
	m_ListID = --g_GuiDrawListID;
	m_HasRendered = false;
	m_Style = NULL;
}

GUI_Drawing::~GUI_Drawing()
{
	SAFE_DELETE(m_Style);
	SAFE_DELETE(m_Context);
	SAFE_DELETE(m_Cache);
}

void GUI_Drawing::Init(drgRenderContext *ctx)
{
	m_Context = ctx;

	drgDrawCommandBufferManager::CreateCommandBuffer(m_ListID, m_Context, DRG_COMMAND_BUFFER_DYNAMIC);
	m_CommandBuffer = (drgDynamicDrawCommandBuffer *)drgDrawCommandBufferManager::GetCommandBuffer(m_ListID);
	assert(m_CommandBuffer && m_CommandBuffer->GetCommandBufferType() == DRG_COMMAND_BUFFER_DYNAMIC);

	m_Cache = drgDrawCommandBufferManager::GetCommandBufferRenderCache(m_ListID);
}

void GUI_Drawing::SetStyle(GUI_Style *style)
{
	if (style == NULL)
	{
		MEM_INFO_SET_NEW;
		style = new GUI_Style();
	}
	if (m_Style != NULL)
	{
		delete m_Style;
		m_Style = NULL;
	}
	m_Style = style;
	m_Style->InitDefault();
	m_Style->SetDrawInfo(this);
}

void GUI_Drawing::BeginFrame(GUI_Window *window)
{
	if (m_CommandBuffer)
	{
		m_CommandBuffer->Begin();
	}
	if (m_HasRendered)
	{
		drgDrawCommandBufferManager::SwapBuffers(window, m_CommandBuffer->GetListID());
	}
	drgColor bgcolor = GetStyle()->m_DefaultColors[WINDOW_BG_COLOR];
	if (window->HasFlagWin(DRG_WINDOW_NOBACKGROUND))
	{
		bgcolor.Set(window->GetColor());
	}
	drgDrawCommandBufferManager::ClearTarget(DRG_CLEAR_ALL, bgcolor, 1.0f, 1, m_ListID);

	// Setup culling
	m_Cache->SetFrontFace(DRG_WINDING_CLOCKWISE);
	m_Cache->SetCullMode(DRG_CULL_NONE);

	// Setup depth testing
	m_Cache->SetDepthState(true, true, DRG_COMPARE_LEQUAL);

	// Setup blending
	m_Cache->SetAlphaBlend(true, DRG_BLEND_FUNC_SOURCE, DRG_BLEND_FUNC_INV_SOURCE, DRG_BLEND_OP_MULT);

	// Setup scissoring
	m_Cache->SetScissorState(true, 0, (short)m_Context->GetDrawHeight(), 0, (short)m_Context->GetDrawWidth());

	m_Cache->SetViewport(0.0f, m_Context->GetDrawHeight(), 0.0f, m_Context->GetDrawWidth());

	m_Style->SetDrawInfo(this);
}

void GUI_Drawing::EndFrame(GUI_Window *window)
{
	if (m_CommandBuffer)
	{
		m_CommandBuffer->End();
		m_CommandBuffer->Draw(true);
	}

	m_HasRendered = true;
}

void GUI_Drawing::DrawRect(GUI_Widget *wid, drgBBox bbox, drgColor *col)
{
	wid->WidgetOffsetRect(&bbox);
	m_CommandBuffer->DrawRect(&bbox, col);
}

void GUI_Drawing::DrawRect(GUI_Widget *wid, drgBBox bbox, drgColor *col, drgTexture *tex)
{
	if (tex == NULL)
	{
		DrawRect(wid, bbox, col);
		return;
	}

	drgBBox tbbox;
	tbbox.minv.Set(0.0f, 0.0f, 0.0f);
	tbbox.maxv.Set(1.0f, 1.0f, 0.0f);
	wid->WidgetOffsetRect(&bbox);
	m_CommandBuffer->DrawRect(&bbox, &tbbox, col, tex);
}

//! param tbbox -- the normalized (0-1) box describing the uv coordinates on the texture to use
void GUI_Drawing::DrawRect(GUI_Widget *wid, drgBBox bbox, drgColor *col, drgTexture *tex, drgBBox *tbbox)
{
	if (tex == NULL)
	{
		DrawRect(wid, bbox, col);
		return;
	}

	wid->WidgetOffsetRect(&bbox);
	m_CommandBuffer->DrawRect(&bbox, tbbox, col, tex);
}

// Splits the texture into three even parts.  The left and right parts of the texture are not stretched,
//    but the middle part of the texture is stretched to fill the widget's bbox.
//! param wid -- the widget to draw
//! param bbox -- the bounding box representing the screen space the widget takes up
//! param col -- the color filter to apply to the texture; or if no texture, the color to draw
//! param tex -- the texture to use.  (the texture should have a 3:1 aspect ratio)
void GUI_Drawing::DrawHorizontallySlicedRect(GUI_Widget *wid, drgBBox bbox, drgColor *col, drgTexture *tex)
{
	if (tex == NULL)
	{
		DrawRect(wid, bbox, col);
		return;
	}

	float third = 1.0f / 3.0f;
	drgBBox sliceBBox;
	drgBBox tbbox;

	float widgetWidth = bbox.maxv.x - bbox.minv.x;
	float widgetHeight = bbox.maxv.y - bbox.minv.y;

	// LEFT
	tbbox.minv.x = 0;
	tbbox.minv.y = 0;
	tbbox.maxv.x = third;
	tbbox.maxv.y = 1;

	sliceBBox.minv.x = bbox.minv.x;
	sliceBBox.minv.y = bbox.minv.y;
	sliceBBox.maxv.x = bbox.minv.x + widgetHeight;
	sliceBBox.maxv.y = bbox.maxv.y;

	DrawRect(wid, sliceBBox, col, tex, &tbbox);

	// MIDDLE
	tbbox.minv.x = third;
	tbbox.minv.y = 0;
	tbbox.maxv.x = 2 * third;
	tbbox.maxv.y = 1;

	sliceBBox.minv.x = bbox.minv.x + widgetHeight;
	sliceBBox.minv.y = bbox.minv.y;
	sliceBBox.maxv.x = bbox.minv.x + widgetWidth - widgetHeight;
	sliceBBox.maxv.y = bbox.maxv.y;

	DrawRect(wid, sliceBBox, col, tex, &tbbox);

	// RIGHT
	tbbox.minv.x = 2 * third;
	tbbox.minv.y = 0;
	tbbox.maxv.x = 1;
	tbbox.maxv.y = 1;

	sliceBBox.minv.x = bbox.minv.x + widgetWidth - widgetHeight;
	sliceBBox.minv.y = bbox.minv.y;
	sliceBBox.maxv.x = bbox.minv.x + widgetWidth;
	sliceBBox.maxv.y = bbox.maxv.y;

	DrawRect(wid, sliceBBox, col, tex, &tbbox);
}

void GUI_Drawing::DrawRectDirect(drgBBox bbox, drgColor *col)
{
	m_CommandBuffer->DrawRect(&bbox, col);
}

void GUI_Drawing::DrawRectDirect(drgBBox bbox, drgColor *col, drgTexture *tex)
{
	drgBBox tbbox;
	tbbox.minv.Set(0.0f, 0.0f, 0.0f);
	tbbox.maxv.Set(1.0f, 1.0f, 0.0f);
	m_CommandBuffer->DrawRect(&bbox, &tbbox, col, tex);
}

void GUI_Drawing::DrawRectDirect(drgBBox bbox, drgColor *col, drgTexture *tex, drgBBox *tbbox)
{
	m_CommandBuffer->DrawRect(&bbox, tbbox, col, tex);
}

void GUI_Drawing::DrawRectStrokeDirect(drgBBox bbox, drgColor *col)
{
	drgBBox top = drgBBox(drgVec3(bbox.minv.x, bbox.minv.y, 0), drgVec3(bbox.maxv.x, bbox.minv.y + 1, 0));
	m_CommandBuffer->DrawRect(&top, col);

	drgBBox bottom = drgBBox(drgVec3(bbox.minv.x, bbox.maxv.y - 1, 0), drgVec3(bbox.maxv.x, bbox.maxv.y, 0));
	m_CommandBuffer->DrawRect(&bottom, col);

	drgBBox left = drgBBox(drgVec3(bbox.minv.x, bbox.minv.y, 0), drgVec3(bbox.minv.x + 1, bbox.maxv.y, 0));
	m_CommandBuffer->DrawRect(&left, col);

	drgBBox right = drgBBox(drgVec3(bbox.maxv.x - 1, bbox.minv.y, 0), drgVec3(bbox.maxv.x, bbox.maxv.y, 0));
	m_CommandBuffer->DrawRect(&right, col);
}

void GUI_Drawing::DrawBox(GUI_Widget *wid, drgBBox bbox, int z, drgColor *col)
{
	drgBBox rOffs(bbox);
	drgColor color(*col);

	if (wid->IsFocused())
		ColorShift(&color, (z < 0) ? g_FocusSunkColorShift : g_FocusRaisedColorShift);
	else
		ColorShift(&color, (z < 0) ? g_NofocusSunkColorShift : g_NofocusRaisedColorShift);

	wid->WidgetOffsetRect(&rOffs);
	m_CommandBuffer->DrawRect(&rOffs, &color);
	DrawFrame(wid, bbox, z, &color);
}

void GUI_Drawing::DrawFrame(GUI_Widget *wid, drgBBox bbox, int z, drgColor *col)
{
	drgColor col1(*col);
	drgColor col2(*col);
	float x2 = bbox.maxv.x - 1.0f;
	float y2 = bbox.maxv.y - 1.0f;

	wid->WidgetOffsetRect(&bbox);
	ColorShift(&col1, (z < 0) ? g_LowColorShift : g_HighColorShift);
	ColorShift(&col2, (z < 0) ? g_HighColorShift : g_LowColorShift);

	if (col1.a < GUI_ALPHA_OPAQUE)
	{
		// m_Context->DrawLineBlended(drv, r.x, r.y, x2,  r.y, c[0], AG_ALPHA_SRC, AG_ALPHA_ZERO);
		// m_Context->DrawLineBlended(drv, r.x, r.y, r.x, y2,  c[0], AG_ALPHA_SRC, AG_ALPHA_ZERO);
	}
	else
	{
		// m_Context->DrawLineH(drv, r.x, x2,  r.y, c[0]);
		// m_Context->DrawLineV(drv, r.x, r.y, y2,  c[0]);
	}
	if (col1.a < GUI_ALPHA_OPAQUE)
	{
		// m_Context->DrawLineBlended(drv, r.x, y2,  x2, y2, c[1], AG_ALPHA_SRC, AG_ALPHA_ZERO);
		// m_Context->DrawLineBlended(drv, x2,  r.y, x2, y2, c[1], AG_ALPHA_SRC, AG_ALPHA_ZERO);
	}
	else
	{
		// m_Context->DrawLineH(drv, r.x, x2,  y2, c[1]);
		// m_Context->DrawLineV(drv, x2,  r.y, y2, c[1]);
	}
}

void GUI_Drawing::DrawText(GUI_Widget *wid, drgFont *font, const unsigned short *text, drgBBox bbox, drgColor *col, int count)
{
	drgBBox rOffs(bbox);

	if (wid)
		wid->WidgetOffsetRect(&rOffs);

	DrawTextDirect(rOffs, font, text, col, count);
}

void GUI_Drawing::DrawTextDirect(drgBBox bbox, drgFont *font, const unsigned short *text, drgColor *col, int count)
{
	font->Print(this->GetCommandBuffer(), *col, (int)bbox.minv.x, (int)bbox.minv.y, text, 1.0f, count);
}

void GUI_Drawing::ColorShift(drgColor *col, char *shift)
{
	int r = col->r + shift[0];
	int g = col->g + shift[1];
	int b = col->b + shift[2];

	if (r > 255)
	{
		r = 255;
	}
	else if (r < 0)
	{
		r = 0;
	}
	if (g > 255)
	{
		g = 255;
	}
	else if (g < 0)
	{
		g = 0;
	}
	if (b > 255)
	{
		b = 255;
	}
	else if (b < 0)
	{
		b = 0;
	}

	col->r = (unsigned char)r;
	col->g = (unsigned char)g;
	col->b = (unsigned char)b;
}
