
#include "render/gui/gui.h"
#include "render/gui/gui_manager.h"
#include "render/gui/gui_draw.h"
#include "render/gui/style.h"
#include "render/gui/gui_properties.h"
#include "render/render_cache.h"
#include "core/input.h"

#define DEFAULT_DOCK_RANGE 0.25f
#define DEFAULT_TITLE_FONT_WIDTH 75
#define DEFAULT_COLLAPSED_HEIGHT 14.0f;

#define DEFAULT_DRAG_THRESHOLD 15.0f

#define WINDOW_BOT_BORDER_DEFAULT 3.0f
#define WINDOW_SIDE_BORDER_DEFAULT 0.0f
#define NUM_DRAW_LIST_NODES 1000

#define SLIDER_INC_DEC_TIME_MAX		(0.8f)
#define SLIDER_INC_DEC_TIME_MIN		(0.02f)

#define SCROLL_INC_DEC_TIME_MAX		(0.8f)
#define SCROLL_INC_DEC_TIME_MIN		(0.02f)
#define SCROLL_MODIFIER				(15.0f)

#define TREE_INDENT_SIZE		(15.0f)
#define ITEM_HEIGHT( fontsize ) (fontsize + 2.0f)


static unsigned int G_UI_GUID = 1;

GUI_Widget::GUI_Widget()
{
	m_Sens.minv.x = 0;
	m_Sens.minv.y = 0;
	m_Sens.maxv.x = 0;
	m_Sens.maxv.y = 0;
	m_View = m_Sens;
	m_PadTop = 0.0f;
	m_PadBot = 0.0f;
	m_PadLeft = 0.0f;
	m_PadRight = 0.0f;
	m_Parent = NULL;
	m_CallBack = NULL;
	m_CallBackType = GUI_CB_GENRAL;
	m_CallBackData = NULL;
	m_MinWidth = 2.0f;
	m_MinHeight = 2.0f;
	m_MaxWidth = FLT_MAX;
	m_MaxHeight = FLT_MAX;
	m_Manager = NULL;
	m_HoverText = string16();

	m_WidgetType = GUI_TYPE_WIDGET;

	m_FlagsWidget = 0;
	// m_FlagsWidget |= DRG_WIDGET_DEBUG_RSENS;	 //debug
	// m_FlagsWidget |= DRG_WIDGET_DEBUG_CLIP_RECT; //debug

	m_GUID = G_UI_GUID++;

	m_ReceiverGUIDs = NULL;
	m_NumReceiverGUIDs = 0;

	assert(G_UI_GUID);
}

GUI_Widget::~GUI_Widget()
{
	Destroy();
}

void GUI_Widget::Init()
{
}

void GUI_Widget::Reinit()
{
}

void GUI_Widget::Destroy()
{
	if (HasFlagWidget(DRG_WIDGET_CURRENT_ACTIVE))
	{
		SetCurrentActive(NULL);
	}

	if (m_Manager)
	{
		if (m_Manager->GetDragWidget() == this)
			m_Manager->SetDragWidget(NULL);

		if (m_Manager->GetFocusedWidget() == this)
			m_Manager->SetFocusedWidget(NULL);

		if (m_Manager->GetSelectedWidget() == this)
			m_Manager->SetSelectedWidget(NULL);

		if (m_Manager->GetHoverWidget() == this)
			m_Manager->SetHoverWidget(NULL);
	}

	SAFE_DELETE(m_ReceiverGUIDs);
	m_NumReceiverGUIDs = 0;
}

void GUI_Widget::Draw(GUI_Drawing *draw)
{
}

void GUI_Widget::DrawDebug(GUI_Drawing *draw)
{
	if (m_FlagsWidget & DRG_WIDGET_DEBUG_RSENS)
	{
		drgBBox bbox = drgBBox(m_Sens);
		this->WidgetOffsetRect(&bbox);
		drgColor debugBoxColor = drgColor(0, 255, 0);
		draw->DrawRectStrokeDirect(bbox, &debugBoxColor);
	}
}

void GUI_Widget::DrawClipRectDebug(GUI_Drawing *draw, drgBBox clipRectOld, drgBBox clipRectNew)
{
	if (m_FlagsWidget & DRG_WIDGET_DEBUG_CLIP_RECT)
	{
		short top, bottom, left, right, topNew, bottomNew, leftNew, rightNew = 0;

		clipRectOld.GetRect(&top, &bottom, &left, &right);
		clipRectNew.GetRect(&topNew, &bottomNew, &leftNew, &rightNew);

		// The clip rect arg passed in (red)
		drgColor debugBoxColorOld = drg_Red;
		drgBBox debugBoxOld = drgBBox(drgVec3((float)left - 3.0f, (float)top - 3.0f, 0), drgVec3((float)right + 3.0f, (float)bottom + 3.0f, 0));
		draw->GetRenderCache()->SetClipRect(0, Float2Short(GetParentWindow()->HeightView()), 0, Float2Short(GetParentWindow()->WidthView()));
		draw->DrawRectStrokeDirect(debugBoxOld, &debugBoxColorOld);

		// The clip rect passed into SetClipRect() (magenta, or yellow if inside out)
		drgColor debugBoxColorNewOutsideIn = drg_Magenta;
		drgColor debugBoxColorNewInsideOut = drg_Yellow;
		drgColor debugBoxColorNew = rightNew < leftNew || bottomNew < topNew ? debugBoxColorNewInsideOut : debugBoxColorNewOutsideIn;
		drgBBox debugBoxNew = drgBBox(drgVec3((float)leftNew - 2.0f, (float)topNew - 2.0f, 0), drgVec3((float)rightNew + 2.0f, (float)bottomNew + 2.0f, 0));
		draw->GetRenderCache()->SetClipRect(0, Float2Short(GetParentWindow()->HeightView()), 0, Float2Short(GetParentWindow()->WidthView()));
		draw->DrawRectStrokeDirect(debugBoxNew, &debugBoxColorNew);

		// The actual rect used in SetClipRect() (blue)
		drgColor debugBoxColorNewAdjusted = drg_Blue;
		draw->GetRenderCache()->SetClipRect(topNew, bottomNew, leftNew, rightNew);
		draw->GetRenderCache()->GetClipRect(&topNew, &bottomNew, &leftNew, &rightNew);
		drgBBox debugBoxNewAdjusted = drgBBox(drgVec3((float)leftNew - 1.0f, (float)topNew - 1.0f, 0), drgVec3((float)rightNew + 1.0f, (float)bottomNew + 1.0f, 0));
		draw->GetRenderCache()->SetClipRect(0, Float2Short(GetParentWindow()->HeightView()), 0, Float2Short(GetParentWindow()->WidthView()));
		draw->DrawRectStrokeDirect(debugBoxNewAdjusted, &debugBoxColorNewAdjusted);
	}
}

void GUI_Widget::Redraw()
{
	m_FlagsWidget |= DRG_WIDGET_DIRTY;
	if (m_Parent != NULL)
		m_Parent->Redraw();
}

void GUI_Widget::SetHoverWidget()
{
	if (m_Manager == NULL || IsDisabled() || IsHidden())
		return;

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (drgInputMouse::IsCursorInside(&rOffs))
	{
		m_Manager->SetHoverWidget(this);
	}
	else
	{
		if (m_Manager->GetHoverWidget() == this)
			m_Manager->SetHoverWidget(NULL);
	}
}

void GUI_Widget::OnParentResize()
{
	UpdateSize();
}

void GUI_Widget::UpdateSize()
{
	m_View = m_Sens;
}

bool GUI_Widget::AddBBox(drgBBox *bbox)
{
	if (IsHidden())
		return false;
	bbox->Add(&m_View);
	return true;
}

void GUI_Widget::SetCurrentActive(GUI_Widget *active)
{
	if (m_Parent != NULL)
		m_Parent->SetCurrentActive(active);
}

bool GUI_Widget::IsCursorInBounds()
{
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);

	rOffs.maxv.x -= GetScrollOffset().x;
	rOffs.maxv.y -= GetScrollOffset().y;

	if (!drgInputMouse::IsCursorInside(&rOffs))
		return false;

	return true;
}

bool GUI_Widget::IsPointerInBounds(int pointerIndex)
{
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);

	rOffs.maxv.x -= GetScrollOffset().x;
	rOffs.maxv.y -= GetScrollOffset().y;

	if (!drgInputTouch::GetPointer(pointerIndex)->IsPointerInside(&rOffs))
		return false;

	return true;
}

void GUI_Widget::SelectSubWidget(GUI_Widget *widget)
{
}

void GUI_Widget::WidgetOffsetRect(drgBBox *bbox)
{
	if (m_Parent != NULL)
		m_Parent->WidgetOffsetRectInt(bbox);
}

int GUI_Widget::GetLayer()
{
	int layer = 0;
	if (m_Parent != NULL)
		layer = m_Parent->GetLayer() + 1;

	if (m_Manager != NULL && layer > m_Manager->GetTopLayer())
		m_Manager->SetTopLayer(layer);

	return layer;
}

void GUI_Widget::SetHoverText(const char *text)
{
	SetHoverText(string16(text));
}

void GUI_Widget::SetHoverText(const string16 text)
{
	m_HoverText.setstr(0, text, (unsigned short)'\0', true);
}

void GUI_Widget::GetHoverText(string16 *text_output)
{
	text_output->setstr(0, m_HoverText, (unsigned short)'\0', true);
}

float GUI_Widget::GetBorderWidth()
{
	return 0.0f;
}

void GUI_Widget::GetNativePosView(drgVec2 *pos)
{
	//
	// If we are not a window, just return the widget offset rect
	//

	if (m_Parent != NULL)
		m_Parent->GetNativePosView(pos);

	pos->x += m_View.minv.x;
	pos->y += m_View.minv.y;
}

void GUI_Widget::GetNativePosBox(drgVec2 *pos)
{
	//
	// If we are not a window, just return the widget offset rect
	//

	if (m_Parent != NULL)
		m_Parent->GetNativePosView(pos);

	pos->x += m_Sens.minv.x;
	pos->y += m_Sens.minv.y;
}

void GUI_Widget::WidgetOffsetRectInt(drgBBox *bbox)
{
	if (m_Parent != NULL)
		m_Parent->WidgetOffsetRectInt(bbox);
	bbox->minv.x += m_View.minv.x;
	bbox->minv.y += m_View.minv.y;
	bbox->maxv.x += m_View.minv.x;
	bbox->maxv.y += m_View.minv.y;
}

void GUI_Widget::Move(float x, float y)
{
	m_Sens.minv.x += x;
	m_Sens.maxv.x += x;
	m_Sens.minv.y += y;
	m_Sens.maxv.y += y;
	UpdateSize();
}

void GUI_Widget::ResizeRel(float top, float bot, float left, float right)
{
	if ((WidthBox() - 1.0f) > (left - right))
	{
		m_Sens.minv.x += left;
		m_Sens.maxv.x += right;
	}
	if ((HeightBox() - 1.0f) > (top - bot))
	{
		m_Sens.minv.y += top;
		m_Sens.maxv.y += bot;
	}

	UpdateSize();
}

void GUI_Widget::Resize(float top, float bot, float left, float right)
{
	//
	// Make sure we clamp to our bounds.
	//

	float width = right - left;
	float height = bot - top;

	width = DRG_MAX(width, m_MinWidth);
	height = DRG_MAX(height, m_MinHeight);

	width = DRG_MIN(width, m_MaxWidth);
	height = DRG_MIN(height, m_MaxHeight);

	right = left + width;
	bot = top + height;

	//
	// Now we can resize.
	//

	m_Sens.minv.x = left;
	m_Sens.maxv.x = right;
	m_Sens.minv.y = top;
	m_Sens.maxv.y = bot;

	UpdateSize();
}

void GUI_Widget::SetPos(float x, float y)
{
	m_Sens.maxv.x = x + (m_Sens.maxv.x - m_Sens.minv.x);
	m_Sens.maxv.y = y + (m_Sens.maxv.y - m_Sens.minv.y);
	m_Sens.minv.x = x;
	m_Sens.minv.y = y;

	UpdateSize();
}

GUI_Window *GUI_Widget::GetParentWindow()
{
	if (m_Parent)
		return m_Parent->GetParentWindow();
	return NULL;
}

GUI_Pane *GUI_Widget::GetParentPane()
{
	if (m_Parent)
		return m_Parent->GetParentPane();
	return NULL;
}

drgVec3 GUI_Widget::GetScrollOffset()
{
	if (m_Parent != NULL)
		return m_Parent->GetScrollOffset();
	return drgVec3(0, 0, 0);
}

drgVec3 GUI_Widget::GetPaneClipMax()
{
	if (m_Parent != NULL)
		return m_Parent->GetPaneClipMax();
	return drgVec3(0, 0, 0);
}

void GUI_Widget::SetPaneClipMax(drgVec3 max)
{
}

GUI_Drawing *GUI_Widget::GetDrawContext()
{
	if (m_Parent)
		return m_Parent->GetDrawContext();
	return NULL;
}

void GUI_Widget::DragDraw(GUI_Drawing *draw)
{
}

void GUI_Widget::ReceiverDraw(GUI_Drawing *draw)
{
	drgBBox box = m_View;
	WidgetOffsetRect(&box);

	drgColor color = draw->GetStyle()->GetDefaultColor(DOCKING_RECEIVER_COLOR);
	draw->DrawBox(this, box, 0, &color);
}

void GUI_Widget::DragDone()
{
}

bool GUI_Widget::IsPointInBounds(drgVec2 *point)
{
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);

	bool isInside = point->x <= rOffs.maxv.x && point->y >= rOffs.minv.x &&
					point->y <= rOffs.maxv.y && point->y >= rOffs.minv.y;

	return isInside;
}

bool GUI_Widget::IsPointInGlobalBounds(drgVec2 *point)
{
	drgBBox rOffs(m_Sens);
	drgVec2 globalPos;
	GetNativePosBox(&globalPos);

	float width = rOffs.maxv.x - rOffs.minv.x;
	float height = rOffs.maxv.y - rOffs.minv.y;

	rOffs.minv.x = globalPos.x;
	rOffs.minv.y = globalPos.y;
	rOffs.maxv.x = globalPos.x + width;
	rOffs.maxv.y = globalPos.y + height;

	bool isInside = point->x <= rOffs.maxv.x && point->x >= rOffs.minv.x &&
					point->y <= rOffs.maxv.y && point->y >= rOffs.minv.y;

	return isInside;
}

bool GUI_Widget::IsChildOf(GUI_Widget *widget)
{
	//
	// If I have no parent, return false.
	//

	if (GetParent() == NULL)
		return false;

	if (widget == GetParent())
	{
		return true;
	}
	else
	{
		return GetParent()->IsChildOf(widget);
	}
}

void GUI_Widget::SetParent(GUI_Widget *widget)
{
	m_Parent = widget;
	if (m_Parent)
	{
		m_Manager = m_Parent->GetManager();
	}
	else
	{
		m_Manager = NULL;
	}
}

GUI_Widget *GUI_Widget::GetParent()
{
	return m_Parent;
}

void GUI_Widget::SetClipRectWithOffsets(GUI_Drawing *draw, short top, short bottom, short left, short right)
{
	drgBBox rOffs(m_View);
	WidgetOffsetRect(&rOffs);

	short topNew, bottomNew, leftNew, rightNew;

	topNew = DRG_MAX(Float2Short((rOffs.minv.y)), top);
	leftNew = DRG_MAX(Float2Short((rOffs.minv.x)), left);

	drgVec3 paneMax = GetPaneClipMax();
	drgVec3 scrollOffset = GetScrollOffset();
	if (paneMax.IsZero() || scrollOffset.IsZero())
	{
		bottomNew = Float2Short(DRG_MIN((rOffs.maxv.y), (float)bottom) - scrollOffset.y);
		rightNew = Float2Short(DRG_MIN((rOffs.maxv.x), (float)right) - scrollOffset.x);
	}
	else
	{
		bottomNew = Float2Short(paneMax.y);
		rightNew = Float2Short(paneMax.x);
	}

	DrawClipRectDebug(draw, drgBBox(top, bottom, left, right), drgBBox(topNew, bottomNew, leftNew, rightNew));

	draw->GetRenderCache()->SetClipRect(topNew, bottomNew, leftNew, rightNew);

	SetPaneClipMax(drgVec3(rightNew, bottomNew, 0));
}

void GUI_Widget::SetUserData(drgObjectBase *data)
{
	m_UserData = data;
}

drgObjectBase *GUI_Widget::GetUserData()
{
	return m_UserData;
}

void GUI_Widget::ReceiveWidget(GUI_Widget *widget)
{
}

void GUI_Widget::AddReceiverGUID(unsigned int receiverGUID)
{
	unsigned int *prev = m_ReceiverGUIDs;
	m_NumReceiverGUIDs++;

	m_ReceiverGUIDs = (unsigned int *)drgMemAlloc(sizeof(unsigned int) * m_NumReceiverGUIDs);
	if (prev)
	{
		drgMemory::MemCopy(m_ReceiverGUIDs, prev, sizeof(unsigned int) * (m_NumReceiverGUIDs - 1));
		drgMemFree(prev);
	}

	m_ReceiverGUIDs[m_NumReceiverGUIDs - 1] = receiverGUID;
}

void GUI_Widget::RemoveReceiverGUID(unsigned int receiverGUID)
{
	bool shouldResize = false;
	for (unsigned int currentReceiverGUID = 0; currentReceiverGUID < m_NumReceiverGUIDs; ++currentReceiverGUID)
	{
		if (m_ReceiverGUIDs[currentReceiverGUID] == receiverGUID)
		{
			shouldResize = true;
			break;
		}
	}

	if (shouldResize == false)
		return;

	m_NumReceiverGUIDs--;
	if (m_NumReceiverGUIDs > 0)
	{
		unsigned int *prev = m_ReceiverGUIDs;
		m_ReceiverGUIDs = (unsigned int *)drgMemAlloc(sizeof(unsigned int) * m_NumReceiverGUIDs);

		int offset = 0;
		if (prev)
		{
			for (unsigned int currentReceiverGUID = 0; currentReceiverGUID < m_NumReceiverGUIDs - 1; ++currentReceiverGUID)
			{
				if (prev[currentReceiverGUID] == receiverGUID)
				{
					offset++;
				}

				m_ReceiverGUIDs[currentReceiverGUID] = prev[currentReceiverGUID + offset];
			}

			drgMemFree(prev);
		}

		m_ReceiverGUIDs[m_NumReceiverGUIDs - 1] = receiverGUID;
	}
}

void GUI_Widget::ClearReceiverGUIDs()
{
	SAFE_FREE(m_ReceiverGUIDs);
	m_NumReceiverGUIDs = 0;
}

bool GUI_Widget::IsValidReceiver(GUI_Widget *widget)
{
	assert(widget);

	if (m_NumReceiverGUIDs == 0 && widget)
		return true;

	for (unsigned int currentReceiverGUID = 0; currentReceiverGUID < m_NumReceiverGUIDs; ++currentReceiverGUID)
	{
		if (m_ReceiverGUIDs[currentReceiverGUID] == widget->GetGUID())
		{
			return true;
		}
	}

	return false;
}

GUI_Container::GUI_Container()
{
	m_ContainerResizeType = DRG_CONTAINER_RESIZE_ARBITRARY;
	m_TitleBar = NULL;

	m_DockRange = DEFAULT_DOCK_RANGE;
	m_WidgetType = GUI_TYPE_CONTAINER;

	m_Menu = NULL;
	m_CollapseButton = NULL;
	m_IsCollapsed = false;
	m_CanCollapse = false;
	m_CollapsedHeight = DEFAULT_COLLAPSED_HEIGHT;
	m_UnCollapsedHeight = 0.0f;
}

GUI_Container::~GUI_Container()
{
	Destroy();
}

void GUI_Container::Init()
{
	m_Type = DRG_CONTAINER_VERT;
	m_ContainerResizeType = DRG_CONTAINER_RESIZE_ARBITRARY;

	InitCollapseButton();
}

void GUI_Container::InitCollapseButton()
{
	if (m_CollapseButton == NULL)
	{
		m_CollapseButton = GUI_Button::CreateChildButton(this, ICON_ARRW_DOWN, ICON_ARRW_DOWN, ICON_ARRW_RIGHT, &GUI_Container::HandleCollapseButtonDown, NULL, DRG_BUTTON_STICKY);
		m_CollapseTitle = GetScriptClass();
	}
}

void GUI_Container::Reinit()
{
}

void GUI_Container::Destroy()
{
	RemoveAllChildren();

	if (m_TitleBar)
	{
		delete m_TitleBar; // CreateTitleBar news the pointer for us. We need to clean it up.
		m_TitleBar = NULL;
	}
}

void GUI_Container::Draw(GUI_Drawing *draw)
{
	if (IsHidden())
		return;

	DRG_PROFILE_FUNK(GUI_Container, 0xFFFFFF00);

	// Setup the clipping rect. Do we need to do this here AND in DrawChildren
	short top = 0, bot = 0, left = 0, right = 0;
	drgBBox rOffs(m_View);
	if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
	{
		draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
		WidgetOffsetRect(&rOffs);

		short newTop = DRG_MAX((short)(rOffs.minv.y), top);
		short newBottom = DRG_MIN((short)(rOffs.maxv.y), bot);
		short newLeft = DRG_MAX((short)(rOffs.minv.x), left);
		short newRight = DRG_MIN((short)(rOffs.maxv.x), right);

		draw->GetRenderCache()->SetClipRect(newTop, newBottom, newLeft, newRight);
	}

	AccordionDrawChildren(draw);

	//
	// Reset the clipping rect.
	//

	if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
	{
		draw->GetRenderCache()->SetClipRect(top, bot, left, right);
	}

	//
	// If we have a title bar, draw it here.
	//

	if (m_TitleBar)
	{
		m_TitleBar->Draw(draw);
	}

	if (m_Menu != NULL)
		m_Menu->Draw(draw);

	GUI_Widget::Draw(draw);
}

void GUI_Container::AccordionDrawChildren(GUI_Drawing *draw)
{
	if (m_CanCollapse)
	{
		m_CollapsedHeight = draw->GetStyle()->GetCollapsedHeight();
	}

	if (!IsCollapsed())
	{
		DrawChildren(draw);
	}

	if (m_CanCollapse)
	{
		draw->GetStyle()->CollapsedTitle(this, m_TitleBar, &m_CollapseTitle, m_CollapseButton);
		if (m_CollapseButton != NULL)
		{
			m_CollapseButton->Draw(draw);
		}
	}
}

void GUI_Container::CallMouseDownCollapseButton(unsigned int winid, int button)
{
	if (m_CollapseButton != NULL)
	{
		m_CollapseButton->CallMouseDown(winid, button);
	}
}

void GUI_Container::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (IsDisabled() || IsHidden())
	{
		return;
	}

	// Make sure the title bar has a chance to handle input
	if (m_TitleBar)
	{
		m_TitleBar->CallKeyClick(winid, key, charval, down);
	}

	// Give our children a chance.
	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallKeyClick(winid, key, charval, down);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}
}

bool GUI_Container::CallMouseDown(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	// Give our children a chance to handle the input
	CallMouseDownCollapseButton(winid, button);

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseDown(winid, button);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		if (itr->CallMouseDown(winid, button))
			return true;
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
		m_Menu->CallMouseDown(winid, button);

	return false;
}

bool GUI_Container::CallMouseClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	// Give our children a chance to handle the input
	if (m_TitleBar)
	{
		m_TitleBar->CallMouseClick(winid, button);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		if (itr->CallMouseClick(winid, button))
			return true;
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
		m_Menu->CallMouseClick(winid, button);

	return false;
}

void GUI_Container::CallMouseUp(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return;

	// Give our children a chance to handle the input
	if (m_TitleBar)
	{
		m_TitleBar->CallMouseUp(winid, button);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallMouseUp(winid, button);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
		m_Menu->CallMouseUp(winid, button);
}

bool GUI_Container::CallMouseDoubleClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	//
	// Give our children a chance to handle the input
	//

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseDoubleClick(winid, button);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		if (itr->CallMouseDoubleClick(winid, button))
			return true;
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
		m_Menu->CallMouseDoubleClick(winid, button);

	return false;
}

void GUI_Container::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	if (IsDisabled() || IsHidden())
		return;

	if (IsCursorInBounds() == false)
		return;

	//
	// Give our children a chance to handle the input
	//

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseWheel(winid, pos, rel);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallMouseWheel(winid, pos, rel);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}
	return;
}

void GUI_Container::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (IsDisabled() || IsHidden())
		return;

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseMove(winid, pos, rel);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallMouseMove(winid, pos, rel);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
	{
		m_Menu->CallMouseMove(winid, pos, rel);
	}
}

void GUI_Container::CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	if (IsDisabled() || IsHidden())
	{
		return;
	}
	// Have our children handle drag as well.
	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallMouseDrag(winid, pos, rel);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}
	CheckForReceievers();
}

bool GUI_Container::CallPointerDown(unsigned int winid, int pointerIndex)
{
	if (IsDisabled() || IsHidden())
	{
		return false;
	}
	if (IsPointerInBounds(pointerIndex) == false)
	{
		return false;
	}
	// Give our children a chance to handle the input
	if (m_TitleBar)
	{
		m_TitleBar->CallPointerDown(winid, pointerIndex);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		if (itr->CallPointerDown(winid, pointerIndex))
		{
			return true;
		}
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
	{
		m_Menu->CallPointerDown(winid, pointerIndex);
	}
	return false;
}

bool GUI_Container::CallPointerClick(unsigned int winid, int pointerIndex)
{
	if (IsDisabled() || IsHidden())
	{
		return false;
	}
	if (IsPointerInBounds(pointerIndex) == false)
	{
		return false;
	}
	// Give our children a chance to handle the input
	if (m_TitleBar)
	{
		m_TitleBar->CallPointerClick(winid, pointerIndex);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		if (itr->CallPointerClick(winid, pointerIndex))
		{
			return true;
		}
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
	{
		m_Menu->CallPointerClick(winid, pointerIndex);
	}
	return false;
}

void GUI_Container::CallPointerUp(unsigned int winid, int pointerIndex)
{
	if (IsDisabled() || IsHidden())
	{
		return;
	}
	if (IsCursorInBounds() == false)
	{
		return;
	}
	// Give our children a chance to handle the input
	if (m_TitleBar)
	{
		m_TitleBar->CallPointerUp(winid, pointerIndex);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallPointerUp(winid, pointerIndex);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
	{
		m_Menu->CallPointerUp(winid, pointerIndex);
	}
}

void GUI_Container::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (IsDisabled() || IsHidden())
		return;

	if (m_TitleBar)
	{
		m_TitleBar->CallPointerMove(winid, pointerIndex, pos, rel);
	}

	GUI_Widget *itr = (GUI_Widget *)m_Children.GetFirst();
	while (itr)
	{
		itr->CallPointerMove(winid, pointerIndex, pos, rel);
		itr = (GUI_Widget *)m_Children.GetNext(itr);
	}

	if (m_Menu != NULL)
		m_Menu->CallPointerMove(winid, pointerIndex, pos, rel);
}

void GUI_Container::AddChild(GUI_Widget *widget)
{
	// Alert some people to make sure they don't get unexpected behavior.
	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		assert(widget->HasFlagWidget(DRG_WIDGET_RESIZE_HFILL) && "Need to be designed to horizontally fill this container.");
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		assert(widget->HasFlagWidget(DRG_WIDGET_RESIZE_VFILL) && "Need to be designed to vertically fill this container.");
	}

	// Add the child
	widget->SetParent(this);
	m_Children.AddHead(widget);

	GUI_Window *parentWin = GetParentWindow();
	// if(parentWin != NULL)
	//	parentWin->UpdateSize();
	// else
	this->UpdateSize();
}

void GUI_Container::AddChildAfter(GUI_Widget *widgetToAdd, GUI_Widget *widgetToAfter)
{
	assert(widgetToAdd);

	// Alert some people to make sure they don't get unexpected behavior.
	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		assert(widgetToAdd->HasFlagWidget(DRG_WIDGET_RESIZE_HFILL) && "Need to be designed to horizontally fill this container.");
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		assert(widgetToAdd->HasFlagWidget(DRG_WIDGET_RESIZE_VFILL) && "Need to be designed to vertically fill this container.");
	}

	widgetToAdd->SetParent(this);
	m_Children.AddAfter(widgetToAdd, widgetToAfter);

	GUI_Window *parentWin = GetParentWindow();
	if (parentWin != NULL)
	{
		parentWin->UpdateSize();
	}
	else
	{
		this->UpdateSize();
	}
}

void GUI_Container::RemoveChild(GUI_Widget *widget)
{
	widget->m_Parent = NULL;
	m_Children.Remove(widget);
	UpdateSize();
}

void GUI_Container::RemoveAllChildren()
{
	// We need to delete all our children, releasing them as necessary
	GUI_Widget *iter = (GUI_Widget *)m_Children.GetLast();
	while (iter)
	{
		if (iter)
		{
			RemoveChild(iter);
			iter->DelayedDelete();
		}

		iter = (GUI_Widget *)m_Children.GetLast();
	}
}

void GUI_Container::DrawChildren(GUI_Drawing *draw)
{
	// Draw all our children
	GUI_Widget *itr = (GUI_Widget *)m_Children.GetLast();
	while (itr)
	{
		itr->Draw(draw);
		itr = (GUI_Widget *)m_Children.GetPrev(itr);
	}
}

void GUI_Container::ReceiverDraw(GUI_Drawing *drawing)
{
	// Setup the clipping rect. Do we need to do this here AND in DrawChildren
	short top = 0, bot = 0, left = 0, right = 0;
	if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
	{
		drawing->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
		drgBBox rOffs(m_View);
		WidgetOffsetRect(&rOffs);

		short newTop = DRG_MAX((short)(rOffs.minv.y), top);
		short newBottom = DRG_MIN((short)(rOffs.maxv.y), bot);
		short newLeft = DRG_MAX((short)(rOffs.minv.x), left);
		short newRight = DRG_MIN((short)(rOffs.maxv.x), right);

		drawing->GetRenderCache()->SetClipRect(newTop, newBottom, newLeft, newRight);
	}

	drgBBox drawBox;
	drgVec2 *dragPos;
	drgColor boxColor = drawing->GetStyle()->GetDefaultColor(DOCKING_RECEIVER_COLOR);
	GUI_Widget *widget = m_Manager->GetDragWidget();

	//
	// Get the docking location
	//

	dragPos = drgInputMouse::GetGlobalPos();
	if (IndicateDockingLocations(widget, dragPos, &drawBox))
	{
		//
		// Draw the box
		//

		drawing->DrawRectDirect(drawBox, &boxColor);
	}

	if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
	{
		drawing->GetRenderCache()->SetClipRect(top, bot, left, right);
	}
}

void GUI_Container::ReceiveWidget(GUI_Widget *widget)
{
	drgVec2 *globalMousePos = drgInputMouse::GetGlobalPos();
	drgVec2 myGlobalPos;
	drgVec2 localMousePos;

	GetNativePosView(&myGlobalPos);
	localMousePos.Set(globalMousePos->x - myGlobalPos.x, globalMousePos->y - myGlobalPos.y);

	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		ReceiveWidgetHorizontal(widget, &localMousePos);
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		ReceiveWidgetVertical(widget, &localMousePos);
	}
	else
	{
		AddChild(widget);
	}
}

void GUI_Container::SelectSubWidget(GUI_Widget *widget)
{
	if (m_Children.InList(widget))
	{
		m_Children.Remove(widget);
		m_Children.AddHead(widget);
	}
}

void GUI_Container::DragDone()
{
	if (m_TitleBar)
	{
		m_TitleBar->DragDone();
	}

	GUI_Widget::DragDone();
}

void GUI_Container::ContainerSetType(DRG_CONTAINER_TYPE type)
{
	// AG_SizeAlloc a;
	m_Type = type;
	// a.x = WIDGET(Container)->x;
	// a.y = WIDGET(Container)->y;
	// a.w = WIDGET(Container)->w;
	// a.h = WIDGET(Container)->h;
	// SizeAllocate(Container, &a);
	Redraw();
}

void GUI_Container::SetContainerResizeType(DRG_CONTAINER_RESIZE_TYPE type)
{
	m_ContainerResizeType = type;
	UpdateSize();
}

bool GUI_Container::IsContainer()
{
	return m_WidgetType == GUI_TYPE_CONTAINER ||
		   m_WidgetType == GUI_TYPE_DIV_CONTAINER ||
		   m_WidgetType == GUI_TYPE_TAB_CONTAINER ||
		   m_WidgetType == GUI_TYPE_TABLE ||
		   m_WidgetType == GUI_TYPE_WINDOW ||
		   m_WidgetType == GUI_TYPE_PANE;
}

void GUI_Container::OnParentResize()
{
	GUI_Widget *itr = (GUI_Widget *)m_Children.GetLast();
	while (itr)
	{
		itr->OnParentResize();
		itr = (GUI_Widget *)m_Children.GetPrev(itr);
	}
}

void GUI_Container::UpdateSize()
{
	m_View = m_Sens;

	OffsetFrameForBorder();

	if (m_TitleBar)
	{
		UpdateTitleBarSize();
	}

	if (m_Menu)
		m_Menu->Resize(-m_Menu->HeightView(), 0.0f, 0.0f, WidthBox());

	UpdateMinMaxValues();

	//
	// We need to go through all our children and determine their
	// new bounds. Then they can resize to match it.
	//

	ResizeChildren();
}

void GUI_Container::OffsetFrameForBorder()
{
	m_View.minv.x += GetBorderWidth();
	m_View.minv.y += GetBorderWidth();
	m_View.maxv.x -= GetBorderWidth();
	m_View.maxv.y -= GetBorderWidth();
}

void GUI_Container::UpdateMinMaxValues()
{
	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		UpdateMinMaxHorizontal();
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		UpdateMinMaxVertical();
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_ARBITRARY)
	{
		UpdateMinMaxArbitrary();
	}
}

void GUI_Container::UpdateMinMaxHorizontal()
{
	float minWidth = 0.0f;
	float minHeight = 0.0f;
	float maxWidth = 0.0f;
	float maxHeight = 0.0f;

	GUI_Widget *widget = GetFirst();
	while (widget)
	{
		minHeight += widget->GetMinHeight();
		minWidth = DRG_MAX(widget->GetMinWidth(), minWidth);
		maxWidth = DRG_MAX(widget->GetMaxWidth(), maxWidth);
		maxHeight = DRG_MAX(widget->GetMaxHeight(), maxHeight);

		widget = GetNext((GUI_Widget *)widget);
	}

	SetMinWidth(DRG_MAX(minWidth, m_MinWidth));
	SetMinHeight(DRG_MAX(minHeight, m_MinHeight));
	SetMaxWidth(DRG_MAX(maxWidth, m_MaxWidth));
	SetMaxHeight(DRG_MAX(maxHeight, m_MaxHeight));
}

void GUI_Container::UpdateMinMaxVertical()
{
	float minWidth = 0.0f;
	float minHeight = 0.0f;
	float maxWidth = 0.0f;
	float maxHeight = 0.0f;

	GUI_Widget *widget = GetFirst();
	while (widget)
	{
		minWidth += widget->GetMinWidth();
		minHeight = DRG_MAX(widget->GetMinHeight(), minHeight);
		maxWidth = DRG_MAX(widget->GetMaxWidth(), maxWidth);
		maxHeight = DRG_MAX(widget->GetMaxHeight(), maxHeight);

		widget = GetNext((GUI_Widget *)widget);
	}

	SetMinWidth(DRG_MAX(minWidth, m_MinWidth));
	SetMinHeight(DRG_MAX(minHeight, m_MinHeight));
	SetMaxWidth(DRG_MAX(maxWidth, m_MaxWidth));
	SetMaxHeight(DRG_MAX(maxHeight, m_MaxHeight));
}

void GUI_Container::UpdateMinMaxArbitrary()
{
	// Do nothing for now.
}

void GUI_Container::UpdateTitleBarSize()
{
	float barSize = m_TitleBar->HeightView();
	m_TitleBar->SetWidth(WidthView());
	m_TitleBar->Resize(-barSize, -barSize + m_TitleBar->HeightView(), 0.0f, WidthView());

	m_View.minv.y += barSize;
}

void GUI_Container::ResizeChildren()
{
	//
	// Make sure we actually have children.
	//

	if (m_Children.GetCount() == 0)
		return;

	//
	// We need to resize the children differently based on our container type
	//

	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		ResizeChildrenHorizontally();
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		ResizeChildrenVertically();
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_ARBITRARY)
	{
		ResizeChildrenArbitrarily();
	}
}

void GUI_Container::ResizeChildrenHorizontally()
{
	float avgHeight = HeightView() / m_Children.GetCount();

	GUI_Widget *iter = (GUI_Widget *)m_Children.GetFirst();
	int currentChild = 0;
	while (iter)
	{
		// Our height is the avg height, (we assume they all try to fill us horizontally.
		// Our position is the multiple of the avg height for the child.

		// iter->SetHeight( avgHeight );
		// iter->SetWidth( WidthView() );									// They will have the same width as us.
		// iter->SetPos( iter->PosXBox(), (avgHeight * currentChild++) );	// UpdateSize get's called here.

		float yPos = avgHeight * currentChild++;
		iter->Resize(iter->PosYBox(), yPos + avgHeight,
					 iter->PosXBox(), iter->PosXBox() + WidthView());

		iter = (GUI_Widget *)m_Children.GetNext(iter);
	}
}

void GUI_Container::ResizeChildrenVertically()
{
	float avgWidth = WidthView() / m_Children.GetCount();

	GUI_Widget *iter = (GUI_Widget *)m_Children.GetFirst();
	int currentChild = 0;
	while (iter)
	{
		// Our height is the avg height, (we assume they all try to fill us horizontally.
		// Our position is the multiple of the avg height for the child.

		// iter->SetWidth( avgWidth );
		// iter->SetHeight( HeightView() );				// They will have the same height as us.
		// iter->SetPos( avgWidth * currentChild++, iter->PosYBox() );		// UpdateSize gets called here

		float xPos = avgWidth * currentChild++;
		iter->Resize(iter->PosYBox(), iter->PosYBox() + HeightView(),
					 xPos, xPos + avgWidth);

		iter = (GUI_Widget *)m_Children.GetNext(iter);
	}
}

void GUI_Container::ResizeChildrenArbitrarily()
{
	float titleBarOffset = m_TitleBar ? m_TitleBar->HeightView() : 0.0f;

	//
	// Here we just want to query their resize flags, and
	// just give them whatever resize they want.
	//

	GUI_Widget *iter = (GUI_Widget *)m_Children.GetFirst();
	while (iter)
	{
		float xPos = iter->PosXBox();
		float yPos = iter->PosYBox();
		float height = iter->HeightBox();
		float width = iter->WidthBox();

		if (iter->HasFlagWidget(DRG_WIDGET_RESIZE_HFILL))
		{
			width = WidthView();
			xPos = 0.0f;
		}

		if (iter->HasFlagWidget(DRG_WIDGET_RESIZE_VFILL))
		{
			float myInheritableHeight = HeightView();
			if (m_CanCollapse)
				myInheritableHeight -= GetCollapsedHeight();
			height = myInheritableHeight;
			yPos = 0.0f;
		}

		// iter->SetWidth( width );
		// iter->SetHeight( height );
		// iter->SetPos( xPos, yPos );

		iter->Resize(yPos, yPos + height, xPos, xPos + width); // Update size gets called implictly here.
		iter = (GUI_Widget *)m_Children.GetNext(iter);
	}
}

bool GUI_Container::AddBBox(drgBBox *bbox)
{
	if (IsHidden())
		return false;
	bbox->Add(&m_View);
	if (!IsCollapsed())
	{
		GUI_Widget *itr = (GUI_Widget *)m_Children.GetLast();
		while (itr)
		{
			itr->AddBBox(bbox);
			itr = (GUI_Widget *)m_Children.GetPrev(itr);
		}
	}
	return true;
}

GUI_Container *GUI_Container::CreateContainer(GUI_Widget *parent, DRG_CONTAINER_TYPE type, unsigned int flags)
{
	GUI_Container *contain = new GUI_Container();
	contain->UseAsScriptObject();
	contain->Init();

	contain->m_Type = type;
	contain->m_FlagsWidget |= flags;

	if (flags & DRG_WIDGET_RESIZE_HFILL)
	{
		contain->m_FlagsWidget |= DRG_WIDGET_RESIZE_HFILL;
	}
	if (flags & DRG_WIDGET_RESIZE_VFILL)
	{
		contain->m_FlagsWidget |= DRG_WIDGET_RESIZE_VFILL;
	}

	if (parent != NULL && (flags & DRG_WIDGET_RESIZE_HFILL || flags & DRG_WIDGET_RESIZE_VFILL))
	{
		contain->Resize(0.0f, parent->HeightView(), 0.0f, parent->WidthView());
	}

	return contain;
}

GUI_DivContainer *GUI_Container::CreateChildDivContainer(unsigned int flags, DRG_CONTAINER_TYPE type, DRG_CONTAINER_CHILD_TYPE pane0, DRG_CONTAINER_CHILD_TYPE pane1)
{
	GUI_DivContainer *div = GUI_DivContainer::CreateDivContainer(this, type, flags);

	DRG_CONTAINER_CHILD_TYPE divs[] = {pane0, pane1};
	for (int currentPane = 0; currentPane < 2; ++currentPane)
	{
		switch (divs[currentPane])
		{
		case DRG_CONTAINER_CHILD_PANE:
			div->m_DivPane[currentPane] = GUI_Pane::CreatePane(0);
			break;
		case DRG_CONTAINER_CHILD_DIV:
			div->m_DivPane[currentPane] = GUI_DivContainer::CreateDivContainer(div, type, 0);
			break;
		case DRG_CONTAINER_CHILD_STANDARD:
			div->m_DivPane[currentPane] = CreateContainer(div, type, 0);
			break;
		case DRG_CONTAINER_CHILD_TAB:
			div->m_DivPane[currentPane] = GUI_TabContainer::CreateTabContainer(div, 0);
			break;
		}
		div->m_DivPane[currentPane]->SetParent(div);
	}
	AddChild(div);
	return div;
}

GUI_TabContainer *GUI_Container::CreateChildTabContainer(unsigned int flags)
{
	GUI_TabContainer *tab = GUI_TabContainer::CreateTabContainer(this, flags);
	AddChild(tab);
	return tab;
}

GUI_Pane *GUI_Container::CreateChildPane(string16 name, unsigned int flags)
{
	GUI_Pane *pane = new GUI_Pane();
	pane->UseAsScriptObject();
	pane->Init();
	pane->m_FlagsPane |= flags;
	pane->SetName(name);
	pane->m_CollapseTitle = name;

	if (flags & DRG_PANE_HFILL)
	{
		pane->m_FlagsWidget |= DRG_WIDGET_RESIZE_HFILL;
	}
	if (flags & DRG_PANE_VFILL)
	{
		pane->m_FlagsWidget |= DRG_WIDGET_RESIZE_VFILL;
	}

	AddChild(pane);
	return pane;
}

GUI_Label *GUI_Container::CreateChildLabel(string16 text, unsigned int flags)
{
	GUI_Label *label = GUI_Label::CreateLabel(this, GetDrawContext()->GetStyle()->GetDefaultFont(), text, flags);
	AddChild(label);
	return label;
}

GUI_Log *GUI_Container::CreateChildLog(unsigned int flags)
{
	GUI_Log *log = new GUI_Log();
	log->UseAsScriptObject();
	log->Init(this);
	log->m_Flags = flags;
	log->SetParent(this);
	AddChild(log);

	return log;
}

GUI_Input *GUI_Container::CreateChildInput(unsigned int flags)
{
	GUI_Input *input = new GUI_Input();
	input->UseAsScriptObject();
	input->Init();
	input->m_FlagsInput |= flags;
	input->m_Text = "";
	input->SetParent(this);
	input->m_Sens.minv.x = 0.0f;
	input->m_Sens.minv.y = 0.0f;
	input->m_Sens.maxv.x = 120.0f;
	input->m_Sens.maxv.y = ((float)GetDrawContext()->GetStyle()->GetInputFont()->GetFontSize()) * 1.5f;
	input->UpdateSize();
	AddChild(input);

	return input;
}

GUI_Spinner *GUI_Container::CreateChildSpinner(int width, int height, double startVal, double min, double max, int flags)
{
	GUI_Spinner *spinner = GUI_Spinner::CreateChildSpinner(this, width, height, startVal, min, max, flags);
	AddChild(spinner);
	return spinner;
}

GUI_ProgressBar *GUI_Container::CreateChildProgressBar(int width, int height, float curVal, float completeVal, const char *label, unsigned int flags)
{
	GUI_ProgressBar *progressbar = GUI_ProgressBar::CreateChildProgressBar(this, width, height, curVal, completeVal, label, flags);
	AddChild(progressbar);
	return progressbar;
}

GUI_Icon *GUI_Container::CreateChildIcon(unsigned int width, unsigned int height, unsigned int flags)
{
	GUI_Icon *icon = new GUI_Icon();
	icon->UseAsScriptObject();
	icon->Init();
	icon->m_FlagsIcon |= (flags | DRG_ICON_BGFILL);
	icon->m_Sens.minv.x = 0;
	icon->m_Sens.minv.y = 0;
	icon->m_Sens.maxv.x = (float)width;
	icon->m_Sens.maxv.y = (float)height;
	icon->UpdateSize();
	AddChild(icon);
	return icon;
}

GUI_Icon *GUI_Container::CreateChildIcon(char *filename, unsigned int flags)
{
	GUI_Icon *icon = new GUI_Icon();
	icon->UseAsScriptObject();
	icon->Init();
	icon->m_FlagsIcon |= flags;
	icon->m_Texture = drgTexture::LoadFromFile(filename);
	icon->m_Sens.minv.x = 0;
	icon->m_Sens.minv.y = 0;
	icon->m_Sens.maxv.x = (float)icon->m_Texture->GetWidth();
	icon->m_Sens.maxv.y = (float)icon->m_Texture->GetHeight();
	icon->UpdateSize();
	AddChild(icon);
	return icon;
}

GUI_Button *GUI_Container::CreateChildButton(string16 text, unsigned int width, unsigned int height, drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = new GUI_Button();
	button->UseAsScriptObject();
	button->Init();
	button->SetParent(this);
	button->m_FlagsButton |= flags;
	button->m_Type = BTN_TYPE_NORMAL;
	button->SetCallBack(callback, cbdata);
	button->m_Sens.minv.x = 0;
	button->m_Sens.minv.y = 0;
	button->m_Sens.maxv.x = (float)width;
	button->m_Sens.maxv.y = (float)height;
	button->SetTitle(&text);
	button->UpdateSize();
	AddChild(button);
	return button;
}

GUI_Button *GUI_Container::CreateChildButton(char *tex, char *tex_up, char *tex_down, drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = new GUI_Button();
	button->UseAsScriptObject();
	button->Init();
	button->m_FlagsButton |= (DRG_BUTTON_NO_BG | flags);
	button->m_Type = BTN_TYPE_NORMAL;
	button->SetCallBack(callback, cbdata);
	button->m_Textures[0] = drgTexture::LoadFromFile(tex);
	if (tex_up != NULL)
		button->m_Textures[1] = drgTexture::LoadFromFile(tex_up);
	if (tex_down != NULL)
		button->m_Textures[2] = drgTexture::LoadFromFile(tex_down);
	if (button->m_Textures[1] == NULL)
		button->m_Textures[1] = button->m_Textures[0];
	if (button->m_Textures[2] == NULL)
		button->m_Textures[2] = button->m_Textures[0];
	button->m_Sens.minv.x = 0;
	button->m_Sens.minv.y = 0;
	button->m_Sens.maxv.x = (float)button->m_Textures[0]->GetWidth();
	button->m_Sens.maxv.y = (float)button->m_Textures[0]->GetHeight();
	button->UpdateSize();
	AddChild(button);
	return button;
}

GUI_Button *GUI_Container::CreateChildButton(DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down, drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = GUI_Button::CreateChildButton(this, icon, icon_up, icon_down, callback, cbdata, flags);
	AddChild(button);
	return button;
}

GUI_Button *GUI_Container::CreateChildCheckBox(drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = GUI_Button::CreateChildButton(this, ICON_CHECK, ICON_CHECK_UP, ICON_CHECK_DOWN, callback, cbdata, flags);
	button->m_Type = BTN_TYPE_CHECK;
	button->m_FlagsButton |= DRG_BUTTON_STICKY;
	button->UpdateSize();
	AddChild(button);
	return button;
}

GUI_Button *GUI_Container::CreateChildRadioButton(drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = GUI_Button::CreateChildButton(this, ICON_CHECK, ICON_CHECK_UP, ICON_CHECK_DOWN, callback, cbdata, flags);
	button->m_Type = BTN_TYPE_RADIO;
	button->m_FlagsButton |= DRG_BUTTON_STICKY;
	AddChild(button);
	return button;
}

// GUI_RadioButtonList* GUI_Container::CreateChildRadioButtonList(unsigned int flags)
//{
// GUI_RadioButtonList* radioList = GUI_RadioButtonList::CreateChildList(this, flags);
// AddChild(radioList);
// return radioList;
//}

GUI_Scroll *GUI_Container::CreateChildScroll(GUI_ScrollType type, float val, float min, float max, int flags)
{
	GUI_Scroll *scroll = GUI_Scroll::CreateChildScroll(this, type, val, min, max, flags);
	AddChild(scroll);
	return scroll;
}

GUI_Slider *GUI_Container::CreateChildSlider(int width, int height, float val, float min, float max, int flags)
{
	GUI_Slider *slider = GUI_Slider::CreateChildSlider(this, width, height, val, min, max, flags);
	AddChild(slider);
	return slider;
}

GUI_List *GUI_Container::CreateChildList(unsigned int flags)
{
	GUI_List *list = new GUI_List();
	list->UseAsScriptObject();
	list->Init();
	list->m_FlagsList |= flags;
	list->m_Sens.minv.x = 0.0f;
	list->m_Sens.minv.y = 0.0f;
	list->m_Sens.maxv.x = 0.0f;
	list->m_Sens.maxv.y = 0.0f;
	list->UpdateSize();
	AddChild(list);
	return list;
}

GUI_DropList *GUI_Container::CreateChildDropList(unsigned int flags)
{
	GUI_DropList *list = new GUI_DropList();
	list->UseAsScriptObject();
	list->Init();
	list->m_FlagsList |= flags;
	list->m_Sens.minv.x = 0.0f;
	list->m_Sens.minv.y = 0.0f;
	list->m_Sens.maxv.x = 0.0f;
	list->m_Sens.maxv.y = 0.0f;
	list->UpdateSize();
	AddChild(list);
	return list;
}

// GUI_Screen* GUI_Container::CreateChildScreen(unsigned int flags)
//{
//	GUI_Screen* screen = new GUI_Screen();
//	screen->UseAsScriptObject();
//	screen->Init();
//	screen->m_FlagsScreen |= flags;
//	screen->m_Sens.minv.x = 0.0f;
//	screen->m_Sens.minv.y = 0.0f;
//	screen->m_Sens.maxv.x = 0.0f;
//	screen->m_Sens.maxv.y = 0.0f;
//	AddChild(screen);
//	screen->UpdateSize();
//	return screen;
// }

// GUI_ScreenXML* GUI_Container::CreateChildScreen(char* filename, unsigned int flags)
//{
//	GUI_ScreenXML* screen = new GUI_ScreenXML();
//	screen->UseAsScriptObject();
//	screen->Init();
//	screen->m_FlagsScreen |= flags;
//	screen->m_Sens.minv.x = 0.0f;
//	screen->m_Sens.minv.y = 0.0f;
//	screen->m_Sens.maxv.x = 0.0f;
//	screen->m_Sens.maxv.y = 0.0f;
//	screen->LoadScreen(filename);
//	AddChild(screen);
//	screen->UpdateSize();
//	return screen;
// }

GUI_Container *GUI_Container::CreateChildContainer(DRG_CONTAINER_TYPE type, unsigned int flags)
{
	GUI_Container *scroll = CreateContainer(this, type, flags);
	AddChild(scroll);
	return scroll;
}

GUI_Table *GUI_Container::CreateChildTable(unsigned int flags)
{
	GUI_Table *table = new GUI_Table();
	AddChild(table);

	table->UseAsScriptObject();
	table->Init();
	table->SetFlags(flags);

	return table;
}

GUI_StatusBar *GUI_Container::CreateChildStatusBar()
{
	GUI_StatusBar *statusBar = GUI_StatusBar::CreateStatusBar(this, GetDrawContext()->GetStyle()->GetDefaultFont());
	AddChild(statusBar);
	return statusBar;
}

GUI_Menu *GUI_Container::CreateChildMenu()
{
	drgFont *font = GetDrawContext()->GetStyle()->GetDefaultFont();
	if (m_Menu != NULL)
		delete m_Menu;
	m_Menu = GUI_Menu::CreateMenu(font, false, 0, 0.0f);
	m_Menu->SetParent(this);
	m_Menu->m_ContainParent = this;

	m_Menu->m_Sens.minv.x = 0.0f;
	m_Menu->m_Sens.maxv.x = WidthBox();
	m_Menu->m_Sens.minv.y = -(m_Menu->m_Sens.minv.y + (((float)font->GetFontSize()) * 1.5f));
	m_Menu->m_Sens.maxv.y = 0.0f;
	m_Menu->UpdateSize();
	m_Menu->ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);
	UpdateSize();

	return m_Menu;
}

GUI_Property *GUI_Container::CreateChildProperty(drgProperty *prop)
{
	assert(prop);

	GUI_Property *guiProp = prop->CreateGUIWidget();
	if (guiProp != NULL)
	{
		guiProp->SetFlags(DRG_WIDGET_RESIZE_HFILL);

		AddChild(guiProp);

		guiProp->SetProperty(prop);
		guiProp->Init();
		guiProp->UpdateSize();
		guiProp->UseAsScriptObject();
	}

	return guiProp;
}

GUI_Property *GUI_Container::CreateChildPropertyWithIndex(drgObjectProp *objWithProperties, int propertyIndex)
{
	assert(objWithProperties);

	drgProperty *properT = objWithProperties->GetProperty(propertyIndex);
	return CreateChildProperty(properT);
}

void GUI_Container::CreateTitleBar(unsigned int flags, string16 titleText)
{
	assert(m_TitleBar == NULL);

	// We should only have one title bar at a time, since we need to offset everything down by it
	m_TitleBar = GUI_TitleBar::CreateTitleBar(this, GetDrawContext()->GetStyle()->GetDefaultFont(),
											  flags | DRG_TITLEBAR_NO_CLOSE | DRG_TITLEBAR_NO_MAXIMIZE, &titleText);

	// Move the bar up by it's height, we'll later shrink the view to keep it from altering
	// other control's positions.
	m_TitleBar->Move(0.0f, -m_TitleBar->HeightView());
	m_MinHeight += m_TitleBar->HeightView();

	UpdateSize();

	m_MinWidth = DRG_MAX(m_MinWidth, DEFAULT_TITLE_FONT_WIDTH);
}

void GUI_Container::RemoveTitleBar()
{
	if (m_TitleBar)
	{
		m_MinHeight -= m_TitleBar->HeightView();

		if (m_Manager->GetSelectedWidget() == this)
		{
			m_Manager->SetSelectedWidget(NULL);
		}
		if (m_Manager->GetFocusedWidget() == this)
		{
			m_Manager->SetFocusedWidget(NULL);
		}
		delete m_TitleBar;
		m_TitleBar = NULL;
	}
}

void GUI_Container::HandleCollapseButtonDown(drgObjectBase *obj, void *data)
{
	GUI_Button *button = (GUI_Button *)obj;
	GUI_Container *container = (GUI_Container *)button->GetParent();
	if (container->m_CanCollapse)
	{
		container->SetCollapsed(button->GetState() == DRG_BUTTON_STATE_DOWN);
	}
}

void GUI_Container::SetCanCollapse(bool canCollapse)
{
	m_CanCollapse = canCollapse;
	if (false == canCollapse)
		SetCollapsed(false);
	if (m_CollapseButton != NULL)
		m_CollapseButton->SetDisabled(!canCollapse);
}

void GUI_Container::SetCollapseTitle(const char *title)
{
	m_CollapseTitle.setstr(0, string16(title), (unsigned short)'\0', true);
}

void GUI_Container::SetCollapseTitle(string16 title)
{
	m_CollapseTitle = title;
}

float GUI_Container::GetCollapsedHeight()
{
	return m_CollapsedHeight;
}

void GUI_Container::SetCollapsed(bool collapsed)
{
	m_IsCollapsed = collapsed;

	if (collapsed)
	{
		m_UnCollapsedHeight = HeightBox();
		m_Sens.maxv.y = m_Sens.minv.y + GetCollapsedHeight();
	}
	else
		m_Sens.maxv.y = m_Sens.minv.y + m_UnCollapsedHeight;

	UpdateSize();
}

void GUI_Container::CheckForReceievers()
{
	if (m_Manager && m_Manager->HasReceiverWidget())
		return; // We've already found our receiver widget

	GUI_Widget *dragWidget = m_Manager->GetDragWidget();
	if (dragWidget && dragWidget->HasFlagWidget(DRG_WIDGET_DOCKER))
	{
		if (HasFlagWidget(DRG_WIDGET_DOCK_RECEIVER))
		{
			drgVec2 *dragGlobalPos = drgInputMouse::GetGlobalPos();

			// Make sure if we it's in our bounds.
			if (IsPointInGlobalBounds(dragGlobalPos))
			{
				drgVec2 myGlobalPos;
				drgVec2 localDropPos;
				GetNativePosView(&myGlobalPos);
				localDropPos.Set(dragGlobalPos->x - myGlobalPos.x, dragGlobalPos->y - myGlobalPos.y);

				//
				// Make sure it's in our docking ranges
				//

				if (CanReceiveWidget(dragWidget, &localDropPos) && dragWidget->IsValidReceiver(this))
				{
					m_Manager->SetReceiverWidget(this);
				}
			}
		}
	}
}

bool GUI_Container::IndicateDockingLocations(GUI_Widget *dragWidget, drgVec2 *pos, drgBBox *boxToDraw)
{
	assert(pos);
	assert(dragWidget);
	assert(boxToDraw);

	//
	// Convert a global position to a local position
	//

	drgVec2 myGlobalPos;
	GetNativePosView(&myGlobalPos);
	drgVec2 dragLocalPos(pos->x - myGlobalPos.x, pos->y - myGlobalPos.y);

	//
	// Indicate the proper draw bounds.
	//

	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		return IndicateHorizontalDockingLocations(dragWidget, pos, boxToDraw);
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		return IndicateVerticalDockingLocations(dragWidget, pos, boxToDraw);
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_ARBITRARY)
	{
		boxToDraw->minv.x = 0.0f;
		boxToDraw->minv.y = 0.0f;
		boxToDraw->maxv.x = WidthView();
		boxToDraw->maxv.y = HeightView();

		WidgetOffsetRectInt(boxToDraw);

		return true;
	}

	return false;
}

bool GUI_Container::IndicateHorizontalDockingLocations(GUI_Widget *widget, drgVec2 *pos, drgBBox *boxToDraw)
{
	//
	// We only care about the upper extreme or the lower extreme
	//

	float avgHeight = m_Children.GetCount() > 0 ? (m_Children.GetCount() + 1 / HeightView()) : HeightView();
	if (DRG_ABS(pos->y - m_View.minv.y) <= m_DockRange * HeightView()) // Are they at our upper extreme?
	{
		boxToDraw->minv.y = 0.0f;
		boxToDraw->minv.x = 0.0f;
		boxToDraw->maxv.y = avgHeight;
		boxToDraw->maxv.x = WidthView();
	}
	else if (DRG_ABS(pos->y - m_View.maxv.y) <= m_DockRange * HeightView())
	{
		boxToDraw->minv.y = HeightView() - avgHeight;
		boxToDraw->minv.x = 0.0f;
		boxToDraw->maxv.y = HeightView();
		boxToDraw->maxv.x = WidthView();
	}
	else
	{
		return false;
	}

	WidgetOffsetRect(boxToDraw);
	return true;
}

bool GUI_Container::IndicateVerticalDockingLocations(GUI_Widget *widget, drgVec2 *pos, drgBBox *boxToDraw)
{
	//
	// We only care about the left extreme or the right extreme
	//

	float avgWidth = m_Children.GetCount() > 0 ? (m_Children.GetCount() + 1 / WidthView()) : WidthView();
	if (DRG_ABS(pos->x - m_View.minv.x) <= m_DockRange * WidthView()) // Are they at our left extreme?
	{
		boxToDraw->minv.y = 0.0f;
		boxToDraw->minv.x = 0.0f;
		boxToDraw->maxv.y = HeightView();
		boxToDraw->maxv.x = avgWidth;
	}
	else if (DRG_ABS(pos->y - m_View.maxv.y) <= m_DockRange * WidthView())
	{
		boxToDraw->minv.y = 0.0f;
		boxToDraw->minv.x = WidthView() - avgWidth;
		boxToDraw->maxv.y = HeightView();
		boxToDraw->maxv.x = WidthView();
	}
	else
	{
		return false;
	}

	WidgetOffsetRect(boxToDraw);
	return true;
}

void GUI_Container::ReceiveWidgetHorizontal(GUI_Widget *widget, drgVec2 *localMousePos)
{
	if (localMousePos->y < HeightView() * 0.5f)
	{
		m_Children.AddHead(widget);
		widget->SetParent(this);
	}
	else
	{
		m_Children.AddTail(widget);
		widget->SetParent(this);
	}
}

void GUI_Container::ReceiveWidgetVertical(GUI_Widget *widget, drgVec2 *localMousePos)
{
	if (localMousePos->x < WidthView() * 0.5f)
	{
		m_Children.AddHead(widget);
		widget->SetParent(this);
	}
	else
	{
		m_Children.AddTail(widget);
		widget->SetParent(this);
	}
}

bool GUI_Container::CanReceiveWidget(GUI_Widget *widget, drgVec2 *localDropPos)
{
	if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN)
	{
		//
		// We only care about the top and bottom extents
		//

		if (DRG_ABS(m_View.minv.y - localDropPos->y) <= m_DockRange * HeightView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.maxv.y - localDropPos->y) <= m_DockRange * HeightView())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (m_ContainerResizeType == DRG_CONTAINER_RESIZE_VERTICAL_ALIGN)
	{
		//
		// We only care about the top and bottom extents
		//

		if (DRG_ABS(m_View.minv.x - localDropPos->x) <= m_DockRange * WidthView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.maxv.x - localDropPos->x) <= m_DockRange * WidthView())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (DRG_ABS(m_View.minv.x - localDropPos->x) <= m_DockRange * WidthView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.maxv.x - localDropPos->x) <= m_DockRange * WidthView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.minv.y - localDropPos->y) <= m_DockRange * HeightView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.maxv.y - localDropPos->y) <= m_DockRange * HeightView())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

GUI_DivContainer::GUI_DivContainer()
{
	m_WidgetType = GUI_TYPE_DIV_CONTAINER;
}

GUI_DivContainer::~GUI_DivContainer()
{
	Destroy();
}

void GUI_DivContainer::Init()
{
	m_DivAmnt = 0.5f;
	m_DivSize = 4.0f;
	m_FlagsContainer = 0;
	m_Type = DRG_CONTAINER_VERT;
	m_DivPane[0] = NULL;
	m_DivPane[1] = NULL;
	m_DivPaneEnable[0] = true;
	m_DivPaneEnable[1] = true;

	InitCollapseButton();
}

void GUI_DivContainer::Reinit()
{
}

void GUI_DivContainer::Destroy()
{
	GUI_Container::Destroy();
}

void GUI_DivContainer::Draw(GUI_Drawing *draw)
{
	//
	// Draw our div panes, make sure we setup the clip rects correctly.
	//

	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	AccordionDrawChildren(draw);

	if (m_DivPaneEnable[0] && m_DivPaneEnable[1])
	{
		draw->GetStyle()->Divider(this, IsSelected());
	}

	draw->GetRenderCache()->SetClipRect(top, bot, left, right);

	GUI_Widget::Draw(draw);
}

void GUI_DivContainer::AccordionDrawChildren(GUI_Drawing *draw)
{
	if (!IsCollapsed())
	{
		if (m_DivPane[0] && m_DivPaneEnable[0])
		{
			m_DivPane[0]->Draw(draw);
		}
		if (m_DivPane[1] && m_DivPaneEnable[1])
		{
			m_DivPane[1]->Draw(draw);
		}
	}

	if (m_CanCollapse)
	{
		draw->GetStyle()->CollapsedTitle(this, m_TitleBar, &m_CollapseTitle, m_CollapseButton);
		if (m_CollapseButton != NULL)
		{
			m_CollapseButton->Draw(draw);
		}
	}
}

void GUI_DivContainer::ReceiverDraw(GUI_Drawing *draw)
{
	if (m_Manager == NULL || m_Manager->HasDragWidget() == false)
		return;

	GUI_Widget *dragWidget = m_Manager->GetDragWidget();

	drgVec2 *mouseGlobalPos;
	drgVec2 myGlobalPos(0.0f, 0.0f);
	drgVec2 localPos(0.0f, 0.0f);

	drgBBox boxToDraw;

	// Get the mouse global position
	mouseGlobalPos = drgInputMouse::GetGlobalPos();
	GetNativePosView(&myGlobalPos);

	// Get the drag widget's position relative to myself
	localPos.x = mouseGlobalPos->x - myGlobalPos.x;
	localPos.y = mouseGlobalPos->y - myGlobalPos.y;

	// Determine what pane they will receieve.
	if (IndicateDockingLocations(dragWidget, &localPos, &boxToDraw))
	{
		short top = 0, bot = 0, left = 0, right = 0;
		if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
		{
			draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
			drgBBox rOffs(m_View);
			WidgetOffsetRect(&rOffs);

			short newTop = DRG_MAX((short)(rOffs.minv.y), top);
			short newBottom = DRG_MIN((short)(rOffs.maxv.y), bot);
			short newLeft = DRG_MAX((short)(rOffs.minv.x), left);
			short newRight = DRG_MIN((short)(rOffs.maxv.x), right);

			draw->GetRenderCache()->SetClipRect(newTop, newBottom, newLeft, newRight);
		}

		// Now we can draw the box
		drgColor color = draw->GetStyle()->GetDefaultColor(DOCKING_INDICATION_COLOR);
		draw->DrawRectDirect(boxToDraw, &color);

		if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
		{
			draw->GetRenderCache()->SetClipRect(top, bot, left, right);
		}
	}
}

void GUI_DivContainer::ReceiveWidget(GUI_Widget *widget)
{
	assert(widget);

	drgVec2 *mousePos = drgInputMouse::GetGlobalPos();
	drgVec2 myGlobalPos(0.0f, 0.0f);
	drgVec2 localPos(0.0f, 0.0f);

	//
	// Convert the mouse pos to a local position
	//

	GetNativePosView(&myGlobalPos);
	localPos.Set(mousePos->x - myGlobalPos.x, mousePos->y - myGlobalPos.y);

	//
	// Figure out the target pane index
	//

	int paneIndex = -1;
	if (m_Type == DRG_CONTAINER_VERT)
	{
		if (localPos.x <= (WidthView() * 0.5f + (m_DivSize * 0.5f)))
		{
			paneIndex = 0;
		}
		else
		{
			paneIndex = 1;
		}
	}
	else if (m_Type == DRG_CONTAINER_HORIZ)
	{
		if (localPos.y <= (HeightView() * 0.5f + (m_DivSize * 0.5f)))
		{
			paneIndex = 0;
		}
		else
		{
			paneIndex = 1;
		}
	}

	//
	// Swap containers if necessary
	//

	if (m_DivPaneEnable[0] == false || m_DivPaneEnable[1] == false)
	{
		if (paneIndex == 0 && m_DivPaneEnable[0] == true)
		{
			m_DivPane[1] = m_DivPane[0];
			m_DivPaneEnable[1] = true;
		}
		else if (paneIndex == 1 && m_DivPaneEnable[1] == true)
		{
			m_DivPane[0] = m_DivPane[1];
			m_DivPaneEnable[0] = true;
		}

		assert(widget->GetWidgetType() == GUI_TYPE_CONTAINER ||
			   widget->GetWidgetType() == GUI_TYPE_TAB_CONTAINER ||
			   widget->GetWidgetType() == GUI_TYPE_DIV_CONTAINER ||
			   widget->GetWidgetType() == GUI_TYPE_TABLE ||
			   widget->GetWidgetType() == GUI_TYPE_PANE);

		m_DivPane[paneIndex] = (GUI_Container *)widget;
		m_DivPaneEnable[paneIndex] = true;
		m_DivAmnt = 0.5f;
		widget->SetParent(this);
	}
	else
	{
		//
		// We have a container in each pane. No biggie. We just need to make the target pane a div container
		// and add the previous child and the new widget as it's children.
		//

		// Use the create function that doesn't setup it's child panes. We'll be overriding them.
		GUI_DivContainer *newDiv = CreateDivContainer(this, m_Type, 0);

		int oppositePane = paneIndex == 0 ? 1 : 0;
		float width = m_DivPane[paneIndex]->WidthBox();
		float height = m_DivPane[paneIndex]->HeightBox();
		float xPos = m_DivPane[paneIndex]->PosXBox();
		float yPos = m_DivPane[paneIndex]->PosYBox();

		// Put the old pane in the opposite pane index in the child div container
		newDiv->m_DivPane[oppositePane] = m_DivPane[paneIndex];
		newDiv->m_DivPaneEnable[oppositePane] = true;
		newDiv->m_DivPane[oppositePane]->SetParent(newDiv);

		// Put the new pane in the pane index of the child div container
		newDiv->m_DivPane[paneIndex] = (GUI_Container *)widget;
		newDiv->m_DivPaneEnable[paneIndex] = true;
		newDiv->m_DivPane[paneIndex]->SetParent(newDiv);

		// Add the child div container as the new pane at the pane index
		m_DivPane[paneIndex] = newDiv;

		// Set the new div pane's parent, and update it's size
		newDiv->m_DivAmnt = 0.5f;
		newDiv->SetParent(this);
		newDiv->SetWidth(width);
		newDiv->SetHeight(height);
		newDiv->SetPos(xPos, yPos);
	}

	UpdateSize();
}

void GUI_DivContainer::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (m_DivPane[0] && m_DivPaneEnable[0])
		m_DivPane[0]->CallKeyClick(winid, key, charval, down);
	if (m_DivPane[1] && m_DivPaneEnable[1])
		m_DivPane[1]->CallKeyClick(winid, key, charval, down);
}

bool GUI_DivContainer::CallMouseDown(unsigned int winid, int button)
{
	CallMouseDownCollapseButton(winid, button);

	// Let our child panes handle it.
	bool handled = false;
	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		handled = m_DivPane[0]->CallMouseDown(winid, button);
	}
	if (m_DivPane[1] && !handled && m_DivPaneEnable[1])
	{
		handled = m_DivPane[1]->CallMouseDown(winid, button);
	}

	// Check if the user is selected our div bar.
	if ((m_Manager && m_Manager->HasSelectedWidget()) || button != DRG_MOUSE_BUTTON_LEFT)
	{
		return handled;
	}

	drgBBox rOffs;
	GetDividerBox(&rOffs);
	WidgetOffsetRect(&rOffs);

	if (drgInputMouse::IsCursorInside(&rOffs))
	{
		SetFocused();
		SetSelected();
		handled = true;
	}

	return handled;
}

bool GUI_DivContainer::CallMouseClick(unsigned int winid, int button)
{
	if (IsSelected())
	{
		m_Manager->SetSelectedWidget(NULL);
	}

	if (IsCursorInBounds() == false)
		return false;

	bool clicked = false;

	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		clicked = m_DivPane[0]->CallMouseClick(winid, button);
	}
	if (m_DivPane[1] && !clicked && m_DivPaneEnable[1])
	{
		clicked = m_DivPane[1]->CallMouseClick(winid, button);
	}

	return clicked;
}

void GUI_DivContainer::CallMouseUp(unsigned int winid, int button)
{
	if (IsSelected())
	{
		m_Manager->SetSelectedWidget(NULL);
	}

	if (IsHidden())
		return;

	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		m_DivPane[0]->CallMouseUp(winid, button);
	}
	if (m_DivPane[1] && m_DivPaneEnable[1])
	{
		m_DivPane[1]->CallMouseUp(winid, button);
	}
}

bool GUI_DivContainer::CallMouseDoubleClick(unsigned int winid, int button)
{
	if (IsCursorInBounds() == false)
		return false;

	bool clicked = false;

	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		clicked = m_DivPane[0]->CallMouseDoubleClick(winid, button);
	}
	if (m_DivPane[1] && !clicked && m_DivPaneEnable[1])
	{
		clicked = m_DivPane[1]->CallMouseDoubleClick(winid, button);
	}

	return clicked;
}

void GUI_DivContainer::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	if (IsCursorInBounds() == false)
		return;

	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		m_DivPane[0]->CallMouseWheel(winid, pos, rel);
	}
	if (m_DivPane[1] && m_DivPaneEnable[1])
	{
		m_DivPane[1]->CallMouseWheel(winid, pos, rel);
	}
}

void GUI_DivContainer::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (IsCursorInBounds() == false)
		return;

	if (m_DivPane[0] && m_DivPaneEnable[0])
		m_DivPane[0]->CallMouseMove(winid, pos, rel);
	if (m_DivPane[1] && m_DivPaneEnable[1])
		m_DivPane[1]->CallMouseMove(winid, pos, rel);

	if (IsSelected())
	{
		drgBBox rOffs(m_Sens);
		WidgetOffsetRect(&rOffs);

		//
		// We need to clamp to our container's widths and heights.
		//

		if (m_Type == DRG_CONTAINER_VERT)
		{
			m_DivAmnt = DRG_CLAMP_NORM((pos->x - rOffs.minv.x) / WidthBox());
			m_DivAmnt = ClampDivAmount(m_DivAmnt);
		}
		else if (m_Type == DRG_CONTAINER_HORIZ)
		{
			m_DivAmnt = DRG_CLAMP_NORM((pos->y - rOffs.minv.y) / HeightBox());
			m_DivAmnt = ClampDivAmount(m_DivAmnt);
		}

		UpdateSize();
	}
}

void GUI_DivContainer::CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		m_DivPane[0]->CallMouseDrag(winid, pos, rel);
	}
	if (m_DivPane[1] && m_DivPaneEnable[1])
	{
		m_DivPane[1]->CallMouseDrag(winid, pos, rel);
	}

	if (m_Manager && m_Manager->HasReceiverWidget() == false)
	{
		CheckForReceievers();
	}
}

bool GUI_DivContainer::CallPointerDown(unsigned int winid, int pointerIndex)
{
	//
	// Let our child panes handle it.
	//

	bool handled = false;
	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		handled = m_DivPane[0]->CallPointerDown(winid, pointerIndex);
	}
	if (m_DivPane[1] && !handled && m_DivPaneEnable[1])
	{
		handled = m_DivPane[1]->CallPointerDown(winid, pointerIndex);
	}

	//
	// Check if the user is selected our div bar.
	//

	if ((m_Manager && m_Manager->HasSelectedWidget()) || pointerIndex != DRG_PRIMARY_POINTER)
	{
		return handled;
	}

	drgBBox rOffs;
	GetDividerBox(&rOffs);
	WidgetOffsetRect(&rOffs);

	if (drgInputTouch::GetPointer(pointerIndex)->IsPointerInside(&rOffs))
	{
		SetFocused();
		SetSelected();
		handled = true;
	}

	return handled;
}

bool GUI_DivContainer::CallPointerClick(unsigned int winid, int pointerIndex)
{
	if (IsSelected())
	{
		m_Manager->SetSelectedWidget(NULL);
	}

	if (IsPointerInBounds(pointerIndex) == false)
		return false;

	bool clicked = false;

	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		clicked = m_DivPane[0]->CallPointerClick(winid, pointerIndex);
	}
	if (m_DivPane[1] && !clicked && m_DivPaneEnable[1])
	{
		clicked = m_DivPane[1]->CallPointerClick(winid, pointerIndex);
	}

	return clicked;
}

void GUI_DivContainer::CallPointerUp(unsigned int winid, int pointerIndex)
{
	if (IsSelected())
	{
		m_Manager->SetSelectedWidget(NULL);
	}

	if (IsHidden())
		return;

	if (m_DivPane[0] && m_DivPaneEnable[0])
	{
		m_DivPane[0]->CallPointerUp(winid, pointerIndex);
	}
	if (m_DivPane[1] && m_DivPaneEnable[1])
	{
		m_DivPane[1]->CallPointerUp(winid, pointerIndex);
	}
}

void GUI_DivContainer::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (IsPointerInBounds(pointerIndex) == false)
		return;

	if (m_DivPane[0] && m_DivPaneEnable[0])
		m_DivPane[0]->CallPointerMove(winid, pointerIndex, pos, rel);
	if (m_DivPane[1] && m_DivPaneEnable[1])
		m_DivPane[1]->CallPointerMove(winid, pointerIndex, pos, rel);

	if (IsSelected())
	{
		drgBBox rOffs(m_Sens);
		WidgetOffsetRect(&rOffs);

		//
		// We need to clamp to our container's widths and heights.
		//

		if (m_Type == DRG_CONTAINER_VERT)
		{
			m_DivAmnt = DRG_CLAMP_NORM((pos->x - rOffs.minv.x) / WidthBox());
			m_DivAmnt = ClampDivAmount(m_DivAmnt);
		}
		else if (m_Type == DRG_CONTAINER_HORIZ)
		{
			m_DivAmnt = DRG_CLAMP_NORM((pos->y - rOffs.minv.y) / HeightBox());
			m_DivAmnt = ClampDivAmount(m_DivAmnt);
		}

		UpdateSize();
	}
}

void GUI_DivContainer::UpdateSize()
{
	//
	// Resize our container
	//

	m_View = m_Sens;

	//
	// Make sure our min/max is at least the same as the sum of our divs
	//

	UpdateMinMaxValues();

	//
	// Clamp our div bar again
	//

	m_DivAmnt = ClampDivAmount(m_DivAmnt);

	//
	// Resize our division bar
	//

	float mid;
	if (m_Type == DRG_CONTAINER_VERT)
	{
		//
		// Now we need to resize the divpane
		//

		mid = WidthBox() * m_DivAmnt;
		if (m_DivPaneEnable[0] == false)
			mid = -(m_DivSize * 0.5f); // Make sure you take into account the offset for the div size.

		if (m_DivPaneEnable[1] == false)
			mid = WidthBox() + (m_DivSize * 0.5f); // Make sure you take into account the offset for the div size.

		if (m_DivPane[0])
		{
			m_DivPane[0]->Resize(0.0f, HeightBox(), 0.0f, mid - (m_DivSize * 0.5f));
		}
		if (m_DivPane[1])
		{
			m_DivPane[1]->Resize(0.0f, HeightBox(), mid + (m_DivSize * 0.5f), WidthBox());
		}
	}
	else if (m_Type == DRG_CONTAINER_HORIZ)
	{
		mid = HeightBox() * m_DivAmnt;
		if (m_DivPaneEnable[0] == false)
			mid = -(m_DivSize * 0.5f); // Make sure you take into account the offset for the div size.

		if (m_DivPaneEnable[1] == false)
			mid = HeightBox() + (m_DivSize * 0.5f); // Make sure you take into account the offset for the div size.

		if (m_DivPane[0])
		{
			m_DivPane[0]->Resize(0.0f, mid - (m_DivSize * 0.5f), 0.0f, WidthBox());
		}
		if (m_DivPane[1])
		{
			m_DivPane[1]->Resize(mid + (m_DivSize * 0.5f), HeightBox(), 0.0f, WidthBox());
		}
	}
}

GUI_Container *GUI_DivContainer::GetContainer(DRG_CONTAINER_DIV_POS pos)
{
	return m_DivPane[(int)pos];
}

void GUI_DivContainer::SetContainer(DRG_CONTAINER_DIV_POS pos, GUI_Container *container)
{
	assert(container);
	GUI_Container *oldContainer = m_DivPane[pos];
	if (oldContainer)
	{
		oldContainer->DelayedDelete();
	}

	m_DivPane[pos] = container;
	m_DivPaneEnable[pos] = true;
}

void GUI_DivContainer::EnableContainer(DRG_CONTAINER_DIV_POS pos, bool enable)
{
	if (m_DivPaneEnable[(int)pos] == enable)
		return;
	m_DivPaneEnable[(int)pos] = enable;
	UpdateSize();
}

void GUI_DivContainer::GetDividerBox(drgBBox *bbox)
{
	if (m_Type == DRG_CONTAINER_VERT)
	{
		bbox->minv.x = PosXView() + ((WidthView() * GetMidValue()) - (GetDivSize() * 0.5f));
		bbox->minv.y = PosYView();
		bbox->maxv.x = bbox->minv.x + GetDivSize();
		bbox->maxv.y = bbox->minv.y + HeightView();
	}
	else
	{
		bbox->minv.x = PosXView();
		bbox->minv.y = PosYView() + ((HeightView() * GetMidValue()) - (GetDivSize() * 0.5f));
		bbox->maxv.x = bbox->minv.x + WidthView();
		bbox->maxv.y = bbox->minv.y + GetDivSize();
	}
}

GUI_DivContainer *GUI_DivContainer::CreateDivContainer(GUI_Widget *parent, DRG_CONTAINER_TYPE type, unsigned int flags)
{
	GUI_DivContainer *contain = new GUI_DivContainer();
	contain->UseAsScriptObject();
	contain->Init();

	contain->m_Type = type;
	contain->m_FlagsContainer |= flags;
	contain->SetParent(parent);

	if (flags & DRG_WIDGET_RESIZE_HFILL)
		contain->m_FlagsWidget |= DRG_WIDGET_RESIZE_HFILL;
	if (flags & DRG_WIDGET_RESIZE_VFILL)
		contain->m_FlagsWidget |= DRG_WIDGET_RESIZE_VFILL;

	return contain;
}

#define MIN_DIV_AMNT 0.04f
float GUI_DivContainer::ClampDivAmount(float divAmount)
{
	float returnDiv = 0.0f;
	if (m_Type == DRG_CONTAINER_VERT)
	{
		//
		// Make sure we clamp to the min and max bounds of our child containers
		//

		if (m_DivPaneEnable[0])
		{
			float container0MinDiv = DRG_MAX(m_DivPane[0]->GetMinWidth() / WidthBox(), 0.0f);
			float container0MaxDiv = DRG_MIN(m_DivPane[0]->GetMaxWidth() / WidthBox(), 1.0f);

			returnDiv = DRG_CLAMP(divAmount, container0MinDiv, container0MaxDiv);
		}

		if (m_DivPaneEnable[1])
		{
			float container1MinDiv = DRG_CLAMP(1.0f - DRG_MAX(m_DivPane[1]->GetMinWidth() / WidthBox(), 0.0f), 0.0f, 1.0f);
			float container1MaxDiv = DRG_CLAMP(1.0f - DRG_MIN(m_DivPane[1]->GetMaxWidth() / WidthBox(), 1.0f), 0.0f, 1.0f);

			returnDiv = DRG_CLAMP(returnDiv, container1MaxDiv, container1MinDiv);
		}
	}
	else if (m_Type == DRG_CONTAINER_HORIZ)
	{
		//
		// Make sure we clamp to the min and max bounds of our child containers
		//

		if (m_DivPaneEnable[0])
		{
			float container0MinDiv = DRG_MAX(m_DivPane[0]->GetMinHeight() / HeightBox(), 0.0f);
			float container0MaxDiv = DRG_MIN(m_DivPane[0]->GetMaxHeight() / HeightBox(), 1.0f);

			returnDiv = DRG_CLAMP(divAmount, container0MinDiv, container0MaxDiv);
		}

		if (m_DivPaneEnable[1])
		{
			float container1MinDiv = DRG_CLAMP(1.0f - DRG_MAX(m_DivPane[1]->GetMinHeight() / HeightBox(), 0.0f), 0.0f, 1.0f);
			float container1MaxDiv = DRG_CLAMP(1.0f - DRG_MIN(m_DivPane[1]->GetMaxHeight() / HeightBox(), 1.0f), 0.0f, 1.0f);

			returnDiv = DRG_CLAMP(returnDiv, container1MaxDiv, container1MinDiv);
		}
	}

	//
	// If we're completely clipping out one pane or another, we
	// need to give the pane some space
	//

	if (m_DivPaneEnable[0] && m_DivPaneEnable[1])
	{
		if (returnDiv < MIN_DIV_AMNT)
		{
			returnDiv = MIN_DIV_AMNT;
		}
		else if (returnDiv > 1.0f - MIN_DIV_AMNT)
		{
			returnDiv = 1.0f - MIN_DIV_AMNT;
		}
	}

	return returnDiv;
}

void GUI_DivContainer::AddChild(GUI_Widget *widget)
{
	if (m_DivPaneEnable[0] == false)
	{
		m_DivPane[0] = (GUI_Container *)widget;
		m_DivPaneEnable[0] = true;
	}
	else if (m_DivPaneEnable[1] == false)
	{
		m_DivPane[1] = (GUI_Container *)widget;
		m_DivPaneEnable[1] = true;
	}

	GUI_Container::AddChild(widget);
}

void GUI_DivContainer::RemoveChild(GUI_Widget *widget)
{
	if (widget == m_DivPane[0])
	{
		m_DivPaneEnable[0] = false;
		m_DivPane[0] = NULL;
	}
	else if (widget == m_DivPane[1])
	{
		m_DivPaneEnable[1] = false;
		m_DivPane[1] = NULL;
	}

	GUI_Container::RemoveChild(widget);
}

bool GUI_DivContainer::IndicateDockingLocations(GUI_Widget *widget, drgVec2 *pos, drgBBox *boxToDraw)
{
	if (m_Type == DRG_CONTAINER_VERT)
	{
		float halfWidth = WidthView() * m_DivAmnt;
		if (m_DivPaneEnable[0] == false && m_DivPane[1] != NULL)
		{
			halfWidth = m_DivPane[1]->WidthView() / 2.0f;
		}
		else if (m_DivPaneEnable[1] == false && m_DivPane[0] != NULL)
		{
			halfWidth = m_DivPane[0]->WidthView() / 2.0f;
		}

		if (DRG_ABS(m_View.minv.x - pos->x) <= m_DockRange * WidthView())
		{
			boxToDraw->minv.x = 0.0f;
			boxToDraw->maxv.x = halfWidth - (m_DivSize * 0.5f) - (halfWidth * 0.5f);
			boxToDraw->minv.y = 0.0f;
			boxToDraw->maxv.y = HeightView();
		}
		else if (DRG_ABS(m_View.maxv.x - pos->x) <= m_DockRange * WidthView())
		{
			boxToDraw->minv.x = halfWidth + (m_DivSize * 0.5f) + (halfWidth * 0.5f);
			boxToDraw->maxv.x = WidthView();
			boxToDraw->minv.y = 0.0f;
			boxToDraw->maxv.y = HeightView();
		}
		else
		{
			return false;
		}
	}
	else if (m_Type == DRG_CONTAINER_HORIZ)
	{
		float halfHeight = HeightView() * m_DivAmnt;
		if (m_DivPaneEnable[0] == false && m_DivPane[1] != NULL)
		{
			halfHeight = m_DivPane[1]->HeightView() / 2.0f;
		}
		else if (m_DivPaneEnable[1] == false && m_DivPane[0] != NULL)
		{
			halfHeight = m_DivPane[0]->HeightView() / 2.0f;
		}

		if (DRG_ABS(m_View.minv.y - pos->y) <= m_DockRange * HeightView())
		{
			boxToDraw->minv.x = 0.0f;
			boxToDraw->maxv.x = WidthView();
			boxToDraw->minv.y = 0.0f;
			boxToDraw->maxv.y = halfHeight - (m_DivSize * 0.5f) - (halfHeight * 0.5f);
		}
		else if (DRG_ABS(m_View.maxv.y - pos->y) <= m_DockRange * HeightView())
		{
			boxToDraw->minv.x = 0.0f;
			boxToDraw->maxv.x = WidthView();
			boxToDraw->minv.y = halfHeight + (m_DivSize * 0.5f) + (halfHeight * 0.5f);
			boxToDraw->maxv.y = HeightView();
		}
		else
		{
			return false;
		}
	}

	WidgetOffsetRect(boxToDraw);
	return true;
}

bool GUI_DivContainer::CanReceiveWidget(GUI_Widget *widget, drgVec2 *localDropPos)
{
	if (m_Type == DRG_CONTAINER_VERT)
	{
		if (DRG_ABS(m_View.minv.x - localDropPos->x) <= m_DockRange * WidthView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.maxv.x - localDropPos->x) <= m_DockRange * WidthView())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (m_Type == DRG_CONTAINER_HORIZ)
	{
		if (DRG_ABS(m_View.minv.y - localDropPos->y) <= m_DockRange * HeightView())
		{
			return true;
		}
		else if (DRG_ABS(m_View.maxv.y - localDropPos->y) <= m_DockRange * HeightView())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

void GUI_DivContainer::UpdateMinMaxValues()
{
	float totalMinWidth = 0.0f;
	float totalMinHeight = 0.0f;
	float totalMaxWidth = 0.0f;
	float totalMaxHeight = 0.0f;

	for (int currentPane = 0; currentPane < 2; ++currentPane)
	{
		if (m_DivPane[currentPane] == NULL || m_DivPaneEnable[currentPane] == false)
			continue;

		totalMinWidth += m_DivPane[currentPane]->GetMinWidth();
		totalMinHeight += m_DivPane[currentPane]->GetMinHeight();

		totalMaxWidth += m_DivPane[currentPane]->GetMaxWidth() * 0.5f;
		totalMaxHeight += m_DivPane[currentPane]->GetMaxHeight() * 0.5f;
	}

	SetMinWidth(totalMinWidth);
	SetMinHeight(totalMinHeight);
	SetMaxWidth(totalMaxWidth);
	SetMaxHeight(totalMaxHeight);
}

void GUI_DivContainer::SetupDivPaneClippingRects(int paneIndex, GUI_Drawing *draw, short *top, short *bottom, short *left, short *right)
{
	if (!(m_FlagsWidget & DRG_WIDGET_NOCLIPPING))
	{
		draw->GetRenderCache()->GetClipRect(top, bottom, left, right);
		drgBBox rOffs(m_View);

		//
		// Shorten the rect if necessary
		//

		if (paneIndex == 0 && m_DivPaneEnable[1])
		{
			if (m_Type == DRG_CONTAINER_VERT)
			{
				rOffs.maxv.x = rOffs.maxv.x * m_DivAmnt + (m_DivSize * 0.5f);
			}
			else if (m_Type == DRG_CONTAINER_HORIZ)
			{
				rOffs.maxv.y = rOffs.maxv.y * m_DivAmnt + (m_DivSize * 0.5f);
			}
		}
		else if (paneIndex == 1 && m_DivPaneEnable[0])
		{
			if (m_Type == DRG_CONTAINER_VERT)
			{
				rOffs.minv.x = rOffs.maxv.x * m_DivAmnt - (m_DivSize * 0.5f);
			}
			else if (m_Type == DRG_CONTAINER_HORIZ)
			{
				rOffs.minv.y = rOffs.maxv.y * m_DivAmnt - (m_DivSize * 0.5f);
			}
		}

		WidgetOffsetRect(&rOffs);

		short newTop = DRG_MAX((short)(rOffs.minv.y), *top);
		short newBottom = DRG_MIN((short)(rOffs.maxv.y), *bottom);
		short newLeft = DRG_MAX((short)(rOffs.minv.x), *left);
		short newRight = DRG_MIN((short)(rOffs.maxv.x), *right);

		draw->GetRenderCache()->SetClipRect(newTop, newBottom, newLeft, newRight);
	}
}

GUI_TabContainer::GUI_TabContainer()
{
	m_FlagsContainer = 0;
	m_CurPane = NULL;
	m_TabIsPressed = false;
	m_DragThreshold = DEFAULT_DRAG_THRESHOLD;
	m_DragMovement.Set(0.0f, 0.0f);

	m_WidgetType = GUI_TYPE_TAB_CONTAINER;
}

GUI_TabContainer::~GUI_TabContainer()
{
	Destroy();
}

void GUI_TabContainer::Init()
{
	m_FlagsContainer = 0;
	m_CurPane = NULL;
}

void GUI_TabContainer::Reinit()
{
}

void GUI_TabContainer::Destroy()
{
}

void GUI_TabContainer::Draw(GUI_Drawing *draw)
{
	if (m_CurPane != NULL)
		m_CurPane->Draw(draw);

	// Draw the tab bar no matter how many tabs, so we can still drag and drop them.
	draw->GetStyle()->TabBar(this);

	if (m_TitleBar)
	{
		m_TitleBar->Draw(draw);
	}

	GUI_Widget::Draw(draw);
}

void GUI_TabContainer::ReceiverDraw(GUI_Drawing *draw)
{
	//
	// Draw a tab box to indicate where the tab will be going
	//

	draw->GetStyle()->ReceieverTab(this);
	GUI_Container::ReceiverDraw(draw);
}

void GUI_TabContainer::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (m_CurPane != NULL)
		m_CurPane->CallKeyClick(winid, key, charval, down);

	if (m_TitleBar)
	{
		m_TitleBar->CallKeyClick(winid, key, charval, down);
	}
}

bool GUI_TabContainer::CallMouseDown(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	m_TabIsPressed = false; // We aren't pressed yet.
	bool handled = false;

	//
	// Check if the mouse is down in our bounds.
	//

	if (button == DRG_MOUSE_BUTTON_LEFT)
	{
		//
		// First we check if it's in our tab bar bounds
		//

		drgBBox rOffs;
		GetTabBarBounds(&rOffs);
		WidgetOffsetRect(&rOffs);

		if (drgInputMouse::IsCursorInside(&rOffs))
		{
			handled = true;
			GUI_Pane *pane = (GUI_Pane *)m_Children.GetLast();
			while (pane)
			{
				GetTabButtonBounds(pane, &rOffs);
				WidgetOffsetRect(&rOffs);

				if (drgInputMouse::IsCursorInside(&rOffs))
				{
					m_TabIsPressed = true;
					m_DragMovement.Set(0.0f, 0.0f);

					if (IsPaneSelected(pane) == false)
					{
						SetCurPane(pane);
						return handled;
					}

					break;
				}

				pane = (GUI_Pane *)GetPrev(pane);
			}
			return handled;
		}
	}

	//
	// Now let our pane and title bar handle the input
	//

	if (m_CurPane != NULL)
		handled = m_CurPane->CallMouseDown(winid, button);

	if (m_TitleBar)
	{
		handled = m_TitleBar->CallMouseDown(winid, button);
	}

	return handled;
}

bool GUI_TabContainer::CallMouseClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	bool clicked = false;
	m_TabIsPressed = false;

	if (m_CurPane != NULL)
		clicked = m_CurPane->CallMouseClick(winid, button);

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseClick(winid, button);
	}
	return clicked;
}

void GUI_TabContainer::CallMouseUp(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return;

	m_TabIsPressed = false;

	if (m_CurPane != NULL)
		m_CurPane->CallMouseUp(winid, button);

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseUp(winid, button);
	}
}

bool GUI_TabContainer::CallMouseDoubleClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	bool clicked = false;

	if (m_CurPane != NULL)
		clicked = m_CurPane->CallMouseDoubleClick(winid, button);

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseDoubleClick(winid, button);
	}
	return clicked;
}

void GUI_TabContainer::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	if (IsCursorInBounds() == false)
		return;

	if (m_CurPane != NULL)
		m_CurPane->CallMouseWheel(winid, pos, rel);

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseWheel(winid, pos, rel);
	}
}

void GUI_TabContainer::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (IsCursorInBounds() == false)
		return;

	if (m_CurPane != NULL)
		m_CurPane->CallMouseMove(winid, pos, rel);

	//
	// Determine if we are dragging a tab out.
	//

	if (m_TabIsPressed)
	{
		GUI_Container *container = (GUI_Container *)m_CurPane->GetFirst();
		if (container && container->HasFlagWidget(DRG_WIDGET_DOCKER))
		{
			assert(container->IsContainer());

			m_DragMovement.x += rel->x;
			m_DragMovement.y += rel->y;

			if (m_DragMovement.x > m_DragThreshold ||
				m_DragMovement.y > m_DragThreshold)
			{
				OnTabDragStart(m_CurPane);
			}
		}
	}

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseMove(winid, pos, rel);
	}
}

void GUI_TabContainer::CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	if (m_CurPane != NULL)
		m_CurPane->CallMouseDrag(winid, pos, rel);

	if (m_TitleBar)
	{
		m_TitleBar->CallMouseDrag(winid, pos, rel);
	}

	if (m_Manager && m_Manager->HasReceiverWidget() == false)
	{
		CheckForReceievers();
	}
}

bool GUI_TabContainer::CallPointerDown(unsigned int winid, int pointerIndex)
{
	if (IsDisabled() || IsHidden())
		return false;

	m_TabIsPressed = false; // We aren't pressed yet.
	bool handled = false;

	//
	// Check if the mouse is down in our bounds.
	//

	if (pointerIndex == DRG_PRIMARY_POINTER)
	{
		//
		// First we check if it's in our tab bar bounds
		//

		drgBBox rOffs;
		GetTabBarBounds(&rOffs);
		WidgetOffsetRect(&rOffs);

		if (drgInputTouch::GetPointer(pointerIndex)->IsPointerInside(&rOffs))
		{
			handled = true;
			GUI_Pane *pane = (GUI_Pane *)m_Children.GetLast();
			while (pane)
			{
				GetTabButtonBounds(pane, &rOffs);
				WidgetOffsetRect(&rOffs);

				if (drgInputTouch::GetPointer(pointerIndex)->IsPointerInside(&rOffs))
				{
					m_TabIsPressed = true;
					m_DragMovement.Set(0.0f, 0.0f);

					if (IsPaneSelected(pane) == false)
					{
						SetCurPane(pane);
						return handled;
					}

					break;
				}

				pane = (GUI_Pane *)GetPrev(pane);
			}
			return handled;
		}
	}

	//
	// Now let our pane and title bar handle the input
	//

	if (m_CurPane != NULL)
		handled = m_CurPane->CallPointerDown(winid, pointerIndex);

	if (m_TitleBar)
	{
		handled = m_TitleBar->CallPointerDown(winid, pointerIndex);
	}

	return handled;
}

bool GUI_TabContainer::CallPointerClick(unsigned int winid, int pointerIndex)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsPointerInBounds(pointerIndex) == false)
		return false;

	bool clicked = false;
	m_TabIsPressed = false;

	if (m_CurPane != NULL)
		clicked = m_CurPane->CallPointerClick(winid, pointerIndex);

	if (m_TitleBar)
	{
		m_TitleBar->CallPointerClick(winid, pointerIndex);
	}
	return clicked;
}

void GUI_TabContainer::CallPointerUp(unsigned int winid, int pointerIndex)
{
	if (IsDisabled() || IsHidden())
		return;

	m_TabIsPressed = false;

	if (m_CurPane != NULL)
		m_CurPane->CallPointerUp(winid, pointerIndex);

	if (m_TitleBar)
	{
		m_TitleBar->CallPointerUp(winid, pointerIndex);
	}
}

void GUI_TabContainer::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (IsCursorInBounds() == false)
		return;

	if (m_CurPane != NULL)
		m_CurPane->CallPointerMove(winid, pointerIndex, pos, rel);

	//
	// Determine if we are dragging a tab out.
	//

	if (m_TabIsPressed)
	{
		GUI_Container *container = (GUI_Container *)m_CurPane->GetFirst();
		if (container && container->HasFlagWidget(DRG_WIDGET_DOCKER))
		{
			assert(container->IsContainer());

			m_DragMovement.x += rel->x;
			m_DragMovement.y += rel->y;

			if (m_DragMovement.x > m_DragThreshold ||
				m_DragMovement.y > m_DragThreshold)
			{
				OnTabDragStart(m_CurPane);
			}
		}
	}

	if (m_TitleBar)
	{
		m_TitleBar->CallPointerMove(winid, pointerIndex, pos, rel);
	}
}

void GUI_TabContainer::CheckForReceievers()
{
	GUI_Widget *dragWidget = m_Manager->GetDragWidget();
	if (dragWidget && dragWidget->HasFlagWidget(DRG_WIDGET_DOCKER))
	{
		if (HasFlagWidget(DRG_WIDGET_DOCK_RECEIVER))
		{
			drgVec2 *dragGlobalPos = drgInputMouse::GetGlobalPos();

			// Make sure if we it's in our bounds.
			if (IsPointInGlobalBounds(dragGlobalPos))
			{
				drgVec2 myGlobalPos;
				drgVec2 localDropPos;
				GetNativePosBox(&myGlobalPos);
				localDropPos.Set(dragGlobalPos->x - myGlobalPos.x, dragGlobalPos->y - myGlobalPos.y);

				//
				// Make sure it's in our docking ranges
				//

				float tabBarOffset = (((float)GetDrawContext()->GetStyle()->GetInputFont()->GetFontSize()) + 8.0f);
				if (DRG_ABS(localDropPos.y) <= tabBarOffset)
				{
					m_Manager->SetReceiverWidget(this);
				}
			}
		}
	}
}

void GUI_TabContainer::ReceiveWidget(GUI_Widget *widget)
{
	GUI_Container *container = (GUI_Container *)widget;
	assert(container && container->IsContainer());

	//
	// Get the title for the new pane.
	//

	string16 title;
	if (container->GetTitleBar())
	{
		title = *container->GetTitleBar()->GetTitle();
	}
	else
	{
		title = "";
	}

	//
	// Now we need to create the pane and add it.
	//

	GUI_Container *paneChild = CreateChildTab(title, DRG_PANE_FRAME, container);
	GUI_Pane *pane = (GUI_Pane *)paneChild->GetParent();
	assert(pane->GetWidgetType() == GUI_TYPE_PANE);

	pane->AddFlagWidget(DRG_WIDGET_DOCKER);

	//
	// Now remove it's title bar.
	//

	container->RemoveTitleBar();

	//
	// And set it as the current pane.
	//

	SetCurPane(pane);

	UpdateSize();
}

void GUI_TabContainer::UpdateSize()
{
	m_View = m_Sens;

	if (m_TitleBar)
	{
		m_View.minv.y += m_TitleBar->HeightView();
	}

	UpdateMinMaxValues();
	ResizeForTabButtons();
}

void GUI_TabContainer::UpdateMinMaxValues()
{
	float minWidth = 0.0f;
	float minHeight = 0.0f;
	float maxWidth = 0.0f;
	float maxHeight = 0.0f;

	GUI_Widget *widget = GetFirst();
	while (widget)
	{
		minWidth = DRG_MAX(widget->GetMinWidth(), minWidth);
		minHeight = DRG_MAX(widget->GetMinHeight(), minHeight);
		maxWidth = DRG_MAX(widget->GetMaxWidth(), maxWidth);
		maxHeight = DRG_MAX(widget->GetMaxHeight(), maxHeight);

		widget = GetNext(widget);
	}

	SetMinWidth(DRG_MAX(m_MinWidth, minWidth));
	SetMinHeight(DRG_MAX(m_MinHeight, minHeight));
	SetMaxWidth(DRG_MAX(m_MaxWidth, maxWidth));
	SetMaxHeight(DRG_MAX(m_MaxHeight, maxHeight));
}

void GUI_TabContainer::ResizeForTabButtons()
{
	if (GetDrawContext() && GetDrawContext()->GetStyle())
	{
		float tabBarOffset = (((float)GetDrawContext()->GetStyle()->GetInputFont()->GetFontSize()) + 8.0f);
		m_View.minv.y += tabBarOffset;

		if (m_TitleBar)
		{
			m_TitleBar->Move(0.0f, -tabBarOffset); // We need to move it further up since we're moving the view further down.
		}

		GUI_Container::ResizeChildren();
	}
}

GUI_Container *GUI_TabContainer::CreateChildTab(string16 name, unsigned int flags, GUI_Container *container)
{
	GUI_Pane *pane = new GUI_Pane();
	pane->UseAsScriptObject();
	pane->Init();
	pane->m_FlagsPane |= (flags | DRG_PANE_EXPAND);
	pane->SetParent(this);
	pane->SetCanCollapse(false);
	pane->SetName(name);

	pane->m_FlagsWidget |= DRG_WIDGET_RESIZE_EXPAND;

	if (m_CurPane == NULL)
	{
		m_CurPane = pane;
	}

	//
	// If they provided a container then we add it and set our mins/maxes to theirs.
	// If they didn't provide one, we create one and return it to them.
	//

	if (container)
	{
		pane->AddChild(container);
		pane->SetMinWidth(container->GetMinWidth());
		pane->SetMinHeight(container->GetMinHeight());
	}
	else
	{
		container = CreateContainer(pane, DRG_CONTAINER_ABRITRARY, DRG_WIDGET_RESIZE_EXPAND);
		pane->AddChild(container);
	}

	container->AddFlagWidget(DRG_WIDGET_NOCLIPPING); // The clipping will be done by the tab container.
	// Honestly, there should probably be a container resize type that will fit their min and max to their children... but that's a lot of work
	// and I don't have time for it.

	AddChild(pane);
	return container;
}

GUI_TabContainer *GUI_TabContainer::CreateTabContainer(GUI_Widget *parent, unsigned int flags)
{
	GUI_TabContainer *contain = new GUI_TabContainer();
	contain->UseAsScriptObject();
	contain->Init();

	contain->m_FlagsContainer |= flags;

	if (flags & DRG_WIDGET_RESIZE_HFILL)
		contain->m_FlagsWidget |= DRG_WIDGET_RESIZE_HFILL;
	if (flags & DRG_WIDGET_RESIZE_VFILL)
		contain->m_FlagsWidget |= DRG_WIDGET_RESIZE_VFILL;

	return contain;
}

void GUI_TabContainer::AddChild(GUI_Widget *widget)
{
	assert(widget->GetWidgetType() == GUI_TYPE_PANE);

	GUI_Container::AddChild(widget);
}

void GUI_TabContainer::RemoveChild(GUI_Widget *widget)
{
	GUI_Pane *pane = (GUI_Pane *)m_Children.GetFirst();
	while (pane)
	{
		if (pane == widget)
		{
			m_Children.Remove(pane);
			return;
		}

		pane = (GUI_Pane *)m_Children.GetNext(pane);
	}

	GUI_Container::RemoveChild(widget);
}

void GUI_TabContainer::OnTabDragStart(GUI_Pane *paneToRemove)
{
	m_TabIsPressed = false;

	// Make sure the child is a docker.
	GUI_Container *container = (GUI_Container *)paneToRemove->GetFirst();
	if (!container->HasFlagWidget(DRG_WIDGET_DOCKER))
	{
		return;
	}

	// Find our current pane.
	GUI_Pane *pane = (GUI_Pane *)m_Children.GetFirst();
	m_CurPane = NULL;
	while (pane)
	{
		if (pane == paneToRemove)
		{
			SetCurPane((GUI_Pane *)m_Children.GetPrev(pane));
			if (m_CurPane == NULL)
			{
				SetCurPane((GUI_Pane *)m_Children.GetNext(pane));
			}
			break;
		}
		pane = (GUI_Pane *)m_Children.GetNext(pane);
	}

	// Here we need to prepare our little pane to be a fully fledged container!
	// Basically it need a title bar.
	container->CreateTitleBar(0, *pane->GetName());
	container->UpdateSize();

	m_Manager->SetDragWidget(container);
	RemoveChild(pane);
	pane->DelayedDelete();
}

void GUI_TabContainer::SetCurPane(GUI_Pane *pane)
{
	if (m_CurPane)
	{
		m_CurPane->Hide();
	}

	m_CurPane = pane;
	if (m_CurPane)
	{
		m_CurPane->Show();
		UpdateSize();
		RunCallBack(GUI_CB_TAB_CHANGE);
	}
}

#define TEXT_LEN_BORDER 20.0f
void GUI_TabContainer::GetTabButtonBounds(GUI_Pane *pane, drgBBox *outBounds)
{
	assert(pane && outBounds);

	//
	// First we determine which pane we are going
	//

	GetTabBarBounds(outBounds);

	drgFont *fnt = GetDrawContext()->GetStyle()->GetInputFont();
	GUI_Pane *childPane = (GUI_Pane *)GetLast();
	while (childPane)
	{
		float text_len = (float)fnt->StringSize(childPane->GetName()->str());
		outBounds->maxv.x = outBounds->minv.x + text_len + TEXT_LEN_BORDER;

		if (childPane == pane)
		{
			return;
		}

		outBounds->minv.x = outBounds->maxv.x;
		childPane = (GUI_Pane *)GetPrev(childPane);
	}
}

void GUI_TabContainer::GetTabBarBounds(drgBBox *outBounds)
{
	assert(outBounds);

	//
	// First we determine which pane we are going
	//

	drgFont *fnt = GetDrawContext()->GetStyle()->GetInputFont();
	outBounds->minv.x = PosXView();
	outBounds->minv.y = PosYView() - (((float)fnt->GetFontSize()) + 8.0f);
	outBounds->maxv.x = PosXView() + WidthView();
	outBounds->maxv.y = PosYView();

	if (m_TitleBar)
	{
		outBounds->minv.y += m_TitleBar->HeightView();
		outBounds->maxv.y += m_TitleBar->HeightView();
	}
}


GUI_Pane::GUI_Pane()
{
	m_ScrollH = NULL;
	m_ScrollV = NULL;
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);

	m_WidgetType = GUI_TYPE_PANE;

	m_PaneClipMax = drgVec3(0, 0, 0);
}

GUI_Pane::~GUI_Pane()
{
	Destroy();
}

void GUI_Pane::Init()
{
	m_FlagsPane = 0;

	InitCollapseButton();
	SetCanCollapse(true);
}

void GUI_Pane::Reinit()
{
}

void GUI_Pane::Destroy()
{
	GUI_Container::Destroy();
}

void GUI_Pane::Draw(GUI_Drawing* draw)
{
	if(IsHidden())
		return;
	
	UpdateBorderWidth(draw);

	if(m_FlagsPane&DRG_PANE_FRAME)
		draw->GetStyle()->Frame(this);

	if( m_TitleBar )
	{
		m_TitleBar->Draw( draw );
	}

	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	AccordionDrawChildren(draw);

	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	if(m_ScrollH!=NULL)
		m_ScrollH->Draw(draw);
	if(!IsCollapsed() && m_ScrollV!=NULL)
		m_ScrollV->Draw(draw);
	m_ScrollOffset = ScrollOffsetSave;

	draw->GetRenderCache()->SetClipRect( top, bot, left, right );

	if(m_FlagsPane&DRG_PANE_FRAME)
		draw->GetStyle()->Border(this);

	GUI_Widget::Draw(draw);
}

void GUI_Pane::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if(IsHidden())
		return;
	GUI_Container::CallKeyClick(winid, key, charval, down);
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		m_ScrollH->CallKeyClick(winid, key, charval, down);
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		m_ScrollV->CallKeyClick(winid, key, charval, down);
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
}

bool GUI_Pane::CallMouseDown(unsigned int winid, int button)
{
	if(IsHidden())
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool handled = false;

	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		if(m_ScrollH->CallMouseDown(winid, button))
			handled = true;
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		if(m_ScrollV->CallMouseDown(winid, button))
			handled = true;
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;

	if( handled == false )
		GUI_Container::CallMouseDown(winid, button);

	UpdateSize();
	return handled;
}

bool GUI_Pane::CallMouseClick(unsigned int winid, int button)
{
	if(IsHidden())
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool clicked = false;
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		if(m_ScrollH->CallMouseClick(winid, button))
			clicked = true;
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		if(m_ScrollV->CallMouseClick(winid, button))
			clicked = true;
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
	
	if(clicked==false)
		GUI_Container::CallMouseClick(winid, button);
		
	UpdateSize();
	return clicked;
}

void GUI_Pane::CallMouseUp(unsigned int winid, int button)
{
	if(IsHidden())
		return;

	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		m_ScrollH->CallMouseUp(winid, button);
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		m_ScrollV->CallMouseUp(winid, button);
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
	
	GUI_Container::CallMouseUp(winid, button);
}

bool GUI_Pane::CallMouseDoubleClick(unsigned int winid, int button)
{
	if(IsHidden())
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool clicked = false;
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		if(m_ScrollH->CallMouseDoubleClick(winid, button))
			clicked = true;
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		if(m_ScrollV->CallMouseDoubleClick(winid, button))
			clicked = true;
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
	
	if(clicked==false)
		GUI_Container::CallMouseDoubleClick(winid, button);
		
	UpdateSize();
	return clicked;
}

void GUI_Pane::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if(IsHidden())
		return;

	if( IsCursorInBounds() == false )
		return;

	GUI_Container::CallMouseMove(winid, pos, rel);
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		m_ScrollH->CallMouseMove(winid, pos, rel);
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		m_ScrollV->CallMouseMove(winid, pos, rel);
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
}

void GUI_Pane::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	if(IsHidden())
		return;

	if( IsCursorInBounds() == false )
		return;

	GUI_Container::CallMouseWheel(winid, pos, rel);
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		m_ScrollH->CallMouseWheel(winid, pos, rel);
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		m_ScrollV->CallMouseWheel(winid, pos, rel);
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
}

bool GUI_Pane::CallPointerDown( unsigned int winid, int pointerIndex )
{
	if(IsHidden())
		return false;

	if( IsPointerInBounds( pointerIndex ) == false )
		return false;

	bool handled = false;

	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		if(m_ScrollH->CallPointerDown(winid, pointerIndex))
			handled = true;
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		if(m_ScrollV->CallPointerDown(winid, pointerIndex))
			handled = true;
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;

	if( handled == false )
		GUI_Container::CallPointerDown(winid, pointerIndex);

	UpdateSize();
	return handled;
}

bool GUI_Pane::CallPointerClick( unsigned int winid, int pointerIndex )
{
	if(IsHidden())
		return false;

	if( IsPointerInBounds( pointerIndex ) == false )
		return false;

	bool clicked = false;
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		if(m_ScrollH->CallPointerClick(winid, pointerIndex))
			clicked = true;
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		if(m_ScrollV->CallPointerClick(winid, pointerIndex))
			clicked = true;
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
	
	if(clicked==false)
		GUI_Container::CallPointerClick(winid, pointerIndex);
		
	UpdateSize();
	return clicked;
}

void GUI_Pane::CallPointerMove( unsigned int winid, int pointerIndex, drgVec2* pos, drgVec2* rel )
{
	SetHoverWidget();

	if(IsHidden())
		return;

	if( IsPointerInBounds( pointerIndex ) == false )
		return;

	GUI_Container::CallPointerMove(winid, pointerIndex, pos, rel);
	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		m_ScrollH->CallPointerMove(winid, pointerIndex, pos, rel);
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		m_ScrollV->CallPointerMove(winid, pointerIndex, pos, rel);
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
}

void GUI_Pane::CallPointerUp(unsigned int winid, int pointerIndex)
{
	if(IsHidden())
		return;

	m_ScrollOffset.Set(0.0f, 0.0f, 0.0f);
	drgVec3 ScrollOffsetSave = m_ScrollOffset;
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) && (m_ScrollH != NULL) && (!m_ScrollH->IsHidden()))
	{
		m_ScrollH->CallPointerUp(winid, pointerIndex);
		ScrollOffsetSave.x = -(m_ScrollH->GetValue());
	}
	if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && (m_ScrollV != NULL) && (!m_ScrollV->IsHidden()))
	{
		m_ScrollV->CallPointerUp(winid, pointerIndex);
		ScrollOffsetSave.y = -(m_ScrollV->GetValue());
	}
	m_ScrollOffset = ScrollOffsetSave;
	
	GUI_Container::CallPointerUp(winid, pointerIndex);
}

drgVec3 GUI_Pane::GetScrollOffset()
{
	if(m_Parent != NULL)
		return m_ScrollOffset + m_Parent->GetScrollOffset();
	return m_ScrollOffset;
}
	
drgVec3 GUI_Pane::GetPaneClipMax()
{
	if(m_Parent != NULL)
	{
		drgVec3 parentClipMax = m_Parent->GetPaneClipMax();
		if(parentClipMax.IsZero() == false)
		{
			drgVec3 mostRestrictiveMax;
			mostRestrictiveMax.Min(&m_PaneClipMax, &parentClipMax);
			return mostRestrictiveMax;
		}
	}
	return m_PaneClipMax;
}
	
void GUI_Pane::SetPaneClipMax(drgVec3 max)
{
	m_PaneClipMax = max;
}

GUI_Pane* GUI_Pane::GetParentPane()
{
	return this;
}

void GUI_Pane::SetHomogenous(int enable)
{
	m_FlagsPane |= DRG_PANE_HOMOGENOUS;
	Redraw();
}

void GUI_Pane::SetName(string16 name) 
{ 
	m_Name = name;  
	SetCollapseTitle(name);
}

void GUI_Pane::UpdateSize()
{
	GUI_Container::UpdateSize();

	StackChildren();

	UpdateScrollbars();
}

void GUI_Pane::UpdateScrollbars()
{
	// Determine if we need to add/remove scroll bars for our children.
	if ((m_FlagsPane & DRG_PANE_SCROLL_X) || (m_FlagsPane & DRG_PANE_SCROLL_Y))
	{
		// Get the bounding box that encompasses all our children.
		drgBBox bbox;
		bbox.minv.x = bbox.minv.y = bbox.maxv.x = bbox.maxv.y = 0.0f;
		GUI_Widget* itr = (GUI_Widget*)m_Children.GetLast();
		while(itr)
		{
			itr->AddBBox(&bbox);
			itr = (GUI_Widget*)m_Children.GetPrev(itr);
		}

		// If the box extends past our bounds, then add a scroll bar.
		if ((m_FlagsPane & DRG_PANE_SCROLL_X) && !IsCollapsed() && ((bbox.minv.x<0) || (bbox.maxv.x>WidthView())))
		{
			if(m_ScrollH==NULL)
				m_ScrollH = GUI_Scroll::CreateChildScroll(this, GUI_SCROLLBAR_HORIZ, 0.0f, bbox.minv.x, bbox.maxv.x-WidthView(), DRG_SCROLLBAR_EXPAND);
			m_ScrollH->SetMin(bbox.minv.x);
			m_ScrollH->SetMax(bbox.maxv.x-WidthView());
			m_ScrollH->Show();
			m_ScrollH->SetPos(0.0f, HeightView()-m_ScrollH->HeightView());
			m_ScrollH->UpdateSize();
			m_ScrollOffset.x = -(m_ScrollH->GetValue());
		}
		else if(m_ScrollH!=NULL)
		{
			m_ScrollH->Hide();
			m_ScrollOffset.x = 0.0f;
		}
		if ((m_FlagsPane & DRG_PANE_SCROLL_Y) && !IsCollapsed() && ((bbox.minv.y<0) || (bbox.maxv.y>HeightView())))
		{
			if(m_ScrollV==NULL)
				m_ScrollV = GUI_Scroll::CreateChildScroll(this, GUI_SCROLLBAR_VERT, 0.0f, bbox.minv.y, bbox.maxv.y-HeightView(), DRG_SCROLLBAR_EXPAND);
			m_ScrollV->SetMin(bbox.minv.y);
			m_ScrollV->SetMax(bbox.maxv.y-HeightView());
			m_ScrollV->Show();
			m_ScrollV->SetPos(WidthView()-m_ScrollV->WidthView(), 0.0f);
			m_ScrollV->UpdateSize();
			m_ScrollOffset.y = -(m_ScrollV->GetValue());
		}
		else if(m_ScrollV!=NULL)
		{
			m_ScrollV->Hide();
			m_ScrollOffset.y = 0.0f;
		}
	}
}

void GUI_Pane::StackChildren()
{
	float sumOfChildrenHeights = m_CanCollapse ? GetCollapsedHeight() : 0.0f;
	if(IsCollapsed() == false)
	{
		GUI_Widget* itr = (GUI_Widget*)m_Children.GetLast();
		for(int i = 0; i < (int)m_Children.GetCount(); i++)
		{
			if(itr != NULL)
			{
				if(itr->GetFlags() & DRG_WIDGET_RESIZE_HFILL)
					itr->SetWidth(WidthView() - 2*itr->GetBorderWidth());

				sumOfChildrenHeights += itr->GetBorderWidth();
				itr->SetPos(itr->GetBorderWidth(), sumOfChildrenHeights);
				sumOfChildrenHeights += itr->HeightView() + itr->GetBorderWidth();

				itr = (GUI_Widget*)m_Children.GetPrev(itr);
			}
		}
	}

	if((m_FlagsPane & DRG_PANE_VFILL) == false) 
	{
		//fit to children
		m_View.maxv.y = m_View.minv.y + sumOfChildrenHeights;
		m_View.maxv.y -= GetBorderWidth();
	}
	//m_View = m_Sens;
	//OffsetFrameForBorder();
}

void GUI_Pane::WidgetOffsetRectInt(drgBBox* bbox)
{
	if(m_Parent != NULL)
		m_Parent->WidgetOffsetRectInt(bbox);
	bbox->minv.x += (m_View.minv.x + m_ScrollOffset.x);
	bbox->minv.y += (m_View.minv.y + m_ScrollOffset.y);
	bbox->maxv.x += (m_View.minv.x + m_ScrollOffset.x);
	bbox->maxv.y += (m_View.minv.y + m_ScrollOffset.y);
}

GUI_Pane* GUI_Pane::CreatePane( unsigned int flags )
{
	GUI_Pane* pane = new GUI_Pane();
	pane->UseAsScriptObject();
	pane->Init();
	pane->m_FlagsPane |= flags;
	pane->SetName(string16(""));

	if (flags & DRG_PANE_HFILL)
	{
		pane->m_FlagsWidget |= DRG_WIDGET_RESIZE_HFILL;
	}
	if (flags & DRG_PANE_VFILL)
	{
		pane->m_FlagsWidget |= DRG_WIDGET_RESIZE_VFILL;
	}

	return pane;
}

void GUI_Pane::OffsetFrameForBorder()
{
	m_View.minv.x += GetBorderWidth();
	m_View.minv.y += GetBorderWidth();
	m_View.maxv.x -= GetBorderWidth();
	m_View.maxv.y -= GetBorderWidth();
}
	
void GUI_Pane::UpdateBorderWidth(GUI_Drawing* draw)
{
	if(draw != NULL)
	{
		float newBorderWidth;
		if(m_FlagsPane&DRG_PANE_FRAME)
			newBorderWidth =  draw->GetStyle()->GetBorderWidth();
		else
			newBorderWidth = 0.0f;

		if(newBorderWidth == GetBorderWidth())
			return;

		m_BorderWidth = newBorderWidth;

		OffsetFrameForBorder();
	}
}

//The border exists outside of the m_View rect
float GUI_Pane::GetBorderWidth()
{
	return m_BorderWidth;
}

float GUI_Pane::GetCollapsedHeight() 
{ 
	return m_CollapsedHeight + 2 * GetBorderWidth() + 2.0f; 
}




GUI_Window *GUI_Window::CreateBaseWindow(string16 title, unsigned int width, unsigned int height, GUI_Style *style)
{
	MEM_INFO_SET_NEW;
	GUI_Window *win = new GUI_Window();
	win->UseAsScriptObject();
	win->Init();
	win->m_Sens.minv.x = 0;
	win->m_Sens.minv.y = 0;
	win->m_Sens.maxv.x = (float)width;
	win->m_Sens.maxv.y = (float)height;
	win->m_View = win->m_Sens;
	win->m_BorderBot = 0.0f;
	win->m_BorderSide = 0.0f;
	win->m_ResizeCtrl = 0.0f;
	win->m_Color.Set(64, 64, 64);
	if (!drgEngine::NativeWindowCreate(win, &title, NULL, DRG_WINDOW_BASE_WINDOW))
	{
		delete win;
		return NULL;
	}
	win->SetStyle(style);

	return win;
}

GUI_Window *GUI_Window::CreateNativeWindow(string16 *title, unsigned int width, unsigned int height, drgVec2 *pos, unsigned int windowFlags, GUI_Style *style)
{
	GUI_Window *win = new GUI_Window();
	win->UseAsScriptObject();
	win->Init();
	win->m_Sens.minv.x = 0;
	win->m_Sens.minv.y = 0;
	win->m_Sens.maxv.x = (float)width;
	win->m_Sens.maxv.y = (float)height;
	win->m_View = win->m_Sens;
	win->m_BorderBot = 0.0f;
	win->m_BorderSide = 0.0f;
	win->m_ResizeCtrl = 0.0f;
	win->m_Color.Set(64, 64, 64);

	if (!drgEngine::NativeWindowCreate(win, title, pos, windowFlags))
	{
		delete win;
		return NULL;
	}
	win->SetStyle(style);

	return win;
}

GUI_Window::GUI_Window()
{
	m_NativeData = NULL;
	m_DrawContext = NULL;
	m_TitleBar = NULL;
	m_Menu = NULL;
	m_FlagsWin = 0;
	m_WinOp = DRG_WINOP_NONE;
	m_CurrentActive = NULL;
	m_WidgetType = GUI_TYPE_WINDOW;
	m_BorderBot = 0.0f;
	m_BorderSide = 0.0f;
	m_ResizeCtrl = 0.0f;
	m_CloseCallback = NULL;
	m_CloseCallbackData = NULL;
}

GUI_Window::~GUI_Window()
{
	Destroy();
}

void GUI_Window::Init()
{
	m_WinOp = DRG_WINOP_NONE;
}

void GUI_Window::Reinit()
{
}

void GUI_Window::Destroy()
{
	if (m_Manager && m_NativeData)
	{
		m_Manager->RemoveWindow(this);
	}

	GUI_Container::Destroy();

	if (m_NativeData != NULL)
		drgEngine::NativeWindowCleaup(this);

	m_TitleBar = NULL;
}

void GUI_Window::Draw()
{
	if (m_NativeData == NULL)
	{
		return;
	}

	DRG_PROFILE_FUNK(GUI_Window_Draw, 0xFFFFFF00);

	if (m_DrawContext->GetRenderContext()->GetDrawWidth() != WidthBox() || m_DrawContext->GetRenderContext()->GetDrawHeight() != HeightBox())
	{
		m_DrawContext->GetRenderContext()->SetDrawSize(WidthBox(), HeightBox());
	}

	assert(m_DrawContext);

	DRG_PROFILE_PUSH(_Begin_, 0xFFFFFF00);
	drgEngine::NativeWindowBegin(this);
	m_DrawContext->BeginFrame(this);
	DRG_PROFILE_POP();

	// UI doesn't depth test
	m_DrawContext->GetRenderCache()->EnableDepthTesting(false);
	m_DrawContext->GetRenderCache()->EnableDepthWriting(false);

	// Draw all of our children.
	DRG_PROFILE_PUSH(_Draw_Items__, 0xFFFFFF00);
	DrawChildren(m_DrawContext);
	DRG_PROFILE_POP();

	if (m_CurrentActive != NULL)
	{
		m_CurrentActive->Draw(m_DrawContext);
	}
	if (m_Menu != NULL)
	{
		m_Menu->Draw(m_DrawContext);
	}
	// Draw the receiever if necessary
	if (m_Manager && m_Manager->HasReceiverWidget())
	{
		GUI_Widget *receiverWidget = m_Manager->GetReceiverWidget();
		if (receiverWidget->GetParentWindow() == this)
		{
			receiverWidget->ReceiverDraw(m_DrawContext);
		}
	}

	if (m_Manager && m_Manager->HasDragWidget())
	{
		GUI_Widget *dragWidget = m_Manager->GetDragWidget();
		if (dragWidget->GetParentWindow() == this)
		{
			dragWidget->DragDraw(m_DrawContext);
		}
	}

	if (m_Manager != NULL)
	{
		m_Manager->DrawHoverText(m_DrawContext);
		GUI_Widget *hoverWidget = m_Manager->GetHoverWidget();
		if (NULL != hoverWidget)
		{
			hoverWidget->DrawDebug(m_DrawContext);
		}
	}

	DRG_PROFILE_PUSH(_End_, 0xFFFFFF00);
	m_DrawContext->EndFrame(this);
	DRG_PROFILE_POP();
}

void GUI_Window::Draw(GUI_Drawing *draw)
{
	if (IsHidden())
	{
		return;
	}

	// if (UpdateNeeded(WIDGET(win)))
	//	AG_WindowUpdate(win);

	draw->GetStyle()->Window(this);
	if (m_TitleBar != NULL)
	{
		m_TitleBar->Draw(draw);
	}
	if (m_Menu != NULL)
	{
		m_Menu->Draw(draw);
	}
	DrawChildren(draw);
	GUI_Widget::Draw(draw);
}

void GUI_Window::Maximize()
{
	if (m_NativeData != NULL)
	{
		return;
	}
	if (m_FlagsWin & DRG_WINDOW_MAXIMIZED)
	{
		Resize(m_Saved.minv.y, m_Saved.maxv.y, m_Saved.minv.x, m_Saved.maxv.x);
		m_FlagsWin &= (~DRG_WINDOW_MAXIMIZED);
	}
	else
	{
		m_Saved = m_Sens;
		Resize(0.0f, m_Parent->HeightView(), 0.0f, m_Parent->WidthView());
		m_FlagsWin |= DRG_WINDOW_MAXIMIZED;
	}
}

void GUI_Window::SetModal(GUI_Window *window)
{
	if (window == NULL)
	{
		m_Manager->SetModalWindow(NULL);
		return;
	}
	if (m_NativeData == NULL)
	{
		return;
	}
	// Don't add the window twiced
	if (!m_Children.InList(window))
	{
		return;
	}
	m_Children.Remove(window);
	m_Children.AddHead(window);
	m_Manager->SetModalWindow(window);
}

void GUI_Window::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (winid != GetGUID())
	{
		return;
	}
	if (IsDisabled() || IsHidden())
	{
		return;
	}

	if (m_CurrentActive != NULL)
	{
		m_CurrentActive->CallKeyClick(winid, key, charval, down);
	}
	// if(m_TitleBar != NULL)
	//	m_TitleBar->CallKeyClick(winid, key, charval, down);
	// if(m_Menu != NULL)
	//	m_Menu->CallKeyClick(winid, key, charval, down);
	// GUI_Container::CallKeyClick(winid, key, charval, down);
}

bool GUI_Window::CallMouseDown(unsigned int winid, int button)
{
	if (m_NativeData && winid != GetGUID())
	{
		return false;
	}
	if (IsDisabled() || IsHidden())
	{
		return false;
	}

	bool clicked = false;
	if (m_CurrentActive != NULL)
	{
		clicked = m_CurrentActive->CallMouseDown(winid, button);
	}
	if (clicked == true)
	{
		return clicked;
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
		{
			clicked = m_TitleBar->CallMouseDown(winid, button);
		}
		if (clicked == true)
		{
			return clicked;
		}
		if (m_Menu != NULL)
		{
			clicked = m_Menu->CallMouseDown(winid, button);
		}
		if (clicked == true)
		{
			return clicked;
		}
		clicked = GUI_Container::CallMouseDown(winid, button);

		if (clicked == true)
		{
			return clicked;
		}
	}

	if ((button == DRG_MOUSE_BUTTON_LEFT) && (m_NativeData == NULL))
	{
		drgBBox rOffs(m_Sens);
		rOffs.minv.y = rOffs.maxv.y - m_ResizeCtrl;
		WidgetOffsetRect(&rOffs);

		if (drgInputMouse::IsCursorInside(&rOffs))
		{
			drgVec2 *mousepos = drgInputMouse::GetPos();
			float xRel = mousepos->x - m_Sens.minv.x;

			if (xRel < m_ResizeCtrl)
			{
				m_WinOp = DRG_WINOP_LRESIZE;
			}
			else if (xRel > (WidthView() - m_ResizeCtrl))
			{
				m_WinOp = DRG_WINOP_RRESIZE;
			}
			else if ((m_FlagsWin & DRG_WINDOW_NOVRESIZE) == 0)
			{
				m_WinOp = DRG_WINOP_HRESIZE;
			}

			if (m_WinOp != DRG_WINOP_NONE)
			{
				SetSelected();
			}
			clicked = true;
		}
	}

	return clicked;
}

bool GUI_Window::CallMouseClick(unsigned int winid, int button)
{
	if (m_NativeData && winid != GetGUID())
	{
		return false;
	}
	if (IsDisabled() || IsHidden())
	{
		return false;
	}

	bool clicked = false;
	if (m_CurrentActive != NULL)
	{
		clicked = m_CurrentActive->CallMouseClick(winid, button);
	}
	if (clicked == true)
	{
		return clicked;
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
		{
			clicked = m_TitleBar->CallMouseClick(winid, button);
		}
		if (clicked == true)
		{
			return clicked;
		}
		if (m_Menu != NULL)
		{
			clicked = m_Menu->CallMouseClick(winid, button);
		}
		if (clicked == true)
		{
			return clicked;
		}
		clicked = GUI_Container::CallMouseClick(winid, button);

		if (clicked == true)
		{
			return clicked;
		}
	}

	if (m_Manager && m_Manager->HasDragWidget())
	{
		m_Manager->GetDragWidget()->DragDone();
		m_Manager->SetDragWidget(NULL);
	}

	if ((button == DRG_MOUSE_BUTTON_LEFT) && (m_NativeData == NULL))
	{
		m_WinOp = DRG_WINOP_NONE;
	}
	return clicked;
}

void GUI_Window::CallMouseUp(unsigned int winid, int button)
{
	if (m_NativeData && winid != GetGUID())
	{
		return;
	}
	if (IsDisabled() || IsHidden())
	{
		return;
	}
	if (m_CurrentActive != NULL)
	{
		m_CurrentActive->CallMouseUp(winid, button);
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
		{
			m_TitleBar->CallMouseUp(winid, button);
		}
		if (m_Menu != NULL)
		{
			m_Menu->CallMouseUp(winid, button);
		}
		GUI_Container::CallMouseUp(winid, button);
	}

	if (m_Manager && m_Manager->HasDragWidget())
	{
		m_Manager->GetDragWidget()->DragDone();
		m_Manager->SetDragWidget(NULL);
	}

	if ((button == DRG_MOUSE_BUTTON_LEFT) && (m_NativeData == NULL))
	{
		m_WinOp = DRG_WINOP_NONE;
	}
}

bool GUI_Window::IsCursorInBounds()
{
	drgBBox rOffs(m_Sens);
	if (m_NativeData == 0)
	{
		WidgetOffsetRect(&rOffs);
	}
	if (!drgInputMouse::IsCursorInside(&rOffs))
	{
		return false;
	}
	return true;
}

bool GUI_Window::CallMouseDoubleClick(unsigned int winid, int button)
{
	if (m_NativeData && winid != GetGUID())
	{
		return false;
	}
	if (IsDisabled() || IsHidden())
	{
		return false;
	}

	bool clicked = false;
	if (m_CurrentActive != NULL)
	{
		clicked = m_CurrentActive->CallMouseDoubleClick(winid, button);
	}
	if (clicked == true)
	{
		return clicked;
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
			clicked = m_TitleBar->CallMouseDoubleClick(winid, button);

		if (clicked == true)
			return clicked;

		if (m_Menu != NULL)
			clicked = m_Menu->CallMouseDoubleClick(winid, button);

		if (clicked == true)
			return clicked;

		clicked = GUI_Container::CallMouseDoubleClick(winid, button);

		if (clicked == true)
			return clicked;
	}

	if ((button == DRG_MOUSE_BUTTON_LEFT) && (m_NativeData == NULL))
	{
		m_WinOp = DRG_WINOP_NONE;
	}
	return clicked;
}

void GUI_Window::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();
	if (m_NativeData && winid != GetGUID())
	{
		return;
	}
	if (IsDisabled() || IsHidden())
	{
		return;
	}

	if ((m_NativeData == NULL) && IsSelected() && !(m_FlagsWin & DRG_WINDOW_MAXIMIZED))
	{
		switch (m_WinOp)
		{
		case DRG_WINOP_MOVE:
			Move(rel->x, rel->y);
			break;
		case DRG_WINOP_LRESIZE:
			ResizeRel(0.0f, rel->y, rel->x, 0.0f);
			break;
		case DRG_WINOP_RRESIZE:
			ResizeRel(0.0f, rel->y, 0.0f, rel->x);
			break;
		case DRG_WINOP_HRESIZE:
			ResizeRel(0.0f, rel->y, 0.0f, 0.0f);
			break;
		default:
			break;
		}
	}
	if (m_TitleBar != NULL)
	{
		m_TitleBar->CallMouseMove(winid, pos, rel);
	}
	if (m_Menu != NULL)
	{
		m_Menu->CallMouseMove(winid, pos, rel);
	}
	GUI_Container::CallMouseMove(winid, pos, rel);

	if (m_CurrentActive != NULL)
	{
		m_CurrentActive->CallMouseMove(winid, pos, rel);
	}
}

void GUI_Window::CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	if (winid != GetGUID())
	{
		return;
	}
	GUI_Container::CallMouseDrag(winid, pos, rel);
}

bool GUI_Window::CallPointerDown(unsigned int winid, int pointerIndex)
{
	if (m_NativeData && winid != GetGUID())
	{
		return false;
	}
	if (IsDisabled() || IsHidden())
	{
		return false;
	}

	bool clicked = false;
	if (m_CurrentActive != NULL)
	{
		clicked = m_CurrentActive->CallPointerDown(winid, pointerIndex);
	}
	if (clicked == true)
	{
		return clicked;
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
			clicked = m_TitleBar->CallPointerDown(winid, pointerIndex);

		if (clicked == true)
			return clicked;

		if (m_Menu != NULL)
			clicked = m_Menu->CallPointerDown(winid, pointerIndex);

		if (clicked == true)
			return clicked;

		clicked = GUI_Container::CallPointerDown(winid, pointerIndex);

		if (clicked == true)
			return clicked;
	}

	if (pointerIndex == DRG_PRIMARY_POINTER && (m_NativeData == NULL))
	{
		drgBBox rOffs(m_Sens);
		rOffs.minv.y = rOffs.maxv.y - m_ResizeCtrl;
		WidgetOffsetRect(&rOffs);

		if (drgInputTouch::GetPointer(pointerIndex)->IsPointerInside(&rOffs))
		{
			drgVec2 *pointerPos = drgInputTouch::GetPointer(pointerIndex)->GetPosition();
			float xRel = pointerPos->x - m_Sens.minv.x;

			if (xRel < m_ResizeCtrl)
			{
				m_WinOp = DRG_WINOP_LRESIZE;
			}
			else if (xRel > (WidthView() - m_ResizeCtrl))
			{
				m_WinOp = DRG_WINOP_RRESIZE;
			}
			else if ((m_FlagsWin & DRG_WINDOW_NOVRESIZE) == 0)
			{
				m_WinOp = DRG_WINOP_HRESIZE;
			}

			if (m_WinOp != DRG_WINOP_NONE)
			{
				SetSelected();
			}
			clicked = true;
		}
	}

	return clicked;
}

bool GUI_Window::CallPointerClick(unsigned int winid, int pointerIndex)
{
	if (m_NativeData && winid != GetGUID())
	{
		return false;
	}
	if (IsDisabled() || IsHidden())
	{
		return false;
	}

	bool clicked = false;
	if (m_CurrentActive != NULL)
	{
		clicked = m_CurrentActive->CallPointerClick(winid, pointerIndex);
	}
	if (clicked == true)
	{
		return clicked;
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
			clicked = m_TitleBar->CallPointerClick(winid, pointerIndex);

		if (clicked == true)
			return clicked;

		if (m_Menu != NULL)
			clicked = m_Menu->CallPointerClick(winid, pointerIndex);

		if (clicked == true)
			return clicked;

		clicked = GUI_Container::CallMouseClick(winid, pointerIndex);

		if (clicked == true)
			return clicked;
	}

	if (m_Manager && m_Manager->HasDragWidget())
	{
		m_Manager->GetDragWidget()->DragDone();
		m_Manager->SetDragWidget(NULL);
	}

	if (pointerIndex == DRG_PRIMARY_POINTER && (m_NativeData == NULL))
	{
		m_WinOp = DRG_WINOP_NONE;
	}
	return clicked;
}

void GUI_Window::CallPointerUp(unsigned int winid, int button)
{
	if (m_NativeData && winid != GetGUID())
	{
		return;
	}
	if (IsDisabled() || IsHidden())
	{
		return;
	}

	if (m_CurrentActive != NULL)
	{
		m_CurrentActive->CallPointerUp(winid, button);
	}
	if (m_CurrentActive == NULL)
	{
		if (m_TitleBar != NULL)
		{
			m_TitleBar->CallPointerUp(winid, button);
		}
		if (m_Menu != NULL)
		{
			m_Menu->CallPointerUp(winid, button);
		}
		GUI_Container::CallPointerUp(winid, button);
	}

	if (m_Manager && m_Manager->HasDragWidget())
	{
		m_Manager->GetDragWidget()->DragDone();
		m_Manager->SetDragWidget(NULL);
	}

	if ((button == DRG_PRIMARY_POINTER) && (m_NativeData == NULL))
	{
		m_WinOp = DRG_WINOP_NONE;
	}
}

void GUI_Window::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();

	if (winid != GetGUID())
	{
		return;
	}
	if (IsDisabled() || IsHidden())
	{
		return;
	}

	if ((m_NativeData == NULL) && IsSelected() && !(m_FlagsWin & DRG_WINDOW_MAXIMIZED))
	{
		switch (m_WinOp)
		{
		case DRG_WINOP_MOVE:
			Move(rel->x, rel->y);
			break;
		case DRG_WINOP_LRESIZE:
			ResizeRel(0.0f, rel->y, rel->x, 0.0f);
			break;
		case DRG_WINOP_RRESIZE:
			ResizeRel(0.0f, rel->y, 0.0f, rel->x);
			break;
		case DRG_WINOP_HRESIZE:
			ResizeRel(0.0f, rel->y, 0.0f, 0.0f);
			break;
		default:
			break;
		}
	}
	if (m_TitleBar != NULL)
	{
		m_TitleBar->CallPointerMove(winid, pointerIndex, pos, rel);
	}
	if (m_Menu != NULL)
	{
		m_Menu->CallPointerMove(winid, pointerIndex, pos, rel);
	}
	GUI_Container::CallPointerMove(winid, pointerIndex, pos, rel);

	if (m_CurrentActive != NULL)
	{
		m_CurrentActive->CallPointerMove(winid, pointerIndex, pos, rel);
	}
}

void GUI_Window::CallResize(unsigned int winid, drgVec2 *size)
{
	if (winid != GetGUID())
	{
		return;
	}
	if ((m_NativeData != NULL) && (winid == m_GUID))
	{
		float newWidth = (float)size->x;
		float newHeight = (float)size->y;

		//	drgEngine::NativeContextCreate(this);
		if (m_DrawContext != NULL)
		{
			m_DrawContext->GetRenderContext()->SetDrawSize(newWidth, newHeight);
		}

		m_Sens.minv.x = 0;
		m_Sens.minv.y = 0;
		m_Sens.maxv.x = newWidth;
		m_Sens.maxv.y = newHeight;
		UpdateSize();

		GUI_Widget *itr = (GUI_Widget *)m_Children.GetLast();
		while (itr)
		{
			itr->OnParentResize();
			itr = (GUI_Widget *)m_Children.GetPrev(itr);
		}
	}

	if (m_TitleBar != NULL)
	{
		m_TitleBar->OnParentResize();
	}

	if (m_Menu != NULL)
	{
		m_Menu->OnParentResize();
	}
	GUI_Container::OnParentResize();
}

void GUI_Window::UpdateSize()
{
	m_View = m_Sens; // Make sure this happens before we update our size.
	GUI_Container::UpdateSize();
	if (m_NativeData == NULL)
	{
		float topbarsize = ((m_TitleBar == NULL) ? 0.0f : m_TitleBar->HeightView()) + ((m_Menu == NULL) ? 0.0f : m_Menu->HeightView());
		m_View.minv.x += m_BorderSide;
		m_View.maxv.x -= m_BorderSide;
		m_View.minv.y += topbarsize;
		m_View.maxv.y -= m_BorderBot;
		if (m_Menu)
			m_Menu->Resize(-m_Menu->HeightView(), 0.0f, 0.0f, WidthBox());
		if (m_TitleBar)
			m_TitleBar->Resize(-topbarsize, -topbarsize + m_TitleBar->HeightView(), 0.0f, WidthBox());
	}
	else if (m_Menu)
	{
		m_View.minv.y += m_Menu->HeightView();
		m_Menu->Resize(-m_Menu->HeightView(), 0.0f, 0.0f, WidthBox());
	}
}

void GUI_Window::SetPos(float x, float y)
{
	if (m_NativeData)
	{
		drgEngine::NativeWindowSetPosition(this, x, y);
	}
	else
	{
		GUI_Widget::SetPos(x, y);
	}
}

void GUI_Window::GetNativePosView(drgVec2 *pos)
{
	if (m_NativeData)
	{
		drgVec2 nativePos = drgEngine::NativeWindowGetPosition(this);
		pos->x = nativePos.x + m_View.minv.x;
		pos->y = nativePos.y + m_View.minv.y;
	}
	else
	{
		if (m_Parent != NULL)
			m_Parent->GetNativePosView(pos);

		pos->x += m_View.minv.x;
		pos->y += m_View.minv.y;
	}
}

void GUI_Window::GetNativePosBox(drgVec2 *pos)
{
	if (m_NativeData)
	{
		drgVec2 nativePos = drgEngine::NativeWindowGetPosition(this);
		pos->x = nativePos.x + m_Sens.minv.x;
		pos->y = nativePos.y + m_Sens.minv.y;
	}
	else
	{
		if (m_Parent != NULL)
		{
			m_Parent->GetNativePosBox(pos);
		}
		pos->x += m_Sens.minv.x;
		pos->y += m_Sens.minv.y;
	}
}

GUI_Style *GUI_Window::GetStyle()
{
	if (m_DrawContext)
	{
		return m_DrawContext->GetStyle();
	}
	return NULL;
}

void GUI_Window::SetStyle(GUI_Style *style)
{
	if (m_DrawContext)
	{
		m_DrawContext->SetStyle(style);
	}
}

void GUI_Window::OnCloseButtonPressed()
{
	if (m_CloseCallback)
	{
		m_CloseCallback(this, m_CloseCallbackData);
	}
}

void GUI_Window::OnParentResize()
{
	if (m_NativeData != NULL)
	{
		return;
	}
	if (m_FlagsWin & DRG_WINDOW_MAXIMIZED)
	{
		Resize(0.0f, m_Parent->HeightView(), 0.0f, m_Parent->WidthView());
	}
}

void GUI_Window::WidgetOffsetRectInt(drgBBox *bbox)
{
	if (m_Parent != NULL && m_NativeData == NULL)
	{
		m_Parent->WidgetOffsetRectInt(bbox);
	}
	bbox->minv.x += m_View.minv.x;
	bbox->minv.y += m_View.minv.y;
	bbox->maxv.x += m_View.minv.x;
	bbox->maxv.y += m_View.minv.y;
}

void GUI_Window::SetCurrentActive(GUI_Widget *active)
{
	if (m_NativeData == NULL && m_Parent != NULL)
	{
		m_Parent->SetCurrentActive(active);
	}
	else if (m_NativeData != NULL)
	{
		if (m_CurrentActive)
		{
			m_CurrentActive->RemoveFlagWidget(DRG_WIDGET_CURRENT_ACTIVE);
		}

		m_CurrentActive = active;

		if (m_CurrentActive)
		{
			m_CurrentActive->AddFlagWidget(DRG_WIDGET_CURRENT_ACTIVE);
		}
	}
}

GUI_Window *GUI_Window::CreateChildWindow(string16 title, unsigned int width, unsigned int height, unsigned int flags)
{
	GUI_Window *win = new GUI_Window();
	win->UseAsScriptObject();
	win->Init();
	win->m_Sens.minv.x = 0;
	win->m_Sens.minv.y = 0;
	win->m_Sens.maxv.x = (float)width;
	win->m_Sens.maxv.y = (float)height;
	win->m_View = win->m_Sens;
	win->m_BorderBot = WINDOW_BOT_BORDER_DEFAULT;
	win->m_BorderSide = WINDOW_SIDE_BORDER_DEFAULT;
	win->m_ResizeCtrl = 10.0f;
	win->m_FlagsWin |= flags;
	AddChild(win);
	if (!(win->m_FlagsWin & DRG_WINDOW_NOTITLE))
	{
		unsigned int tbarflags = 0;
		if (win->m_FlagsWin & DRG_WINDOW_NOCLOSE)
			tbarflags |= DRG_TITLEBAR_NO_CLOSE;
		if (win->m_FlagsWin & DRG_WINDOW_NOMAXIMIZE)
			tbarflags |= DRG_TITLEBAR_NO_MAXIMIZE;
		win->m_TitleBar = GUI_TitleBar::CreateTitleBar(win, GetDrawContext()->GetStyle()->GetDefaultFont(), tbarflags, &title);
		win->m_TitleBar->Move(0.0f, -win->m_TitleBar->HeightBox());
	}
	win->UpdateSize();
	return win;
}

GUI_Menu *GUI_Window::CreateMenu()
{
	drgFont *font = GetDrawContext()->GetStyle()->GetDefaultFont();
	if (m_Menu != NULL)
		delete m_Menu;
	m_Menu = GUI_Menu::CreateMenu(font, false, 0, 0.0f);
	m_Menu->SetParent(this);
	m_Menu->m_Owner = this;

	m_Menu->m_Sens.minv.x = 0.0f;
	m_Menu->m_Sens.maxv.x = WidthBox();
	m_Menu->m_Sens.minv.y = -(m_Menu->m_Sens.minv.y + (((float)font->GetFontSize()) * 1.5f));
	m_Menu->m_Sens.maxv.y = 0.0f;
	m_Menu->UpdateSize();
	m_Menu->ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);
	UpdateSize();

	return m_Menu;
}

int GUI_Window::GetWindowLayer()
{
	return 0;
}

int GUI_Window::GetLayer()
{
	if (GetFlagsWin() & DRG_WINDOW_MODAL && m_Manager != NULL)
	{
		return m_Manager->GetTopLayer() + 1;
	}
	else
	{
		return GUI_Widget::GetLayer();
	}
}

void GUI_Window::ResizeRel(float top, float bot, float left, float right)
{
	float newWidth = (m_Sens.maxv.x + right) - (m_Sens.minv.x + left);
	float newHeight = (m_Sens.maxv.y + bot) - (m_Sens.minv.y + top);
	if (newWidth > GetMinWidth() && newWidth < GetMaxWidth())
	{
		m_Sens.minv.x += left;
		m_Sens.maxv.x += right;
		if (m_TitleBar)
		{
			m_TitleBar->ResizeRel(0.0f, 0.0f, 0.0f, (right - left));
		}
		if (m_Menu)
		{
			m_Menu->ResizeRel(0.0f, 0.0f, 0.0f, (right - left));
		}
	}
	float hBar = (m_TitleBar != NULL) ? m_TitleBar->HeightBox() : 0.0f;
	if (newHeight > GetMinHeight() && newHeight < GetMaxHeight())
	{
		m_Sens.minv.y += top;
		m_Sens.maxv.y += bot;
	}
	UpdateSize();
	GUI_Container::OnParentResize();
}

void GUI_Window::Resize(float top, float bot, float left, float right)
{
	GUI_Widget::Resize(top, bot, left, right);
	GUI_Container::OnParentResize();
}

bool GUI_Window::WindowSelectedWM(DRG_WM_OPERATION op)
{
	return (m_Manager->GetSelectedWidget() == this && m_WinOp == op);
}

GUI_Window *GUI_Window::GetParentWindow()
{
	if (m_Parent == NULL || m_NativeData)
	{
		if (HasFlagWin(DRG_WINDOW_BASE_WINDOW) == false && m_Parent)
		{
			return m_Parent->GetParentWindow();
		}
		else
		{
			return this;
		}
	}
	else
	{
		return m_Parent->GetParentWindow();
	}
}

GUI_Drawing *GUI_Window::GetDrawContext()
{
	if (m_DrawContext)
	{
		return m_DrawContext;
	}
	if (m_Parent)
	{
		return m_Parent->GetDrawContext();
	}
	return NULL;
}

GUI_Label::GUI_Label()
{
	m_Font = NULL;
	m_FlagsLabel = 0;
	m_Text = "";

	m_WidgetType = GUI_TYPE_LABEL;
}

GUI_Label::~GUI_Label()
{
	Destroy();
}

void GUI_Label::Init()
{
	m_PadTop = 2.0f;
	m_PadBot = 2.0f;
	m_PadLeft = 6.0f;
	m_PadRight = 6.0f;
}

void GUI_Label::Reinit()
{
}

void GUI_Label::Destroy()
{
}

void GUI_Label::Draw(GUI_Drawing *draw)
{
	if (!IsHidden())
	{
		short top, bot, left, right;
		draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
		SetClipRectWithOffsets(draw, top, bot, left, right);

		bool selected = IsParentSelected(); //|| IsMouseOverParent();

		draw->GetStyle()->Label(this, m_Font, selected);

		draw->GetRenderCache()->SetClipRect(top, bot, left, right);

		GUI_Widget::Draw(draw);
	}
}

void GUI_Label::UpdateSize()
{
	m_Sens.maxv.x = (float)m_Font->StringSize(GetText()->str());
	m_View = m_Sens;
}

void GUI_Label::SetText(string16 *text)
{
	m_Text = *text;
}

void GUI_Label::SetText(char *text)
{
	m_Text = text;
}

void GUI_Label::SetText(const char *text)
{
	m_Text = text;
}

string16 *GUI_Label::GetText()
{
	return &m_Text;
}

void GUI_Label::SetClipRectWithOffsets(GUI_Drawing *draw, short top, short bot, short left, short right)
{
	drgBBox rOffs(m_View);
	WidgetOffsetRect(&rOffs);

	short topNew, bottomNew, leftNew, rightNew;

	topNew = DRG_MAX(Float2Short((rOffs.minv.y)), top);
	leftNew = DRG_MAX(Float2Short((rOffs.minv.x)), left);

	drgVec3 paneMax = GetPaneClipMax();
	drgVec3 scrollOffset = GetScrollOffset();
	if (paneMax.IsZero() || scrollOffset.IsZero())
	{
		bottomNew = Float2Short(DRG_MIN((rOffs.maxv.y), bot) - scrollOffset.y);
		rightNew = right - Float2Short(GetScrollOffset().x);
	}
	else
	{
		bottomNew = Float2Short(paneMax.y);
		rightNew = Float2Short(paneMax.x);
	}

	DrawClipRectDebug(draw, drgBBox(top, bot, left, right), drgBBox(topNew, bottomNew, leftNew, rightNew));

	draw->GetRenderCache()->SetClipRect(topNew, bottomNew, leftNew, rightNew);
}

bool GUI_Label::IsMouseOverParent()
{
	bool mouseOver = false;
	GUI_MenuItem *parentMenuItem = (GUI_MenuItem *)(m_Parent);
	if (parentMenuItem != NULL)
		mouseOver = parentMenuItem->IsMouseOver();
	return mouseOver;
}

GUI_Label *GUI_Label::CreateLabel(GUI_Widget *parent, drgFont *font, string16 text, unsigned int flags)
{
	GUI_Label *label = new GUI_Label();
	label->UseAsScriptObject();
	label->Init();
	label->m_FlagsLabel |= flags;
	label->m_Text = text;
	label->m_Font = font;
	label->SetParent(parent);
	label->m_Sens.minv.x = 0;
	label->m_Sens.minv.y = ((float)font->GetFontSize()) * 0.15f;
	label->m_Sens.maxv.x = (float)font->StringSize(label->GetText()->str());
	label->m_Sens.maxv.y = ((float)font->GetFontSize()) * 1.6f;
	label->UpdateSize();
	return label;
}

GUI_Icon::GUI_Icon()
{
	m_FlagsIcon = 0;
	m_Texture = NULL;

	m_WidgetType = GUI_TYPE_ICON;
}

GUI_Icon::~GUI_Icon()
{
	Destroy();
}

void GUI_Icon::Init()
{
	m_FlagsWidget |= DRG_WIDGET_FOCUSABLE;
	m_FlagsIcon = 0;
}

void GUI_Icon::Reinit()
{
}

void GUI_Icon::Destroy()
{
}

void GUI_Icon::Draw(GUI_Drawing *draw)
{
	if (IsHidden())
		return;

	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	if (m_FlagsIcon & DRG_ICON_BGFILL)
		draw->DrawRect(this, m_View, &m_Color);
	if (m_Texture != NULL)
		draw->DrawRect(this, m_View, &m_Color, m_Texture);

	draw->GetRenderCache()->SetClipRect(top, bot, left, right);

	GUI_Widget::Draw(draw);
}

void GUI_Icon::SetTexture(char *filename)
{
	if (m_Texture != NULL)
		m_Texture->DecRef();
	m_Texture = drgTexture::LoadFromFile(filename);
}

void GUI_Icon::SetTexture(drgTexture *texture)
{
	m_Texture = texture;
}

GUI_Button::GUI_Button()
{
	m_Textures[0] = NULL;
	m_Textures[1] = NULL;
	m_Textures[2] = NULL;
	m_Icons[0] = ICON_NONE;
	m_Icons[1] = ICON_NONE;
	m_Icons[2] = ICON_NONE;
	m_Label = NULL;

	m_WidgetType = GUI_TYPE_BUTTON;
}

GUI_Button::~GUI_Button()
{
	Destroy();
}

void GUI_Button::Init()
{
	m_FlagsButton = 0;
	m_FlagsWidget = DRG_WIDGET_FOCUSABLE | DRG_WIDGET_UNFOCUSED_MOTION | DRG_WIDGET_UNFOCUSED_BUTTONUP | DRG_WIDGET_TABLE_EMBEDDABLE;
	m_State = DRG_BUTTON_STATE_UP;
	m_PadLeft = 4.0f;
	m_PadRight = 4.0f;
	m_PadTop = 3.0f;
	m_PadBot = 3.0f;
	m_FlagsButton = 0;
	m_CallBack = NULL;
	m_CallBackType = GUI_CB_GENRAL;
	m_CallBackData = NULL;
	m_Label = NULL;
}

void GUI_Button::Reinit()
{
}

void GUI_Button::Destroy()
{
}

void GUI_Button::Draw(GUI_Drawing *draw)
{
	if (IsHidden())
	{
		return;
	}
	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	if (m_FlagsButton & DRG_BUTTON_NO_BG)
	{
		if (m_Textures[0] != NULL)
		{
			if (m_FlagsButton & DRG_BUTTON_MOUSEOVER)
			{
				draw->DrawRect(this, m_View, &m_Color, (m_State == DRG_BUTTON_STATE_DOWN) ? m_Textures[2] : m_Textures[1]);
			}
			else
			{
				draw->DrawRect(this, m_View, &m_Color, (m_State == DRG_BUTTON_STATE_DOWN) ? m_Textures[2] : m_Textures[0]);
			}
		}
		else if (m_Icons[0] != ICON_NONE)
		{
			if (m_FlagsButton & DRG_BUTTON_MOUSEOVER)
			{
				draw->GetStyle()->IconFill(this, (m_State == DRG_BUTTON_STATE_DOWN) ? m_Icons[2] : m_Icons[1]);
			}
			else
			{
				draw->GetStyle()->IconFill(this, (m_State == DRG_BUTTON_STATE_DOWN) ? m_Icons[2] : m_Icons[0]);
			}
		}
	}
	else
	{
		draw->GetStyle()->ButtonBackground(this, (m_State == DRG_BUTTON_STATE_DOWN));
		if (m_Label != NULL)
		{
			float centerOffset = (WidthView() - m_Label->WidthView()) * 0.5f;
			m_Label->Move(centerOffset, 0.0f);
			m_Label->Draw(draw);
			m_Label->Move(-centerOffset, 0.0f);
		}
	}
	draw->GetRenderCache()->SetClipRect(top, bot, left, right);
	GUI_Widget::Draw(draw);
}

void GUI_Button::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (IsDisabled() || IsHidden())
	{
		return;
	}
	if (down)
	{
		if (key != DRG_KEY_ENTER && key != DRG_KEY_SPACE)
		{
			return;
		}
		m_State = DRG_BUTTON_STATE_DOWN;
		RunCallBack(GUI_CB_BUTTON_DOWN);
		m_FlagsButton |= DRG_BUTTON_KEYDOWN;

		if (m_FlagsButton & DRG_BUTTON_REPEAT)
		{
			// DRG_DelTimeout(bu, &bu->repeat_to);
			// DRG_ScheduleTimeout(bu, &bu->delay_to, 800);
		}
	}
	else
	{
		if (m_FlagsButton & DRG_BUTTON_REPEAT)
		{
			// DRG_DelTimeout(bu, &bu->delay_to);
			// DRG_DelTimeout(bu, &bu->repeat_to);
		}
		if (key != DRG_KEY_ENTER && key != DRG_KEY_SPACE)
		{
			return;
		}
		m_State = DRG_BUTTON_STATE_UP;

		if (m_FlagsButton & DRG_BUTTON_KEYDOWN)
		{
			m_FlagsButton &= ~(DRG_BUTTON_KEYDOWN);
			RunCallBack(GUI_CB_BUTTON_CLICK);
		}
	}
}

bool GUI_Button::CallMouseDown(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
	{
		return false;
	}
	// DRG_WidgetFocus(bu);
	if (button != DRG_MOUSE_BUTTON_LEFT)
	{
		return false;
	}
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs))
	{
		return false;
	}
	if (!(m_FlagsButton & DRG_BUTTON_STICKY))
	{
		m_State = DRG_BUTTON_STATE_DOWN;
	}
	else
	{
		m_State = (DRG_BUTTON_STATE) !(int)m_State;
	}

	RunCallBack((m_State == DRG_BUTTON_STATE_DOWN) ? GUI_CB_BUTTON_DOWN : GUI_CB_BUTTON_CLICK);
	if (m_FlagsButton & DRG_BUTTON_REPEAT)
	{
		// DRG_DelTimeout(bu, &bu->repeat_to);
		// DRG_ScheduleTimeout(bu, &bu->delay_to, agMouseSpinDelay);
	}
	return true;
}

bool GUI_Button::CallMouseClick(unsigned int winid, int button)
{
	if (m_FlagsButton & DRG_BUTTON_REPEAT)
	{
		// DRG_DelTimeout(bu, &bu->repeat_to);
		// DRG_DelTimeout(bu, &bu->delay_to);
	}

	if (button != DRG_MOUSE_BUTTON_LEFT)
	{
		return false;
	}
	if (IsDisabled())
	{
		return false;
	}
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs))
	{
		return false;
	}
	if ((m_State == DRG_BUTTON_STATE_DOWN) && !(m_FlagsButton & DRG_BUTTON_STICKY))
	{
		m_State = DRG_BUTTON_STATE_UP;
		RunCallBack(GUI_CB_BUTTON_CLICK);
	}
	return true;
}

void GUI_Button::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	SetHoverWidget();
	if (IsDisabled() || IsHidden())
	{
		return;
	}
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs))
	{
		if ((m_FlagsButton & DRG_BUTTON_STICKY) == 0 && m_State == DRG_BUTTON_STATE_DOWN)
		{
			m_State = DRG_BUTTON_STATE_UP;
		}
		if (m_FlagsButton & DRG_BUTTON_MOUSEOVER)
		{
			m_FlagsButton &= ~(DRG_BUTTON_MOUSEOVER);
			RunCallBack(GUI_CB_BUTTON_OUT);
		}
	}
	else if (!(m_FlagsButton & DRG_BUTTON_MOUSEOVER))
	{
		m_FlagsButton |= DRG_BUTTON_MOUSEOVER;
		RunCallBack(GUI_CB_BUTTON_IN);
	}
}

void GUI_Button::SetTitle(string16 *title)
{
	if (m_Label == NULL)
	{
		m_Label = GUI_Label::CreateLabel(this, GetDrawContext()->GetStyle()->GetDefaultFont(), *title, 0);
	}
	m_Label->SetText(title);
}

void GUI_Button::SetIcon(DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down)
{
	m_Icons[0] = icon;
	m_Icons[1] = icon_up;
	m_Icons[2] = icon_down;
}

void GUI_Button::SetTexture(char *filename)
{
	m_Textures[0] = drgTexture::LoadFromFile(filename);
}

void GUI_Button::SetClipRectWithOffsets(GUI_Drawing *draw, short top, short bot, short left, short right)
{
	float leftF = (float)left;
	float rightF = (float)right;
	AdjustWidth(&leftF, &rightF);
	left = (short)DRG_ROUNDINT(leftF, 1);
	right = (short)DRG_ROUNDINT(rightF, 1);

	drgBBox rOffs(m_View);
	WidgetOffsetRect(&rOffs);

	short topNew, bottomNew, leftNew, rightNew;
	topNew = DRG_MAX(Float2Short((rOffs.minv.y)), top);
	leftNew = DRG_MIN(Float2Short((rOffs.minv.x)), left);

	drgVec3 paneMax = GetPaneClipMax();
	drgVec3 scrollOffset = GetScrollOffset();
	if (paneMax.IsZero() || scrollOffset.IsZero())
	{
		bottomNew = Float2Short(DRG_MIN((rOffs.maxv.y), (float)bot) - scrollOffset.y);
		rightNew = Float2Short(DRG_MAX((rOffs.maxv.x), (float)right) - scrollOffset.x);
	}
	else
	{
		bottomNew = Float2Short(paneMax.y);
		rightNew = Float2Short(paneMax.x);
	}

	DrawClipRectDebug(draw, drgBBox(top, bot, left, right), drgBBox(topNew, bottomNew, leftNew, rightNew));

	draw->GetRenderCache()->SetClipRect(topNew, bottomNew, leftNew, rightNew);
}

void GUI_Button::AdjustWidth(float *left, float *right)
{
	if (m_Label != NULL && m_Label->GetFont() != NULL && m_Label->GetText() != NULL)
	{
		float xCenter = (PosXView() + (PosXView() + WidthView())) / 2.0f;
		float textWidth = (float)m_Label->GetFont()->StringSize(m_Label->GetText()->str());
		*left = DRG_MIN((*left), xCenter - (textWidth / 2.0f));
		*right = DRG_MAX((*right), xCenter + (textWidth / 2.0f));
	}
}

GUI_Button *GUI_Button::CreateChildButton(GUI_Widget *parent, DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down, drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = new GUI_Button();
	button->UseAsScriptObject();
	button->Init();
	button->m_FlagsButton |= (DRG_BUTTON_NO_BG | flags);
	button->m_Type = BTN_TYPE_NORMAL;
	button->SetCallBack(callback, cbdata);
	button->m_Icons[0] = icon;
	button->m_Icons[1] = icon_up;
	button->m_Icons[2] = icon_down;
	button->m_Sens.minv.x = 0;
	button->m_Sens.minv.y = 0;
	button->m_Sens.maxv.x = 16;
	button->m_Sens.maxv.y = 16;
	button->UpdateSize();

	button->SetParent(parent);
	return button;
}

GUI_Button *GUI_Button::CreateChildRadioButton(GUI_Widget *parent, DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down, drgCallbackObj callback, void *cbdata, unsigned int flags)
{
	GUI_Button *button = new GUI_Button();
	button->UseAsScriptObject();
	button->Init();
	button->m_FlagsButton |= (DRG_BUTTON_NO_BG | flags);
	button->m_FlagsButton |= DRG_BUTTON_STICKY;
	button->m_Type = BTN_TYPE_RADIO;
	button->SetCallBack(callback, cbdata);
	button->m_Icons[0] = icon;
	button->m_Icons[1] = icon_up;
	button->m_Icons[2] = icon_down;
	button->m_Sens.minv.x = 0;
	button->m_Sens.minv.y = 0;
	button->m_Sens.maxv.x = 16;
	button->m_Sens.maxv.y = 16;
	button->UpdateSize();

	button->SetParent(parent);
	return button;
}




const int GUI_Input::INVALID_HIGHLIGHT_POS = -1;

GUI_Input::GUI_Input()
{
	m_FlagsInput = 0;
	m_Text = "";
	m_CurPos = 0;
	m_StartPos = 0;
	m_HighlightPos = INVALID_HIGHLIGHT_POS;
	m_TextOffset = 0;
	m_MaxLength = 256;
	m_UpdatePos = true;
	m_bShiftDown = false;

	m_pos = NULL;
	m_rel = NULL;
}

GUI_Input::~GUI_Input()
{
	Destroy();
}

void GUI_Input::Init()
{
	m_PadTop = 2.0f;
	m_PadBot = 2.0f;
	m_PadLeft = 6.0f;
	m_PadRight = 6.0f;
	m_Text = "";
	m_CurPos = 0;
	m_StartPos = 0;
	m_HighlightPos = INVALID_HIGHLIGHT_POS;
	m_TextOffset = 0;
	m_MaxLength = 256;
	m_UpdatePos = true;
	m_CallBack = NULL;
	m_bShiftDown = false;
	m_bReplaceMode = false;
	m_CallBackType = GUI_CB_GENRAL;
	m_CallBackData = NULL;
}

void GUI_Input::Reinit()
{
}

void GUI_Input::Destroy()
{
}

void GUI_Input::Draw(GUI_Drawing* draw)
{
	if(IsHidden())
		return;

	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	draw->GetStyle()->TextboxBackground(this);
	
	if(IsFocused() == false)
		m_CurPos = 0;

	if(m_UpdatePos)
	{
		drgFont* font = draw->GetStyle()->GetInputFont();
		unsigned short* text = (unsigned short*)m_Text.str();
		m_EndPos = m_CurPos;

		int count = (m_EndPos>m_StartPos)?(m_EndPos-(m_StartPos+1)):0;
		int length = font->StringSize(&text[m_StartPos], 1.0f, count);
		while(length<WidthView())
		{
			if((unsigned int)m_EndPos<m_Text.length())
				m_EndPos++;
			else
				break;
			length = font->StringSize(&text[m_StartPos], 1.0f, (m_EndPos-m_StartPos));
		}
		length = font->StringSize(&text[m_StartPos], 1.0f, (m_EndPos-m_StartPos));

		m_Offset = 0;
		if((m_StartPos!=m_CurPos) && ((length+4.0f)>WidthView()))
			m_Offset = ((int)WidthView()-length)-4;
		m_UpdatePos = false;
	}
	draw->GetStyle()->TextboxText(this, (unsigned short*)m_Text.str(), m_StartPos, m_EndPos, m_Offset, m_CurPos, IsFocused(), m_HighlightPos, m_CurPos);

	draw->GetRenderCache()->SetClipRect( top, bot, left, right );

	GUI_Widget::Draw(draw);
}

void GUI_Input::SetText(string16* title)
{
	m_Text = *title;
	m_UpdatePos = true;
	RunCallBack(GUI_CB_INPUT_CHANGE);
}

void GUI_Input::SetText(const char* title)
{
	m_Text = title;
	m_UpdatePos = true;
	RunCallBack(GUI_CB_INPUT_CHANGE);
}

void GUI_Input::SetText(char* title)
{
	const char* constTitle = title;
	SetText(constTitle);
}

string16* GUI_Input::GetText()
{
	return &m_Text;
}

void GUI_Input::CheckForKeyUp(int key)
{
	switch(key)
	{
		case DRG_KEY_LSHIFT:
		case DRG_KEY_RSHIFT:
			m_bShiftDown = false;
			break;
		default:
			break;
	}
}

//Main keyboard input function
void GUI_Input::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (IsDisabled() || IsHidden())
		return;

	if(!IsFocused())
	{
		m_HighlightPos = INVALID_HIGHLIGHT_POS;
		return;
	}

	if(!down)
	{
		CheckForKeyUp(key);
		return;
	}

	m_UpdatePos = true;

	//drgPrintOut("m_HighlightPos: %i\r\n", m_HighlightPos);
	//drgPrintOut("%i  %i\n", charval, key);
	if(charval<0)
	{
		bool deleted = false;
		switch(key)
		{
			case DRG_KEY_LEFT:
				if(m_bShiftDown && m_HighlightPos == INVALID_HIGHLIGHT_POS)
					m_HighlightPos = m_CurPos;
				else if(!m_bShiftDown)
					m_HighlightPos = INVALID_HIGHLIGHT_POS;

				if(m_CurPos>0)
					m_CurPos = DRG_CLAMP(m_CurPos - 1, 0, m_EndPos);
				break;
			case DRG_KEY_RIGHT:
				if(m_bShiftDown && m_HighlightPos == INVALID_HIGHLIGHT_POS)
					m_HighlightPos = m_CurPos;
				else if(!m_bShiftDown)
					m_HighlightPos = INVALID_HIGHLIGHT_POS;

				if((unsigned int)m_CurPos<m_Text.length())
					m_CurPos = DRG_CLAMP(m_CurPos + 1, 0, m_EndPos);
				break;
			case DRG_KEY_LSHIFT:
			case DRG_KEY_RSHIFT:
				m_bShiftDown = true;
				break;
			case DRG_KEY_INSERT:
				m_bReplaceMode = !m_bReplaceMode;
				SetReplaceModeHighlight();
				break;
			case DRG_KEY_DEL:
				if(m_HighlightPos != INVALID_HIGHLIGHT_POS)
				{
					ReplaceHighlightedText();
					deleted = true;
				}
				else if((unsigned int)m_CurPos<m_Text.length())
				{
					m_Text.remove(m_CurPos, 1);
					deleted = true;
				}
				if(deleted)
				{
					SetReplaceModeHighlight();
					RunCallBack(GUI_CB_INPUT_CHANGE);
				}
				break;
			case DRG_KEY_HOME:
				if(m_bShiftDown && m_HighlightPos == INVALID_HIGHLIGHT_POS)
					m_HighlightPos = m_CurPos;
				else if(!m_bShiftDown)
					m_HighlightPos = INVALID_HIGHLIGHT_POS;
				m_CurPos = 0;
				break;
			case DRG_KEY_END:
				if(m_bShiftDown && m_HighlightPos == INVALID_HIGHLIGHT_POS)
					m_HighlightPos = m_CurPos;
				else if(!m_bShiftDown)
					m_HighlightPos = INVALID_HIGHLIGHT_POS;
				m_CurPos = m_Text.length();
				break;
			default:
				break;
		};
	}
	else
	{
		if(key == DRG_KEY_ESC) 
		{
			RunCallBack(GUI_CB_INPUT_FOCUS_LOST);
			m_Manager->SetFocusedWidget( NULL );
			SetUnFocused();
		}
		else if(key == DRG_KEY_TAB) 
		{
			RunCallBack(GUI_CB_INPUT_FOCUS_LOST);
			m_Manager->SetFocusedWidget( NULL );
			SetUnFocused();
		}
		else if(key == DRG_KEY_ENTER || key == DRG_KEY_KP_ENTER) 
		{
			RunCallBack(GUI_CB_INPUT_FOCUS_LOST);
			m_Manager->SetFocusedWidget( NULL );
			SetUnFocused();
		}
		else if(key == DRG_KEY_BACKSPACE) 
		{
			bool deleted = false;
			if(m_HighlightPos != INVALID_HIGHLIGHT_POS && !m_bReplaceMode)
			{
				ReplaceHighlightedText();
				deleted = true;
			}
			else if(m_CurPos>0)
			{
				m_CurPos = DRG_CLAMP(m_CurPos - 1, 0, m_EndPos);
				m_Text.remove(m_CurPos, 1);
				deleted = true;
			}
			if(deleted)
			{				
				SetReplaceModeHighlight();
				RunCallBack(GUI_CB_INPUT_CHANGE);
			}
		}
		else if(charval == 0X18) // CUT
		{
			int len = 0;
			string16 copy_val = "";
			
			NormalizeHighlight();
			copy_val = m_Text.substr(m_CurPos, m_HighlightPos - m_CurPos);
			drgEngine::SetClipboardText(&copy_val);

			ReplaceHighlightedText();

			RunCallBack(GUI_CB_INPUT_CHANGE);
		}
		else if(charval == 0X03) // COPY
		{
			string16 copy_val = "";
			NormalizeHighlight();
			copy_val = m_Text.substr(m_CurPos, m_HighlightPos - m_CurPos);
			drgEngine::SetClipboardText(&copy_val);
		}
		else if(charval == 0X16) // PASTE
		{
			string16 paste_val = "";
			int len = drgEngine::GetClipboardText(&paste_val);
			if(len>0)
			{
				if(m_HighlightPos != INVALID_HIGHLIGHT_POS)
				{
					ReplaceHighlightedText(paste_val);
				}
				else
				{
					m_Text.insert(m_CurPos, paste_val);
					m_CurPos += len;
				}
				SetReplaceModeHighlight();
				RunCallBack(GUI_CB_INPUT_CHANGE);
			}
		}
		else //A-Z, 0-9, etc.
		{
			if(m_Text.length()<(unsigned int)m_MaxLength)
			{
				if(m_HighlightPos != INVALID_HIGHLIGHT_POS)
				{
					ReplaceHighlightedText(charval);
				}
				else
				{
					m_Text.insert(m_CurPos, charval);
					m_EndPos++;
					m_CurPos = DRG_CLAMP(m_CurPos + 1, 0, m_EndPos);
				}
				SetReplaceModeHighlight();
				RunCallBack(GUI_CB_INPUT_CHANGE);
			}
		}
	}
}

bool GUI_Input::CallMouseDown(unsigned int winid, int button)
{
	return false;
}

bool GUI_Input::CallMouseClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	bool clicked = false;
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if(drgInputMouse::IsCursorInside(&rOffs))
	{
		m_CurPos = m_Text.length();
		if(m_bShiftDown || m_HighlightPos == INVALID_HIGHLIGHT_POS)
		{
			m_HighlightPos = 0;
		}
		else 
		{
			m_HighlightPos = INVALID_HIGHLIGHT_POS;
		}

		SetFocused();
		clicked = true;
	}

	m_UpdatePos = true;
	return clicked;
}

void GUI_Input::SetUnFocused()
{
	m_Manager->SetFocusedWidget( NULL );
	m_HighlightPos = INVALID_HIGHLIGHT_POS;

	//
	// If we're being unfocused because there is a click outside of our
	// bounds, then run our focus lost callback
	//

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if(!drgInputMouse::IsCursorInside(&rOffs))
	{
		RunCallBack(GUI_CB_INPUT_FOCUS_LOST);
	}
}

void GUI_Input::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if (IsDisabled() || IsHidden())
		return;
	if(!IsFocused())
		return;
	m_UpdatePos = true;

	//drgPrintOut("MOUSE MOVE: POS (%d, %d), REL (%d, %d)\r\n", pos->x, pos->y, rel->x, rel->y);

	//m_pos = pos;
	//m_rel = rel;
}

void GUI_Input::NormalizeHighlight() 
{
	if(m_HighlightPos < m_CurPos)
	{
		int temp = m_CurPos;
		m_CurPos = m_HighlightPos;
		m_HighlightPos = temp;
	}
}

void GUI_Input::ReplaceHighlightedText(int charval)
{
	ReplaceHighlightedText(string16((short)charval));
}

void GUI_Input::ReplaceHighlightedText(string16 text)
{
	NormalizeHighlight();

	string16 finalStr = m_Text.substr(0, m_CurPos);
	unsigned int leftOfReplaceLength = finalStr.length();
	finalStr.concat(text);
	finalStr.concat(m_Text.substr(m_HighlightPos));
	m_Text = finalStr;
	m_CurPos = DRG_CLAMP(leftOfReplaceLength + text.length(), 0, (unsigned int)m_EndPos);
	m_HighlightPos = INVALID_HIGHLIGHT_POS;
}

void GUI_Input::SetReplaceModeHighlight()
{
	if(m_bReplaceMode && m_CurPos < m_EndPos)
		m_HighlightPos = DRG_CLAMP(m_CurPos + 1, 0, m_EndPos);
	else
		m_HighlightPos = INVALID_HIGHLIGHT_POS;
}




void GUI_TitleBar::BtnCallBckClose(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	if(btn->GetTypeCB() == GUI_CB_BUTTON_CLICK) {
		GUI_TitleBar* title = (GUI_TitleBar*)data;
		if (title->m_Parent != NULL) {
			title->m_Parent->RunCallBack(title->m_CallBack, title->m_CallBackData, GUI_CB_WINDOW_CLOSE);
		}
	}
}

void BtnCallBckMaximize(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	if(btn->GetTypeCB() == GUI_CB_BUTTON_CLICK) {
		GUI_Window* win = (GUI_Window*)data;
		win->Maximize();
		if (win->HasFlagWin(DRG_WINDOW_MAXIMIZED)) {
			btn->SetIcon(ICON_MINIMIZE, ICON_MINIMIZE_UP, ICON_MINIMIZE_DOWN);
		}
		else {
			btn->SetIcon(ICON_MAXIMIZE, ICON_MAXIMIZE_UP, ICON_MAXIMIZE_DOWN);
		}
		btn->RemoveFlagButton(DRG_BUTTON_MOUSEOVER);
	}
}

GUI_TitleBar::GUI_TitleBar()
{
	m_FlagsTitleBar = 0;
	m_Pressed = false;
	m_Label = NULL;
	m_BtnClose = NULL;
	m_BtnMax = NULL;
	m_WidgetType = GUI_TYPE_TITLE_BAR;
	m_DragMovement.Set( 0.0f, 0.0f );
	m_DragThreshold = DEFAULT_DRAG_THRESHOLD;

}

GUI_TitleBar::~GUI_TitleBar()
{
	Destroy();
}

void GUI_TitleBar::Init()
{
	m_FlagsWidget |= DRG_WIDGET_UNFOCUSED_BUTTONUP | DRG_WIDGET_RESIZE_HFILL;
	
	m_FlagsTitleBar = 0;
	m_Pressed = false;
	SetParent( NULL );
	//tbar->maximize_btn = NULL;
	//tbar->minimize_btn = NULL;
	//tbar->close_btn = NULL;
	
	//tbar->label = AG_LabelNewS(tbar, AG_LABEL_HFILL|AG_LABEL_NOMINSIZE, ("Untitled"));
	//AG_LabelSizeHint(tbar->label, 1, "X");
	//AG_LabelSetPadding(tbar->label, 5,5,2,2);
}

void GUI_TitleBar::Reinit()
{
}

void GUI_TitleBar::Destroy()
{
}

void GUI_TitleBar::Draw(GUI_Drawing* draw)
{
	draw->GetStyle()->Titlebar(this, m_Label, IsPressed());
	if (m_BtnClose) {
		m_BtnClose->Draw(draw);
	}
	if (m_BtnMax) {
		m_BtnMax->Draw(draw);
	}
	GUI_Widget::Draw(draw);
}

#define DRAG_BOX_WIDTH_MODIFIER 0.8f
#define DRAG_BOX_X_MODIFIER 0.4f
#define DRAG_BOX_Y_MODIFIER 0.5f
#define DEFAULT_FONT_SIZE 22.0f

void GUI_TitleBar::DragDraw(GUI_Drawing* draw)
{
	drgBBox dragbox;
	float fontsize = DEFAULT_FONT_SIZE;
	float dragsize = ((m_Sens.maxv.x-m_Sens.minv.x) * DRAG_BOX_WIDTH_MODIFIER);
	drgColor color = draw->GetStyle()->GetDefaultColor(TITLEBAR_DRAG_COLOR);

	if (m_Parent != NULL) {
		fontsize = (float)draw->GetStyle()->GetDefaultFont()->GetFontSize();
	}

	float xoff = drgInputMouse::GetPos()->x-(dragsize * DRAG_BOX_X_MODIFIER);
	float yoff = drgInputMouse::GetPos()->y-(fontsize * DRAG_BOX_Y_MODIFIER);

	dragbox.minv.Set(xoff, yoff, 0.0f);
	dragbox.maxv.Set(dragsize+xoff, fontsize+yoff, 0.0f);

	draw->DrawRectDirect(dragbox, &color);

	color.a = color.r = color.g = color.b = 255;
	draw->DrawText( NULL, draw->GetStyle()->GetDefaultFont(), m_Label->GetText()->str(), dragbox, &color );
}

void GUI_TitleBar::DragDone()
{
	if( IsPressed() ) {
		m_Pressed = false;
	}
}

void GUI_TitleBar::Resize(float top, float bot, float left, float right)
{
	GUI_Widget::Resize(top, bot, left, right);
	float xpos = WidthView()-16.0f;
	if(m_BtnClose)
	{
		xpos -= (m_BtnClose->WidthBox()+8.0f);
		m_BtnClose->SetPos( xpos, (HeightView()-m_BtnClose->HeightBox())/2 );
	}
	if(m_BtnMax)
	{
		xpos -= (m_BtnMax->WidthBox()+8.0f);
		m_BtnMax->SetPos( xpos, (HeightView()-m_BtnMax->HeightBox())/2 );
	}
}

void GUI_TitleBar::ResizeRel(float top, float bot, float left, float right)
{
	GUI_Widget::ResizeRel(top, bot, left, right);
	float xpos = WidthView()-16.0f;
	if(m_BtnClose) {
		xpos -= (m_BtnClose->WidthBox()+8.0f);
		m_BtnClose->SetPos( xpos, (HeightView()-m_BtnClose->HeightBox())/2 );
	}
	if(m_BtnMax) {
		xpos -= (m_BtnMax->WidthBox()+8.0f);
		m_BtnMax->SetPos( xpos, (HeightView()-m_BtnMax->HeightBox())/2 );
	}
}

void GUI_TitleBar::SetTitle(string16* title)
{
	m_Label->SetText(title);
}

void GUI_TitleBar::CreateMaximizeButton()
{
	m_BtnMax = GUI_Button::CreateChildButton(this, ICON_MAXIMIZE, ICON_MAXIMIZE_UP, ICON_MAXIMIZE_DOWN, BtnCallBckMaximize, m_Parent, 0);
}

void GUI_TitleBar::CreateCloseButton()
{
	m_BtnClose = GUI_Button::CreateChildButton(this, ICON_CLOSE, ICON_CLOSE_UP, ICON_CLOSE_DOWN, BtnCallBckClose, this, 0);
}

bool GUI_TitleBar::CallMouseDown(unsigned int winid, int button)
{
	if (button != DRG_MOUSE_BUTTON_LEFT) {
		return false;
	}
	if (m_BtnClose) {
		m_BtnClose->CallMouseDown(winid, button);
	}
	if (m_BtnMax) {
		m_BtnMax->CallMouseDown(winid, button);
	}
	if (((m_BtnClose) && m_BtnClose->GetState()) || ((m_BtnMax) && m_BtnMax->GetState())) {
		return false;
	}
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);

	if( drgInputMouse::IsCursorInside(&rOffs) && m_Parent ) {
		m_Pressed = true;
		m_DragMovement.Set( 0.0f, 0.0f );

		m_Parent->SetFocused();
		m_Parent->SetSelected();

		if (m_Parent->GetWidgetType() == GUI_TYPE_WINDOW) {
			GUI_Window* parent_win = (GUI_Window*)m_Parent;
			if (!(parent_win->m_FlagsWin & DRG_WINDOW_NOMOVE)) {
				parent_win->m_WinOp = DRG_WINOP_MOVE;
			}
		}
		return true;
	}
	return false;
}

bool GUI_TitleBar::CallMouseClick(unsigned int winid, int button)
{
	if (button != DRG_MOUSE_BUTTON_LEFT) {
		return false;
	}

	bool clicked = false;
	if (m_BtnClose) {
		clicked = m_BtnClose->CallMouseClick(winid, button);
	}

	if (m_BtnMax) {
		clicked = m_BtnMax->CallMouseClick(winid, button);
	}
	
	if( m_Parent->IsSelected() ) {
		clicked = true;
		m_Pressed = false;
		if (m_Parent->GetWidgetType() == GUI_TYPE_WINDOW) {
			GUI_Window* parent_win = (GUI_Window*)m_Parent;
			parent_win->m_WinOp = DRG_WINOP_NONE;
		}
		m_Manager->SetSelectedWidget( NULL );
		m_Manager->SetFocusedWidget( NULL );
		m_Manager->SetDragWidget( NULL );
	}
	return clicked;
}

void GUI_TitleBar::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if (m_BtnClose) {
		m_BtnClose->CallMouseMove(winid, pos, rel);
	}
	if (m_BtnMax) {
		m_BtnMax->CallMouseMove(winid, pos, rel);
	}

	// Make us the drag widget if we are pressed currently
	if( IsPressed() && m_Manager && m_Manager->HasDragWidget() == false )
	{
		m_DragMovement.x += rel->x;
		m_DragMovement.y += rel->y;

		if (m_DragMovement.x >= m_DragThreshold || m_DragMovement.y >= m_DragThreshold) {
			if (m_Parent->HasFlagWidget(DRG_WIDGET_DOCKER)) {
				m_Parent->SetDragBox();
			}
		}
	}
}

GUI_TitleBar* GUI_TitleBar::CreateTitleBar(GUI_Widget* parent, drgFont* font, unsigned int flags, string16* title)
{
	GUI_TitleBar* tbar = new GUI_TitleBar();
	tbar->Init();
	tbar->SetParent( parent );
	tbar->m_FlagsTitleBar = flags;

	if ((tbar->m_FlagsTitleBar & DRG_TITLEBAR_NO_MAXIMIZE) == 0)
		tbar->CreateMaximizeButton();
	if ((tbar->m_FlagsTitleBar & DRG_TITLEBAR_NO_CLOSE) == 0)
		tbar->CreateCloseButton();

	tbar->m_Label = GUI_Label::CreateLabel(tbar, font, *title, 0);
	tbar->SetTitle(title);
	tbar->m_Label->Move(6.0f, 0.0f);

	tbar->m_Sens.minv.x = 0.0f;
	tbar->m_Sens.minv.y = 0.0f;
	tbar->m_Sens.maxv.x = parent->WidthBox();
	tbar->m_Sens.maxv.y = tbar->m_Label->HeightBox()*1.3f;
	tbar->ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);

	return tbar;
}




GUI_MenuItem::GUI_MenuItem()
{
	m_Seperator = false;
	m_MouseOver = false;
	m_SubMenu = NULL;
}

GUI_MenuItem::~GUI_MenuItem()
{
	Destroy();
}

void GUI_MenuItem::Init()
{
}

void GUI_MenuItem::Reinit()
{
}

void GUI_MenuItem::Destroy()
{
}

void GUI_MenuItem::Draw(GUI_Drawing* draw)
{
	if(m_Seperator)
	{
		draw->GetStyle()->MenuItemSeparator(this);
	}
	else
	{
		if(m_MouseOver)
			draw->GetStyle()->MenuItemBackground(this, false);
		m_Label->Draw(draw);
	}
}

void GUI_MenuItem::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if(m_Seperator)
		return;

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	m_MouseOver = drgInputMouse::IsCursorInside(&rOffs);

	bool setsubactive = (m_SubMenu && m_MouseOver);
	if(setsubactive && (m_Parent!=NULL))
	{
		GUI_Menu* menu = (GUI_Menu*)m_Parent;
		if(!menu->m_PopUp && !menu->IsFocused())
			setsubactive = false;
	}
	if(setsubactive)
		SetCurrentActive(m_SubMenu);
}

bool GUI_MenuItem::CallMouseDown(unsigned int winid, int button)
{
	return false;
}

bool GUI_MenuItem::CallMouseClick(unsigned int winid, int button)
{
	if(m_Seperator)
		return false;
	if(!m_MouseOver)
		return false;
	if(button != DRG_MOUSE_BUTTON_LEFT)
		return false;

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if(!drgInputMouse::IsCursorInside(&rOffs))
		return false;

	if(m_Parent!=NULL)
	{
		GUI_Menu* menu = (GUI_Menu*)m_Parent;
		if(!menu->m_PopUp)
		{
			menu->SetFocused();
			if(m_SubMenu)
				SetCurrentActive(m_SubMenu);
		}
	}
	RunCallBack(GUI_CB_MENU_CLICK);
	return true;
}

int  GUI_MenuItem::GetLayer()
{ 
	if(m_Manager != NULL)
		return m_Manager->GetTopLayer() + 1;
	else
		return GUI_Widget::GetLayer();
}


GUI_Menu::GUI_Menu()
{
	m_ItemMaxSpace = 100.0f;
	m_ItemSpaceX = 20.0f;
	m_ItemSpaceY = 4.0f;
	m_ItemSepSize = 2.0f;
	m_PopUp = false;
	m_Owner = NULL;
	m_ContainParent = NULL;
	m_ParentMenu = NULL;
	m_Font = NULL;

	m_WidgetType = GUI_TYPE_MENU;
}

GUI_Menu::~GUI_Menu()
{
	Destroy();
}

void GUI_Menu::Init()
{
	m_FlagsWidget |= DRG_WIDGET_UNFOCUSED_BUTTONUP | DRG_WIDGET_RESIZE_HFILL;
	SetParent( NULL );
	m_Owner = NULL;
	m_ContainParent = NULL;
}

void GUI_Menu::Reinit()
{
}

void GUI_Menu::Destroy()
{
}

void GUI_Menu::Draw(GUI_Drawing* draw)
{
	if(m_PopUp && (m_ParentMenu!=NULL) && m_ParentMenu->m_PopUp)
		m_ParentMenu->Draw(draw);

	draw->GetStyle()->MenuBackground(this);

	GUI_MenuItem* itr = (GUI_MenuItem*)m_MenuItems.GetLast();
	while(itr)
	{
		itr->Draw(draw);
		itr = (GUI_MenuItem*)m_MenuItems.GetPrev(itr);
	}
}

bool GUI_Menu::CallMouseDown(unsigned int winid, int button)
{
	bool clicked = false;
	if (button == DRG_MOUSE_BUTTON_LEFT)
	{
		GUI_MenuItem* itr = (GUI_MenuItem*)m_MenuItems.GetFirst();
		while(itr)
		{
			clicked = itr->CallMouseDown(winid, button);
			if(clicked)
				break;
			itr = (GUI_MenuItem*)m_MenuItems.GetNext(itr);
		}
	}

	return clicked;
}

bool GUI_Menu::CallMouseClick(unsigned int winid, int button)
{
	bool clicked = false;
	if (button == DRG_MOUSE_BUTTON_LEFT)
	{
		GUI_MenuItem* itr = (GUI_MenuItem*)m_MenuItems.GetFirst();
		while(itr)
		{
			clicked = itr->CallMouseClick(winid, button);
			if(clicked)
				break;
			itr = (GUI_MenuItem*)m_MenuItems.GetNext(itr);
		}
	}
	if(m_PopUp)
		SetCurrentActive(NULL);
	return clicked;
}

void GUI_Menu::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	GUI_MenuItem* itr = (GUI_MenuItem*)m_MenuItems.GetFirst();
	while(itr)
	{
		itr->CallMouseMove(winid, pos, rel);
		itr = (GUI_MenuItem*)m_MenuItems.GetNext(itr);
	}
}

float GUI_Menu::OwnerTitleHeight()
{
	if((m_Owner != NULL) && (m_Owner->GetTitleBar() != NULL))
		return m_Owner->GetTitleBar()->HeightBox();
	else if((m_ContainParent != NULL) && (m_ContainParent->GetTitleBar() != NULL))
		return m_ContainParent->GetTitleBar()->HeightBox();
	else
		return 0.0f;
}

float GUI_Menu::GetItemsWidth()
{
	float retwidth = 0.0f;
	GUI_MenuItem* itr = (GUI_MenuItem*)m_MenuItems.GetLast();
	while(itr)
	{
		retwidth += itr->WidthBox();
		itr = (GUI_MenuItem*)m_MenuItems.GetPrev(itr);
	}
	return retwidth;
}

float GUI_Menu::GetItemsHeight()
{
	float retheight = 0.0f;
	GUI_MenuItem* itr = (GUI_MenuItem*)m_MenuItems.GetLast();
	while(itr)
	{
		retheight += itr->HeightBox();
		itr = (GUI_MenuItem*)m_MenuItems.GetPrev(itr);
	}
	return retheight;
}

GUI_Menu* GUI_Menu::CreateSubMenu(string16 text)
{
	GUI_MenuItem* item = CreateMenuItem(text, NULL, NULL);
	if(m_PopUp)
		item->m_SubMenu = GUI_Menu::CreateMenu(m_Font, true, 0, m_ItemMaxSpace);
	else
		item->m_SubMenu = GUI_Menu::CreateMenu(m_Font, true, 1, 0.0f);
	item->m_SubMenu->m_ParentMenu = this;
	item->m_SubMenu->SetParent( item );
	return item->m_SubMenu;
}

GUI_MenuItem* GUI_Menu::CreateMenuItem(string16 text, drgCallbackObj callback, void* data)
{
	GUI_MenuItem* item = new GUI_MenuItem();
	item->UseAsScriptObject();
	item->Init();

	item->SetParent( this );
	item->SetCallBack(callback, data);
	item->m_Label = GUI_Label::CreateLabel(item, m_Font, text, 0);
	item->m_Label->Move(m_ItemSpaceX*0.5f, 0.0f);

	if(m_PopUp)
	{
		item->m_Sens.minv.x = 0.0f;
		item->m_Sens.minv.y = GetItemsHeight();
		item->m_Sens.maxv.x = m_ItemMaxSpace;
		item->m_Sens.maxv.y = item->m_Sens.minv.y + item->m_Label->HeightBox() + m_ItemSpaceY;
		m_MenuItems.AddHead(item);
		m_Sens.maxv.y = m_Sens.minv.y + GetItemsHeight();
		ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		item->m_Sens.minv.x = GetItemsWidth();
		item->m_Sens.minv.y = 0.0f;
		item->m_Sens.maxv.x = item->m_Sens.minv.x + item->m_Label->WidthBox() + m_ItemSpaceX;
		item->m_Sens.maxv.y = item->m_Sens.minv.y + item->m_Label->HeightBox();
		m_MenuItems.AddHead(item);
	}
	item->UpdateSize();
	item->ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);

	return item;
}

void GUI_Menu::CreateMenuSeperator()
{
	if(!m_PopUp)
		return;

	GUI_MenuItem* item = new GUI_MenuItem();
	item->UseAsScriptObject();
	item->Init();

	item->SetParent( this );
	item->m_Seperator = true;

	item->m_Sens.minv.x = 0.0f;
	item->m_Sens.minv.y = GetItemsHeight();
	item->m_Sens.maxv.x = m_ItemMaxSpace;
	item->m_Sens.maxv.y = item->m_Sens.minv.y + m_ItemSepSize;
	m_MenuItems.AddHead(item);
	m_Sens.maxv.y = m_Sens.minv.y + GetItemsHeight();
	ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);

	item->UpdateSize();
	item->ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);
}

GUI_Menu* GUI_Menu::CreateMenu(drgFont* font, bool popup, int lines, float offx)
{
	GUI_Menu* menu = new GUI_Menu();
	menu->UseAsScriptObject();
	menu->Init();
	menu->m_Font = font;
	menu->m_PopUp = popup;
	if(menu->m_PopUp)
	{
		menu->m_Sens.minv.x = offx;
		menu->m_Sens.minv.y = (((float)font->GetFontSize())*1.5f)*((float)lines);
		menu->m_Sens.maxv.x = menu->m_Sens.minv.x + menu->m_ItemMaxSpace;
		menu->m_Sens.maxv.y = menu->m_Sens.minv.y;
	}
	else
	{
		menu->m_Sens.minv.x = 0.0f;
		menu->m_Sens.minv.y = 0.0f;
		menu->m_Sens.maxv.x = 0.0f;
		menu->m_Sens.maxv.y = menu->m_Sens.minv.y + (((float)font->GetFontSize())*1.5f);
	}
	menu->UpdateSize();
	menu->ResizeRel(0.0f, 0.0f, 0.0f, 0.0f);
	return menu;
}

void GUI_Slider::BtnCallBckInc(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	GUI_Slider* slider = (GUI_Slider*)data;
	if((btn->GetTypeCB() == GUI_CB_BUTTON_DOWN) && (slider->m_State == GUI_SLIDER_STATE_NONE))
	{
		slider->IncrementSlider();
	}
	else if((btn->GetTypeCB() == GUI_CB_BUTTON_CLICK) && (slider->m_State == GUI_SLIDER_STATE_INC))
	{
		slider->m_State = GUI_SLIDER_STATE_NONE;
		slider->m_CountIncDec = -1.0f;
	}
}

void GUI_Slider::BtnCallBckDec(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	GUI_Slider* slider = (GUI_Slider*)data;
	if((btn->GetTypeCB() == GUI_CB_BUTTON_DOWN) && (slider->m_State == GUI_SLIDER_STATE_NONE))
	{
		slider->DecrementSlider();
	}
	else if((btn->GetTypeCB() == GUI_CB_BUTTON_CLICK) && (slider->m_State == GUI_SLIDER_STATE_DEC))
	{
		slider->m_State = GUI_SLIDER_STATE_NONE;
		slider->m_CountIncDec = -1.0f;
	}
}


GUI_Slider::GUI_Slider()
{
	m_FlagsSlider	= 0;
	m_Value			= 0.0f;
	m_Min			= 0.0f;
	m_Max			= 100.0f;
	m_Scale			= 1.0f;
	m_Size			= 100.0;
	m_CountIncDec	= -1.0f;
	m_CountIncAmt   = 1.0f;
	m_State			= GUI_SLIDER_STATE_NONE;
	m_BtnInc		= NULL;
	m_BtnDec		= NULL;

	m_WidgetType	= GUI_TYPE_SLIDER;

}

GUI_Slider::~GUI_Slider()
{
	Destroy();
}

void GUI_Slider::Init()
{
	m_Min			= 0.0f;
	m_Max			= 100.0f;
	m_Scale			= 1.0f;
	m_State			= GUI_SLIDER_STATE_NONE;
	m_FlagsSlider	= 0;

	m_FlagsWidget |= DRG_WIDGET_FOCUSABLE;
}

void GUI_Slider::Reinit()
{
}

void GUI_Slider::Destroy()
{
}

void GUI_Slider::Draw(GUI_Drawing* draw)
{
	if(IsHidden())
		return;

	if ((m_FlagsSlider & DRG_SCROLLBAR_AUTOHIDE) && ((m_SliderBox.maxv.x - m_SliderBox.minv.x) <= 0.01f))
		return;

	if(m_CountIncDec > 0.0f)
	{
		m_CountIncDec -= 0.003f;
		if(m_CountIncDec < 0.0f)
		{
			m_CountIncAmt *= 0.8f;
			if(m_CountIncAmt < SLIDER_INC_DEC_TIME_MIN)
				m_CountIncAmt = SLIDER_INC_DEC_TIME_MIN;
			if(m_State == GUI_SLIDER_STATE_INC)
			{
				SetValue(m_Value + m_Scale);
				m_CountIncDec = m_CountIncAmt;
			}
			else if(m_State == GUI_SLIDER_STATE_DEC)
			{
				SetValue(m_Value - m_Scale);
				m_CountIncDec = m_CountIncAmt;
			}
		}
	}

	draw->GetStyle()->Slider(this, m_State, m_SliderArea.minv.x, m_SliderArea.maxv.x, m_SliderBox.minv.x, m_SliderBox.maxv.x);

	m_BtnInc->Draw(draw);
	m_BtnDec->Draw(draw);

	GUI_Widget::Draw(draw);
}

void GUI_Slider::UpdateSize()
{
	GUI_Widget::UpdateSize();

	if( m_BtnInc )
		m_BtnInc->SetPos( 0.0f, 0.0f );
	
	if( m_BtnDec )
		m_BtnDec->SetPos( WidthView() - m_BtnDec->WidthView(), 0.0f );

	UpdateSliderPos();
}

void GUI_Slider::UpdateSliderPos()
{
	float len = (m_Max - m_Min);
	float slide_size = GetSliderBoxWidth();
	//float slide_size = (m_Sens.maxv.x - m_Sens.minv.x - (SLIDER_BAR_OFFSET * 2)) / (m_Max - m_Min);
	float slide_pos = m_Value / len;;

	float xStart = m_BtnInc == NULL ? SLIDER_BAR_OFFSET : m_BtnInc->PosXView() + m_BtnInc->WidthView();
	float xEnd = m_BtnDec == NULL ? SLIDER_BAR_OFFSET : m_BtnDec->PosXView();
	m_SliderArea.minv.x = xStart;
	m_SliderArea.minv.y = m_Sens.minv.y;
	m_SliderArea.maxv.x = xEnd;
	m_SliderArea.maxv.y = m_Sens.maxv.y;

	m_SliderBox.minv.x = (m_SliderArea.maxv.x - m_SliderArea.minv.x) * slide_pos;
	m_SliderBox.maxv.x = m_SliderBox.minv.x + slide_size;
}

float GUI_Slider::GetSliderBoxWidth()
{
	return (WidthView() - (SLIDER_BAR_OFFSET * 2)) / (m_Max - m_Min);
}

void GUI_Slider::CalcSliderValue(drgVec2* pos)
{
	drgBBox rOffs(m_SliderArea);
	WidgetOffsetRect(&rOffs);

	if((rOffs.maxv.x - rOffs.minv.x) > 0.0f)
		SetValue(m_Min + (((pos->x - rOffs.minv.x) / (rOffs.maxv.x - rOffs.minv.x)) * ((m_Max - m_Min))));
}

bool GUI_Slider::CallMouseDown(unsigned int winid, int button)
{	
	if(IsHidden())
		return false;

	if(button!=DRG_MOUSE_BUTTON_LEFT)
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool clicked = false;
	clicked = m_BtnInc->CallMouseDown(winid, button);
	if(m_BtnDec->CallMouseDown(winid, button))
		clicked = true;

	if(m_State == GUI_SLIDER_STATE_NONE)
	{
		if( m_Manager && m_Manager->HasSelectedWidget() == false )
		{
			drgBBox rOffs(m_SliderArea);
			WidgetOffsetRect(&rOffs);
			if(drgInputMouse::IsCursorInside(&rOffs))
			{
				m_State = GUI_SLIDER_STATE_SLIDE;
				SetFocused();
				SetSelected();
				drgVec2 mousePos = *drgInputMouse::GetPos();
				mousePos.x -= GetSliderBoxWidth() / 2.0f;
				CalcSliderValue(&mousePos);
				return true;
			}
		}
	}

	return clicked;
}

bool GUI_Slider::CallMouseClick(unsigned int winid, int button)
{
	if(IsHidden())
		return false;

	if(button != DRG_MOUSE_BUTTON_LEFT)
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool clicked = false;
	clicked = m_BtnInc->CallMouseClick(winid, button);
	if(m_BtnDec->CallMouseClick(winid, button))
		clicked = true;

	if(m_State == GUI_SLIDER_STATE_SLIDE)
	{
		m_State = GUI_SLIDER_STATE_NONE;
		m_Manager->SetSelectedWidget( NULL );
	}
	return clicked;
}

void GUI_Slider::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if(IsHidden())
		return;

	if( IsSelected() && IsCursorInBounds() == false )
	{
		m_Manager->SetSelectedWidget( NULL );
		return;
	}

	if(IsSelected() && (m_State == GUI_SLIDER_STATE_SLIDE))
		CalcSliderValue(pos);

	m_BtnInc->CallMouseMove(winid, pos, rel);
	m_BtnDec->CallMouseMove(winid, pos, rel);

	if((m_State == GUI_SLIDER_STATE_INC) && (m_BtnInc->GetState() == DRG_BUTTON_STATE_UP))
	{
		m_State = GUI_SLIDER_STATE_NONE;
		m_CountIncDec = -1.0f;
	}
	if((m_State == GUI_SLIDER_STATE_DEC) && (m_BtnDec->GetState() == DRG_BUTTON_STATE_UP))
	{
		m_State = GUI_SLIDER_STATE_NONE;
		m_CountIncDec = -1.0f;
	}
}

GUI_Slider* GUI_Slider::CreateChildSlider(GUI_Widget* parent, int width, int height, float val, float min, float max, int flags)
{
	GUI_Slider* slider = new GUI_Slider();
	slider->UseAsScriptObject();
	slider->Init();
	slider->SetParent(parent);
	slider->m_Min = min;
	slider->m_Max = max;
	slider->m_Scale = 1.0f;
	slider->m_FlagsSlider |= flags;
	slider->m_Sens.minv.x = 0;
	slider->m_Sens.minv.y = 0;
	slider->m_Sens.maxv.x = (float)(width + 20);
	slider->m_Sens.maxv.y = (float)(height);
	slider->SetValue(val);
	slider->UpdateSize();

	slider->m_BtnInc = GUI_Button::CreateChildButton(slider, ICON_ARRW_RIGHT, ICON_ARRW_RIGHT_UP, ICON_ARRW_RIGHT_DOWN, BtnCallBckInc, slider, 0);
	slider->m_BtnDec = GUI_Button::CreateChildButton(slider, ICON_ARRW_LEFT, ICON_ARRW_LEFT_UP, ICON_ARRW_LEFT_DOWN, BtnCallBckDec, slider, 0);

	slider->m_BtnDec->SetPos(0.0f, 2.0f);
	slider->m_BtnInc->SetPos((float)width - 10.0f, 2.0f);

	return slider;
}

void GUI_Slider::IncrementSlider()
{
	m_State = GUI_SLIDER_STATE_INC;
	SetValue(GetValue() + m_Scale);
	m_CountIncAmt = SLIDER_INC_DEC_TIME_MAX;
	m_CountIncDec = m_CountIncAmt;
}

void GUI_Slider::DecrementSlider()
{
	m_State = GUI_SLIDER_STATE_DEC;
	SetValue( GetValue() - m_Scale);
	m_CountIncAmt = SLIDER_INC_DEC_TIME_MAX;
	m_CountIncDec = m_CountIncAmt;
}



void GUI_Scroll::BtnCallBckInc(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	GUI_Scroll* scroll = (GUI_Scroll*)data;
	if((btn->GetTypeCB() == GUI_CB_BUTTON_DOWN) && (scroll->m_State==GUI_SCROLLBAR_STATE_NONE))
	{
		scroll->IncrementScroll();
	}
	else if((btn->GetTypeCB() == GUI_CB_BUTTON_CLICK) && (scroll->m_State==GUI_SCROLLBAR_STATE_INC))
	{
		scroll->m_State = GUI_SCROLLBAR_STATE_NONE;
		scroll->m_CountIncDec = -1.0f;
	}
}

void GUI_Scroll::BtnCallBckDec(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	GUI_Scroll* scroll = (GUI_Scroll*)data;
	if((btn->GetTypeCB() == GUI_CB_BUTTON_DOWN) && (scroll->m_State==GUI_SCROLLBAR_STATE_NONE))
	{
		scroll->DecrementScroll();
	}
	else if((btn->GetTypeCB() == GUI_CB_BUTTON_CLICK) && (scroll->m_State==GUI_SCROLLBAR_STATE_DEC))
	{
		scroll->m_State = GUI_SCROLLBAR_STATE_NONE;
		scroll->m_CountIncDec = -1.0f;
	}
}


GUI_Scroll::GUI_Scroll()
{
	m_FlagsScroll = 0;
	m_Value = 0.0f;
	m_Min = 0.0f;
	m_Max = 100.0f;
	m_Scale = 1.0f;
	m_Size = 16.0f;
	m_CountIncDec = -1.0f;
	m_Type = GUI_SCROLLBAR_VERT;
	m_State = GUI_SCROLLBAR_STATE_NONE;

	m_WidgetType = GUI_TYPE_SCROLL;
}

GUI_Scroll::~GUI_Scroll()
{
	Destroy();
}

void GUI_Scroll::Init()
{
	m_Min = 0.0f;
	m_Max = 100.0f;
	m_Scale = 1.0f;
	m_Type = GUI_SCROLLBAR_VERT;
	m_State = GUI_SCROLLBAR_STATE_NONE;
	m_FlagsWidget |= DRG_WIDGET_FOCUSABLE;
	m_FlagsScroll = 0;
}

void GUI_Scroll::Reinit()
{
}

void GUI_Scroll::Destroy()
{
}

void GUI_Scroll::Draw(GUI_Drawing* draw)
{
	if(IsHidden())
		return;

	if ((m_FlagsScroll & DRG_SCROLLBAR_AUTOHIDE) && ((m_ScrollBox.maxv.x-m_ScrollBox.minv.x) <= 0.01f))
		return;

	if(m_CountIncDec>0.0f)
	{
		m_CountIncDec -= 0.003f;//drgEngine::getInstance()->GetDeltatTime();
		if(m_CountIncDec<0.0f)
		{
			m_CountIncAmt *= 0.8f;
			if(m_CountIncAmt<SCROLL_INC_DEC_TIME_MIN)
				m_CountIncAmt = SCROLL_INC_DEC_TIME_MIN;
			if(m_State==GUI_SCROLLBAR_STATE_INC)
			{
				SetValue(m_Value+m_Scale);
				m_CountIncDec = m_CountIncAmt;
			}
			else if(m_State==GUI_SCROLLBAR_STATE_DEC)
			{
				SetValue(m_Value-m_Scale);
				m_CountIncDec = m_CountIncAmt;
			}
		}
	}

	if(m_Type == GUI_SCROLLBAR_HORIZ)
		draw->GetStyle()->ScrollbarHoriz(this, m_State, m_ScrollBox.minv.x, m_ScrollBox.maxv.x);
	else if(m_Type == GUI_SCROLLBAR_VERT)
		draw->GetStyle()->ScrollbarVert(this, m_State, m_ScrollBox.minv.y, m_ScrollBox.maxv.y);
	m_BtnInc->Draw(draw);
	m_BtnDec->Draw(draw);

	GUI_Widget::Draw(draw);
}

void GUI_Scroll::UpdateSize()
{
	if(m_Type == GUI_SCROLLBAR_HORIZ)
	{
		if((m_Parent!=NULL) && (m_FlagsScroll & DRG_SCROLLBAR_EXPAND))
		{
			m_Sens.minv.x = 0.0f;
			m_Sens.maxv.x = m_Parent->WidthView();
		}
		m_Sens.maxv.y = m_Sens.minv.y + m_Size;
		m_BtnDec->Resize(0.0f, m_Size, m_Sens.minv.x, m_Sens.minv.x+m_Size);
		m_BtnInc->Resize(0.0f, m_Size, m_Sens.maxv.x-m_Size, m_Sens.maxv.x);
	}
	else if(m_Type == GUI_SCROLLBAR_VERT)
	{
		if((m_Parent!=NULL) && (m_FlagsScroll & DRG_SCROLLBAR_EXPAND))
		{
			m_Sens.minv.y = 0.0f;
			m_Sens.maxv.y = m_Parent->HeightView();
		}
		m_Sens.maxv.x = m_Sens.minv.x + m_Size;
		m_BtnDec->Resize(m_Sens.minv.y, m_Sens.minv.y+m_Size, 0.0f, m_Size);
		m_BtnInc->Resize(m_Sens.maxv.y-m_Size, m_Sens.maxv.y, 0.0f, m_Size);
	}
	m_View = m_Sens;
	UpdateScrollPos();
}

void GUI_Scroll::UpdateScrollPos()
{
	float len = (m_Max-m_Min);
	float visible_size = 0.0f;

	if(m_Type == GUI_SCROLLBAR_HORIZ)
		visible_size = (m_View.maxv.x-m_View.minv.x) - (2.0f*m_Size);
	else if(m_Type == GUI_SCROLLBAR_VERT)
		visible_size = (m_View.maxv.y-m_View.minv.y) - (2.0f*m_Size);

	if ((m_FlagsScroll & DRG_SCROLLBAR_AUTOHIDE) && ((len <= 0.0f) || (visible_size <= 0.0f)))
	{
		m_ScrollBox.minv.x = 0.0f;
		m_ScrollBox.maxv.x = 0.0f;
		m_ScrollBox.minv.y = 0.0f;
		m_ScrollBox.maxv.y = 0.0f;
		m_ScrollArea = m_ScrollBox;
		return;
	}

	float slide_size = 1.0f/(len*(1.0f/m_Scale))*(visible_size*(0.33f*m_Size));
	if(slide_size < (0.33f*m_Size))
		slide_size = (0.33f*m_Size);
	if(slide_size > (visible_size*0.9f))
		slide_size = (visible_size*0.9f);

	float slide_pos = ((m_Value-m_Min)/len)*(visible_size-slide_size);

	if(m_Type == GUI_SCROLLBAR_HORIZ)
	{
		m_ScrollArea.minv.x = m_Size;
		m_ScrollArea.maxv.x = m_View.maxv.x - m_Size;
		m_ScrollArea.minv.y = m_View.minv.y;
		m_ScrollArea.maxv.y = m_View.minv.y + m_Size;
		m_ScrollBox = m_ScrollArea;
		m_ScrollBox.minv.x = m_Size + slide_pos;
		m_ScrollBox.maxv.x = m_ScrollBox.minv.x + slide_size;
	}
	else if(m_Type == GUI_SCROLLBAR_VERT)
	{
		m_ScrollArea.minv.x = m_View.minv.x;
		m_ScrollArea.maxv.x = m_View.minv.x + m_Size;
		m_ScrollArea.minv.y = m_Size;
		m_ScrollArea.maxv.y = m_View.maxv.y - m_Size;
		m_ScrollBox = m_ScrollArea;
		m_ScrollBox.minv.y = m_Size + slide_pos;
		m_ScrollBox.maxv.y = m_ScrollBox.minv.y + slide_size;
	}
}

void GUI_Scroll::CalcScrollValue(drgVec2* pos)
{
	drgBBox rOffs(m_ScrollArea);
	WidgetOffsetRect(&rOffs);
	if(m_Type == GUI_SCROLLBAR_HORIZ)
	{
		if((rOffs.maxv.x-rOffs.minv.x)>0.0f)
			SetValue(m_Min+(((pos->x-rOffs.minv.x)/(rOffs.maxv.x-rOffs.minv.x))*((m_Max-m_Min))));
	}
	else if(m_Type == GUI_SCROLLBAR_VERT)
	{
		if((rOffs.maxv.y-rOffs.minv.y)>0.0f)
			SetValue(m_Min+(((pos->y-rOffs.minv.y)/(rOffs.maxv.y-rOffs.minv.y))*((m_Max-m_Min))));
	}
}

bool GUI_Scroll::CallMouseDown(unsigned int winid, int button)
{	
	if( IsSelected() && IsCursorInBounds() == false )
	{
		m_Manager->SetSelectedWidget( NULL );
		return false;
	}

	if(IsHidden())
		return false;

	if(button!=DRG_MOUSE_BUTTON_LEFT)
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool clicked = false;
	clicked = m_BtnInc->CallMouseDown(winid, button);
	if(m_BtnDec->CallMouseDown(winid, button))
		clicked = true;

	if(m_State == GUI_SCROLLBAR_STATE_NONE)
	{
		if( m_Manager && m_Manager->HasSelectedWidget() == false )
		{
			drgBBox rOffs(m_ScrollArea);
			WidgetOffsetRect(&rOffs);
			if(drgInputMouse::IsCursorInside(&rOffs))
			{
				m_State=GUI_SCROLLBAR_STATE_SCROLL;
				SetFocused();
				SetSelected();
				CalcScrollValue(drgInputMouse::GetPos());
				return true;
			}
		}
	}

	return clicked;
}

bool GUI_Scroll::CallMouseClick(unsigned int winid, int button)
{
	if( IsSelected() && IsCursorInBounds() == false )
	{
		m_Manager->SetSelectedWidget( NULL );
		return false;
	}

	if(IsHidden())
		return false;

	if(button!=DRG_MOUSE_BUTTON_LEFT)
		return false;

	if( IsCursorInBounds() == false )
		return false;

	bool clicked = false;
	clicked = m_BtnInc->CallMouseClick(winid, button);
	if(m_BtnDec->CallMouseClick(winid, button))
		clicked = true;

	if(m_State == GUI_SCROLLBAR_STATE_SCROLL)
	{
		m_State = GUI_SCROLLBAR_STATE_NONE;
		m_Manager->SetSelectedWidget( NULL );
	}
	return clicked;
}

void GUI_Scroll::CallMouseUp(unsigned int winid, int button)
{
	if(IsHidden())
		return;

	if(button!=DRG_MOUSE_BUTTON_LEFT)
		return;

	m_BtnInc->CallMouseUp(winid, button);
	m_BtnDec->CallMouseUp(winid, button);

	if(m_State == GUI_SCROLLBAR_STATE_SCROLL)
	{
		m_State = GUI_SCROLLBAR_STATE_NONE;
		m_Manager->SetSelectedWidget( NULL );
	}
}

void GUI_Scroll::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	if( m_Type == GUI_SCROLLBAR_HORIZ )
		return;

	float diff = drgInputMouse::GetWheelPos() - drgInputMouse::GetWheelPrevPos();

	if( rel < 0 )
	{
		m_Scale = rel * SCROLL_MODIFIER;
		DecrementScroll();
	}
	else
	{
		m_Scale = -rel * SCROLL_MODIFIER;
		IncrementScroll();
	}
}

void GUI_Scroll::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if(IsHidden())
		return;

	if(IsSelected() && (m_State==GUI_SCROLLBAR_STATE_SCROLL))
		CalcScrollValue(pos);
	m_BtnInc->CallMouseMove(winid, pos, rel);
	m_BtnDec->CallMouseMove(winid, pos, rel);

	if((m_State==GUI_SCROLLBAR_STATE_INC) && (m_BtnInc->GetState()==DRG_BUTTON_STATE_UP))
	{
		m_State = GUI_SCROLLBAR_STATE_NONE;
		m_CountIncDec = -1.0f;
	}
	if((m_State==GUI_SCROLLBAR_STATE_DEC) && (m_BtnDec->GetState()==DRG_BUTTON_STATE_UP))
	{
		m_State = GUI_SCROLLBAR_STATE_NONE;
		m_CountIncDec = -1.0f;
	}
}

GUI_Scroll* GUI_Scroll::CreateChildScroll(GUI_Widget* parent, GUI_ScrollType type, float val, float min, float max, int flags)
{
	GUI_Scroll* scroll = new GUI_Scroll();
	scroll->UseAsScriptObject();
	scroll->Init();
	scroll->SetParent( parent );
	scroll->m_Min = min;
	scroll->m_Max = max;
	scroll->m_Type = type;
	scroll->m_Scale = 1.0f;
	scroll->m_FlagsScroll |= flags;
	scroll->m_Sens.minv.x = 0;
	scroll->m_Sens.minv.y = 0;
	scroll->m_Sens.maxv.x = 0;
	scroll->m_Sens.maxv.y = 0;
	scroll->SetValue(val);
	if(scroll->m_Type == GUI_SCROLLBAR_HORIZ)
	{
		scroll->m_BtnInc = GUI_Button::CreateChildButton(scroll, ICON_ARRW_RIGHT, ICON_ARRW_RIGHT_UP, ICON_ARRW_RIGHT_DOWN, BtnCallBckInc, scroll, 0);
		scroll->m_BtnDec = GUI_Button::CreateChildButton(scroll, ICON_ARRW_LEFT, ICON_ARRW_LEFT_UP, ICON_ARRW_LEFT_DOWN, BtnCallBckDec, scroll, 0);
	}
	else if(scroll->m_Type == GUI_SCROLLBAR_VERT)
	{
		scroll->m_BtnInc = GUI_Button::CreateChildButton(scroll, ICON_ARRW_DOWN, ICON_ARRW_DOWN_UP, ICON_ARRW_DOWN_DOWN, BtnCallBckInc, scroll, 0);
		scroll->m_BtnDec = GUI_Button::CreateChildButton(scroll, ICON_ARRW_UP, ICON_ARRW_UP_UP, ICON_ARRW_UP_DOWN, BtnCallBckDec, scroll, 0);
	}
	scroll->UpdateSize();
	return scroll;
}

void GUI_Scroll::IncrementScroll()
{
	m_State = GUI_SCROLLBAR_STATE_INC;
	SetValue(GetValue() + m_Scale);
	m_CountIncAmt = SCROLL_INC_DEC_TIME_MAX;
	m_CountIncDec = m_CountIncAmt;
}

void GUI_Scroll::DecrementScroll()
{
	m_State = GUI_SCROLLBAR_STATE_DEC;
	SetValue( GetValue() - m_Scale);
	m_CountIncAmt = SCROLL_INC_DEC_TIME_MAX;
	m_CountIncDec = m_CountIncAmt;
}



void GUI_Spinner::BtnCBSpinnerInc(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	GUI_Spinner* spinner = (GUI_Spinner*)data;

	if((btn->GetTypeCB() == GUI_CB_BUTTON_DOWN))
	{
		spinner->IncrementSpinner();
	}
}

void GUI_Spinner::BtnCBSpinnerDec(drgObjectBase* widget, void* data)
{
	GUI_Button* btn = (GUI_Button*)widget;
	GUI_Spinner* spinner = (GUI_Spinner*)data;

	if((btn->GetTypeCB() == GUI_CB_BUTTON_DOWN))
	{
		spinner->DecrementSpinner();
	}
}

GUI_Spinner::GUI_Spinner()
{
	m_InputBox = NULL;
	m_ButtonUp = NULL;
	m_ButtonDown = NULL;
	m_dSpinnerValue = 0.0;
	m_FlagsSpinner = 0;
	m_dMaxValue = 100.0;
	m_dMinValue = 0.0;
	m_dStepValue = 1.0;
}

GUI_Spinner::~GUI_Spinner()
{
	Destroy();
}

void GUI_Spinner::Init()
{
	m_dSpinnerValue = 0.0;
	m_dMaxValue = 100.0;
	m_dMinValue = 0.0;
	m_dStepValue = 1.0;

	m_FlagsWidget |= DRG_WIDGET_FOCUSABLE;
	m_FlagsSpinner = 0;
	
}

void GUI_Spinner::Reinit()
{
}

void GUI_Spinner::Destroy()
{
}

void GUI_Spinner::Draw(GUI_Drawing* draw)
{
	GUI_Container::Draw( draw );
}

GUI_Spinner* GUI_Spinner::CreateChildSpinner(GUI_Widget* parent, int width, int height, double startVal, double min, double max, int flags)
{
	GUI_Spinner* spinner = new GUI_Spinner();
	spinner->UseAsScriptObject();
	spinner->Init();
	spinner->SetParent(parent);
	spinner->m_dMinValue = min;
	spinner->m_dMaxValue = max;
	spinner->m_dSpinnerValue = startVal;
	spinner->m_FlagsSpinner |= flags;
	spinner->m_Sens.minv.x = 0;
	spinner->m_Sens.minv.y = 0;
	spinner->m_Sens.maxv.x = (float)width;
	spinner->m_Sens.maxv.y = (float)height;
	
	GUI_Table* table = spinner->CreateChildTable( DRG_WIDGET_RESIZE_EXPAND );
	table->AddRow( 15.0f, 1.0f, 0.0f, false );
	table->AddRow( 15.0f, 0.0f, 0.0f, false );
	table->AddCol( 0.0f, 1.0f, 1.0f, false );
	table->AddCol( 30.0f, 0.0f, 0.0f, true );

	GUI_Container* cell = table->CreateCell( 0, 0, 2, 1 );
	spinner->m_InputBox = cell->CreateChildInput( 0 );
	spinner->m_InputBox->AddFlagWidget( DRG_WIDGET_RESIZE_EXPAND );
	spinner->m_InputBox->UpdateSize();

	cell = table->CreateCell( 0, 1, 1, 1 );
	spinner->m_ButtonDown = cell->CreateChildButton( ICON_ARRW_UP, ICON_ARRW_UP_UP, ICON_ARRW_UP_DOWN, BtnCBSpinnerInc, spinner, 0 );

	cell = table->CreateCell( 1, 1 );
	spinner->m_ButtonUp = cell->CreateChildButton( ICON_ARRW_DOWN, ICON_ARRW_DOWN_UP, ICON_ARRW_DOWN_DOWN, BtnCBSpinnerDec, spinner, 0 );
	
	char valToStr[50] = "";
	drgString::PrintFormated(valToStr, "%f", spinner->m_dSpinnerValue);
	spinner->m_InputBox->SetText(valToStr);
	spinner->m_InputBox->SetCallBack( OnInputChanged, spinner );

	spinner->m_ButtonUp = GUI_Button::CreateChildButton(spinner, ICON_ARRW_UP, ICON_ARRW_UP_UP, ICON_ARRW_UP_DOWN, BtnCBSpinnerInc, spinner, 0);
	spinner->m_ButtonDown = GUI_Button::CreateChildButton(spinner, ICON_ARRW_DOWN, ICON_ARRW_DOWN_UP, ICON_ARRW_DOWN_DOWN, BtnCBSpinnerDec, spinner, 0);

	spinner->UpdateSize();

	return spinner;
}

void GUI_Spinner::OnInputChanged(drgObjectBase* object, void* data)
{
	GUI_Input* input = (GUI_Input*)object;
	GUI_Spinner* spinner = (GUI_Spinner*)data;

	if( input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST && spinner )
	{
		string8 text = *input->GetText();
		double value = (double)drgString::AtoF( text.c_str() );
		spinner->SetSpinnerValue( value );
	}
}

void GUI_Spinner::IncrementSpinner()
{
	double newValue = GetSpinnerValue() + m_dStepValue;
	if( HasFlagSpinner( GUI_SPINNER_CLAMP_VALUES ) )
	{
		newValue = DRG_FLOOR_DOUBLE( newValue );
	}

	newValue = DRG_MIN( m_dMaxValue, newValue );

	SetSpinnerValue( newValue );

	char valToStr[50] = "";
	drgString::PrintFormated(valToStr, "%f", m_dSpinnerValue);
	m_InputBox->SetText(valToStr);
}

void GUI_Spinner::DecrementSpinner()
{
	double newValue = GetSpinnerValue() - m_dStepValue;
	if( HasFlagSpinner( GUI_SPINNER_CLAMP_VALUES ) )
	{
		newValue = DRG_FLOOR_DOUBLE( newValue );
	}

	newValue = DRG_MAX( m_dMinValue, newValue );

	SetSpinnerValue( newValue );

	char valToStr[50] = "";
	drgString::PrintFormated(valToStr, "%f", m_dSpinnerValue);
	m_InputBox->SetText(valToStr);
}

bool GUI_Spinner::CallMouseDown(unsigned int winid, int button)
{	
	return GUI_Container::CallMouseDown( winid, button );
}

bool GUI_Spinner::CallMouseClick(unsigned int winid, int button)
{
	return GUI_Container::CallMouseClick( winid, button );
}



GUI_ProgressBar::GUI_ProgressBar()
{
	m_nProgress		= 0.0f;
	m_nCurrentVal	= 0.0f;
	m_nCompleteVal	= 0.0f;
	m_Label			= NULL;
	m_Textures[0]	= NULL;
	m_Textures[1]	= NULL;
}

GUI_ProgressBar::~GUI_ProgressBar()
{
	Destroy();
}

void GUI_ProgressBar::Init()
{
	m_nProgress		= 0.0f;
	m_nCurrentVal	= 0.0f;
	m_nCompleteVal	= 0.0f;
	m_FlagsWidget = DRG_WIDGET_TABLE_EMBEDDABLE;
	m_PadLeft = 4.0f;
	m_PadRight = 4.0f;
	m_PadTop = 3.0f;
	m_PadBot = 3.0f;
	m_CallBack = NULL;
	m_CallBackType = GUI_CB_GENRAL;
	m_CallBackData = NULL;
}

void GUI_ProgressBar::Reinit()
{
}

void GUI_ProgressBar::Destroy()
{
}

void GUI_ProgressBar::Draw(GUI_Drawing* draw)
{
	if (IsHidden())
		return;

	if (m_Label != NULL)
	{
		float centerOffset = (WidthView() - m_Label->WidthView()) * 0.5f;
		m_Label->Move( centerOffset, 0.0f );
		m_Label->Draw( draw );
		m_Label->Move( -centerOffset, 0.0f );
	}
		
	draw->GetStyle()->ProgressBar(this, 0, m_nProgress);

	GUI_Widget::Draw(draw);
}
		
void GUI_ProgressBar::Update()
{
	if(m_nCompleteVal <= m_nCurrentVal)
	{
		m_nProgress = 1.0f;
	}
	else
	{
		m_nProgress = (float)(m_nCurrentVal / m_nCompleteVal);

		if(m_nProgress > 1.0f)
		{
			while(m_nProgress > 1.0f)
			{
				m_nProgress = (float)(m_nProgress / 10.0f);
			}
		}

		if(m_nProgress < 0.0f)
		{
			m_nProgress = 0.0f;
		}
	}
}

void GUI_ProgressBar::SetTitle(string16* title)
{
	if(m_Label == NULL)
	{
		m_Label = GUI_Label::CreateLabel(this, GetDrawContext()->GetStyle()->GetDefaultFont(), *title, 0);
	}

	m_Label->SetText(title);
}

void GUI_ProgressBar::SetTexture(char* filenameBG, char* filenameProgress)
{
	m_Textures[0] = drgTexture::LoadFromFile(filenameBG);
	m_Textures[1] = drgTexture::LoadFromFile(filenameProgress);
}
		
GUI_ProgressBar* GUI_ProgressBar::CreateChildProgressBar(GUI_Widget* parent, int width, int height, float curVal, float completeVal, const char* label, unsigned int flags)
{
	GUI_ProgressBar* progressbar = new GUI_ProgressBar();
	progressbar->UseAsScriptObject();
	progressbar->Init();
	progressbar->SetParent(parent);
	progressbar->m_Sens.minv.x = 0;
	progressbar->m_Sens.minv.y = 0;
	progressbar->m_Sens.maxv.x = (float)(width);
	progressbar->m_Sens.maxv.y = (float)(height);

	progressbar->m_nCurrentVal = curVal;
	progressbar->m_nCompleteVal = completeVal;
	progressbar->Update();

	string16 strLabel = "";
	strLabel.concat(label);
	progressbar->SetTitle(&strLabel);

	//progressbar->Set

	progressbar->UpdateSize();
	return progressbar;
}



GUI_StatusBar::GUI_StatusBar()
{
	m_Label = NULL;
	m_Font = drgFont::GetDefaultFont();
	
	m_CurrentStatusLevel = DRG_DEBUG_LEVEL_DEFAULT;
	m_CurrentStatus = NULL;
}

GUI_StatusBar::~GUI_StatusBar()
{
	if(m_Label != NULL)
	{
		delete(m_Label);
		m_Label = NULL;
	}

	if(m_CurrentStatus != NULL)
	{
		free(m_CurrentStatus);
		m_CurrentStatus = NULL;
	}
}

void  GUI_StatusBar::Init()
{
	Reinit();
}

void  GUI_StatusBar::Reinit()
{
	if(m_Label == NULL)
	{
		m_Label = GUI_Label::CreateLabel(this, m_Font, "", 0x00);
	}

	float fontHeight = 1.5f * (m_Font == NULL ? 16 : (float)m_Font->GetFontSize());
	
	m_Sens.minv.x = 0.0f;
	m_Sens.minv.y = 0.0f;
	m_Sens.maxv.x = 0.0f;
	m_Sens.maxv.y = m_Sens.minv.y + fontHeight;
	UpdateSize();
}

void  GUI_StatusBar::Destroy()
{

}

void  GUI_StatusBar::Draw(GUI_Drawing* draw)
{
	if(IsHidden())
		return;

	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	draw->GetStyle()->StatusBarBackground(this, m_CurrentStatusLevel);
	
	if(m_CurrentStatus != NULL)
		m_Label->SetText(m_CurrentStatus);
	m_Label->Draw(draw);

	draw->GetRenderCache()->SetClipRect( top, bot, left, right );

	GUI_Widget::Draw(draw);
}

GUI_StatusBar* GUI_StatusBar::CreateStatusBar(GUI_Widget* parent, drgFont* font)
{
	GUI_StatusBar* statusBar = new GUI_StatusBar();
	statusBar->UseAsScriptObject();
	statusBar->m_Font = font;
	statusBar->SetParent(parent);
	statusBar->Init();	
	statusBar->AddFlagWidget(DRG_WIDGET_RESIZE_HFILL);

	return statusBar;
}

void GUI_StatusBar::HandleChangeStatus(DRG_DEBUG_LEVEL debugLevel, const char* message, int size)
{
	if(debugLevel >= m_CurrentStatusLevel)
	{
		m_CurrentStatusLevel = debugLevel;
			
		int length = DRG_MIN(size, STATUS_BUFFER_LEN) + 1;

		if(m_CurrentStatus == NULL)
			m_CurrentStatus = (char*)malloc(length * sizeof(char));
		else
			m_CurrentStatus = (char*)realloc(m_CurrentStatus, length * sizeof(char));
		memcpy(m_CurrentStatus, message, length);
	}
}

void GUI_StatusBar::HandleChangeStatus(int debugLevel, const char* message, int size)
{
	HandleChangeStatus((DRG_DEBUG_LEVEL)debugLevel, message, size);
}

void GUI_StatusBar::Clear()
{
	m_CurrentStatusLevel = DRG_DEBUG_LEVEL_DEFAULT;
	m_CurrentStatus[0] = '\0';
	m_Label->SetText(m_CurrentStatus);
}


drgSelectObject::drgSelectObject(drgObjectBase* object)
{
	m_Object = object;
}

GUI_List::GUI_List()
{
	m_FlagsList = 0;
	m_List.m_Depth = 0;
	m_bMultiSelect = false;
	m_State = GUI_LIST_STATE_UP;
	m_TimeOfDragStart = 0.0f;

	m_OnItemExpand = NULL;
	m_OnItemExpandData = NULL;

	m_Selected = NULL;

	m_WidgetType = GUI_TYPE_LIST;

	m_SelectedObjs = new drgObjectList();
}

GUI_List::~GUI_List()
{
	Destroy();
}

void GUI_List::Init()
{
	m_PadTop = 2.0f;
	m_PadBot = 2.0f;
	m_PadLeft = 6.0f;
	m_PadRight = 6.0f;
	m_List.m_Owner = this;
	m_List.m_Depth = 0;
	m_CallBack = NULL;
	m_CallBackType = GUI_CB_GENRAL;
	m_CallBackData = NULL;
	m_Hover = NULL;
}

void GUI_List::Reinit()
{
}

void GUI_List::Destroy()
{
	if (m_SelectedObjs == NULL)
		return;

	drgSelectObject* item = (drgSelectObject*)m_SelectedObjs->GetLast();
	while (item)
	{
		drgSelectObject* prev = (drgSelectObject*)m_SelectedObjs->GetPrev(item);
		m_SelectedObjs->Remove(item);
		//delete item->m_Object;
		delete item;

		item = prev;
	}

	SAFE_DELETE(m_SelectedObjs);
}

void GUI_List::Draw(GUI_Drawing* draw)
{
	if (IsHidden())
		return;

	drgFont* font = draw->GetStyle()->GetDefaultFont();
	drgColor fontcolor = draw->GetStyle()->GetDefaultColor(TITLEBAR_CAPTION_COLOR);
	drgColor fontSelectedColor = draw->GetStyle()->GetDefaultColor(TLIST_TEXT_SEL_COLOR);
	drgColor iconcolor = draw->GetStyle()->GetDefaultColor(ICON_COLOR);
	drgBBox rOffsArw = m_View;
	rOffsArw.minv.x += 5.0f;
	rOffsArw.maxv.x = rOffsArw.minv.x;
	WidgetOffsetRect(&rOffsArw);

	drgBBox rOffsIcon = rOffsArw;
	drgBBox rOffsTxt = rOffsArw;
	rOffsArw.maxv.x += (font->GetFontSize()*0.6f);
	rOffsArw.minv.y += (font->GetFontSize()*0.3f);
	rOffsArw.maxv.y = (rOffsArw.minv.y + (font->GetFontSize()*0.6f));
	rOffsIcon.minv.x = rOffsArw.minv.x;

	if (m_FlagsList & DRG_LIST_TREE)
		rOffsIcon.minv.x = rOffsArw.maxv.x + (font->GetFontSize()*0.05f);

	rOffsIcon.maxv.x = (rOffsIcon.minv.x + (font->GetFontSize()*0.9f));
	rOffsIcon.minv.y += (font->GetFontSize()*0.25f);
	rOffsIcon.maxv.y = (rOffsIcon.minv.y + (font->GetFontSize()*0.9f));
	rOffsTxt.minv.x = rOffsIcon.minv.x;

	if (m_FlagsList & DRG_LIST_ICONS)
		rOffsTxt.minv.x = rOffsIcon.maxv.x + (font->GetFontSize()*0.1f);

	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	if (m_FlagsList & DRG_LIST_TREE)
		DrawInternalTree(&m_List, draw, &rOffsArw, &rOffsIcon, &rOffsTxt, font, fontcolor, fontSelectedColor, iconcolor, (float)font->GetFontSize(), 0);
	else
		DrawInternal(&m_List, draw, &rOffsIcon, &rOffsTxt, font, fontcolor, fontSelectedColor, iconcolor, (float)font->GetFontSize(), 0);

	draw->GetRenderCache()->SetClipRect(top, bot, left, right);

	GUI_Widget::Draw(draw);
}

void GUI_List::DrawInternal(GUI_ListItem* item, GUI_Drawing* draw, drgBBox* bbox_icon, drgBBox* bbox_txt, drgFont* font, drgColor textColor, drgColor textSelectedColor, drgColor iconcolor, float fontsize, int level)
{
	GUI_ListItem* itr = (GUI_ListItem*)item->GetFirst();
	while (itr)
	{
		drgColor currentTextColor = textColor;
		if (itr->IsSelected())
		{
			drgBBox selbox = m_View;
			WidgetOffsetRect(&selbox);
			selbox.minv.y = bbox_icon->minv.y - 2.0f;
			selbox.maxv.y = bbox_icon->maxv.y + 2.0f;
			drgColor tmp_color = draw->GetStyle()->GetDefaultColor(TLIST_SEL_COLOR);
			draw->DrawRectDirect(selbox, &tmp_color);
			currentTextColor = textSelectedColor;
		}

		if (m_FlagsList & DRG_LIST_ICONS)
			draw->DrawRectDirect(*bbox_icon, &iconcolor, draw->GetStyle()->GetDefaultIcon(itr->m_Icon));
		font->Print(draw->GetCommandBuffer(), currentTextColor, (int)bbox_txt->minv.x, (int)bbox_txt->minv.y, itr->GetText()->str());
		bbox_icon->minv.y += (fontsize + 2.0f);
		bbox_icon->maxv.y += (fontsize + 2.0f);
		bbox_txt->minv.y += (fontsize + 2.0f);

		itr = (GUI_ListItem*)item->GetNext(itr);
	}
}

void GUI_List::DrawInternalTree(GUI_ListItem* item, GUI_Drawing* draw, drgBBox* bbox_arw, drgBBox* bbox_icon, drgBBox* bbox_txt, drgFont* font, drgColor textColor, drgColor textSelectedColor, drgColor iconcolor, float fontsize, int level)
{
	GUI_ListItem* itr = (GUI_ListItem*)item->GetFirst();
	while (itr)
	{
		if (itr->IsSelected())
		{
			drgBBox selbox = m_View;
			WidgetOffsetRect(&selbox);
			selbox.minv.y = bbox_icon->minv.y - 2.0f;
			selbox.maxv.y = bbox_icon->maxv.y + 2.0f;
			float margin = draw->GetStyle()->GetBorderWidth();
			selbox.minv.x += margin;
			selbox.maxv.x -= margin;
			drgColor tmp_color = draw->GetStyle()->GetDefaultColor(TLIST_SEL_COLOR);
			draw->DrawRectDirect(selbox, &tmp_color);
		}

		if (itr->m_IsDragReceiver)
		{
			drgBBox selbox = m_View;
			WidgetOffsetRect(&selbox);
			selbox.minv.y = bbox_icon->minv.y - 2.0f;
			selbox.maxv.y = bbox_icon->maxv.y + 2.0f;
			float margin = draw->GetStyle()->GetBorderWidth();
			selbox.minv.x += margin;
			selbox.maxv.x -= margin;
			drgColor tmp_color = draw->GetStyle()->GetDefaultColor(TLIST_RECEIVER_COLOR);
			draw->DrawRectDirect(selbox, &tmp_color);
		}

		if (itr->GetCount() > 0)
			draw->GetStyle()->CollapseButtonIcon(*bbox_arw, (~(itr->m_FlagsListItem) & DRG_LIST_ITM_EXPAND) != 0, (itr->m_FlagsListItem & DRG_LIST_ITM_HOVER) != 0);

		if (m_FlagsList & DRG_LIST_ICONS)
			draw->DrawRectDirect(*bbox_icon, &iconcolor, draw->GetStyle()->GetDefaultIcon(itr->m_Icon));

		drgColor currentTextColor = itr->IsSelected() ? textSelectedColor : textColor;
		font->Print(draw->GetCommandBuffer(), currentTextColor, (int)bbox_txt->minv.x, (int)bbox_txt->minv.y, itr->GetText()->str());
		bbox_arw->minv.y += (fontsize + 2.0f);
		bbox_arw->maxv.y += (fontsize + 2.0f);
		bbox_icon->minv.y += (fontsize + 2.0f);
		bbox_icon->maxv.y += (fontsize + 2.0f);
		bbox_txt->minv.y += (fontsize + 2.0f);

		if ((itr->GetCount() > 0) && (itr->m_FlagsListItem&DRG_LIST_ITM_EXPAND))
		{
			bbox_arw->minv.x += TREE_INDENT_SIZE;
			bbox_arw->maxv.x += TREE_INDENT_SIZE;
			bbox_icon->minv.x += TREE_INDENT_SIZE;
			bbox_icon->maxv.x += TREE_INDENT_SIZE;
			bbox_txt->minv.x += TREE_INDENT_SIZE;
			bbox_txt->maxv.x += TREE_INDENT_SIZE;
			DrawInternalTree(itr, draw, bbox_arw, bbox_icon, bbox_txt, font, textColor, textSelectedColor, iconcolor, fontsize, level + 1);
			bbox_arw->minv.x -= TREE_INDENT_SIZE;
			bbox_arw->maxv.x -= TREE_INDENT_SIZE;
			bbox_icon->minv.x -= TREE_INDENT_SIZE;
			bbox_icon->maxv.x -= TREE_INDENT_SIZE;
			bbox_txt->minv.x -= TREE_INDENT_SIZE;
			bbox_txt->maxv.x -= TREE_INDENT_SIZE;
		}

		itr = (GUI_ListItem*)item->GetNext(itr);
	}
}

void GUI_List::DragDraw(GUI_Drawing* draw)
{
	drgBBox dragbox;
	float fontsize = 22.0f;
	float dragsize = ((m_Sens.maxv.x - m_Sens.minv.x)*0.8f);
	drgColor color = draw->GetStyle()->GetDefaultColor(TLIST_DRAG_COLOR);
	if (m_Parent != NULL)
		fontsize = (float)draw->GetStyle()->GetDefaultFont()->GetFontSize();
	float xoff = drgInputMouse::GetPos()->x - (dragsize*0.4f);
	float yoff = drgInputMouse::GetPos()->y - (fontsize*0.5f);
	dragbox.minv.Set(xoff, yoff, 0.0f);
	dragbox.maxv.Set(dragsize + xoff, fontsize + yoff, 0.0f);

	draw->DrawRectDirect(dragbox, &color);

	GUI_ListItem* item = GetSelected();
	if (item)
	{
		drgColor white(255, 255, 255, 255);
		draw->DrawText(NULL, draw->GetStyle()->GetDefaultFont(), item->GetText()->str(), dragbox, &white);
	}
}

void GUI_List::ReceiverDraw(GUI_Drawing* draw)
{
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);

	drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
	rOffs.maxv.y = (rOffs.minv.y + ITEM_HEIGHT(font->GetFontSize()));

	GUI_ListItem* item = NULL;
	if (m_FlagsList & DRG_LIST_TREE)
		item = GetMouseOverItemInternalTree(&m_List, &rOffs, font, (float)font->GetFontSize());
	else
		item = GetMouseOverItemInternal(&m_List, &rOffs, font, (float)font->GetFontSize());

	if (item)
	{
		GUI_ListItem* itr = GetFirstItem();
		while (itr)
		{
			itr->m_IsDragReceiver = false;
			itr = GetNextItemRecursive(itr);
		}

		item->m_IsDragReceiver = true;
	}
}

void GUI_List::DragDone()
{
	RunCallBack(GUI_CB_LIST_DROP);
	m_State = GUI_LIST_STATE_UP;
	m_DragItem = NULL;
	//m_DragHover = NULL;
}

GUI_ListItem* GUI_List::GetSelected()
{
	return m_Selected;
}

void GUI_List::SetSelected(GUI_ListItem* selected)
{
	bool bRemoveSelect = false;

	if (m_Selected)
		m_Selected->m_FlagsListItem &= (~DRG_LIST_ITM_SELECTED);

	if ((selected != NULL) && (selected->m_Owner == this))
	{
		// Multi-Select occurs when the user is holding Shift or CTRL
		if (m_bMultiSelect)
		{
			if (m_Selected != NULL)
			{
				m_Selected->m_FlagsListItem |= DRG_LIST_ITM_SELECTED;
			}

			// If the item hasn't already been added, add it.
			if (!(selected->m_FlagsListItem & DRG_LIST_ITM_SELECTED))
			{
				drgSelectObject* obj = new drgSelectObject(m_Selected);
				m_SelectedObjs->AddHead(obj);
			}
			// If the item has already been added, clear that item from the list.
			else
			{
				bool bRemoved = false;
				drgSelectObject* obj = (drgSelectObject*)m_SelectedObjs->GetLast();

				drgSelectObject* lastSelect = new drgSelectObject(m_Selected);
				m_SelectedObjs->AddHead(lastSelect);

				while (obj)
				{
					drgSelectObject* prev = (drgSelectObject*)m_SelectedObjs->GetPrev(obj);
					GUI_ListItem* item = (GUI_ListItem*)obj->m_Object;

					if (item == selected)
					{
						m_SelectedObjs->Remove(obj);
						item->m_FlagsListItem &= (~DRG_LIST_ITM_SELECTED);
						delete obj;
						bRemoved = true;
						bRemoveSelect = true;
					}

					obj = prev;

					if (bRemoved)
						break;
				}
			}
		}
		// If it isn't a multi-select, clear out the multiple highlight references.
		else
		{
			drgSelectObject* obj = (drgSelectObject*)m_SelectedObjs->GetLast();

			while (obj)
			{
				drgSelectObject* prev = (drgSelectObject*)m_SelectedObjs->GetPrev(obj);
				GUI_ListItem* item = (GUI_ListItem*)obj->m_Object;

				m_SelectedObjs->Remove(obj);
				item->m_FlagsListItem &= (~DRG_LIST_ITM_SELECTED);
				delete obj;
				obj = prev;
			}
		}

		if (!bRemoveSelect)
		{
			m_Selected = selected;
			m_Selected->m_FlagsListItem |= DRG_LIST_ITM_SELECTED;
		}
	}
	else
	{
		m_Selected = NULL;
	}

	RunCallBack(GUI_CB_LIST_CHANGE);
}

bool GUI_List::IsMultiSelect()
{
	return m_bMultiSelect;
}

drgObjectList* GUI_List::GetSelectedList()
{
	return m_SelectedObjs;
}

GUI_ListItem* GUI_List::GetHovered()
{
	return m_Hover;
}

void GUI_List::SetHovered(GUI_ListItem* hovered)
{
	if (m_Hover)
		m_Hover->m_FlagsListItem &= (~DRG_LIST_ITM_HOVER);
	if ((hovered != NULL) && (hovered->m_Owner == this))
	{
		m_Hover = hovered;
		m_Hover->m_FlagsListItem |= DRG_LIST_ITM_HOVER;
	}
	else
	{
		m_Hover = NULL;
	}
}

unsigned int GUI_List::GetNumChild()
{
	unsigned int count = 0;
	GUI_ListItem* item = GetFirstItem();
	while (item)
	{
		count++;
		item = GetNextItem(item);
	}

	return count;
}

GUI_ListItem* GUI_List::GetFirstItem()
{
	return (GUI_ListItem*)m_List.GetFirst();
}

GUI_ListItem* GUI_List::GetNextItem(GUI_ListItem* item)
{
	return GetNextItemRecursive(item);
}

GUI_ListItem* GUI_List::AddItem(string16 text, DEFAULT_TEXTURE icon, unsigned int flags)
{
	GUI_ListItem* item = m_List.AddItem(text, icon, flags);

	//
	// We just grew, so let's make sure our parent's can react to that.
	//

	if (m_Parent)
	{
		m_Parent->UpdateSize();
	}

	return item;
}

bool GUI_List::GetItemPathInternal(GUI_ListItem* list, string16* path, GUI_ListItem* item, char concatChar)
{
	GUI_ListItem* itr = (GUI_ListItem*)list->GetFirst();
	while (itr)
	{
		if (itr == item)
		{
			(*path) = (*itr->GetText());
			return true;
		}
		if (itr->GetCount() > 0)
		{
			if (GetItemPathInternal(itr, path, item, concatChar) == true)
			{
				(*path) = ((*itr->GetText()) + concatChar + (*path));
				return true;
			}
		}
		itr = (GUI_ListItem*)list->GetNext(itr);
	}
	return false;
}

string16 GUI_List::GetItemPath(GUI_ListItem* item, char concatChar)
{
	string16 path = "";
	if (m_List.GetFirst() != NULL)
		GetItemPathInternal(&m_List, &path, item, concatChar);
	return path;
}

GUI_ListItem* GUI_List::GetItemByPathInternal(GUI_ListItem* list, string16 path)
{
	int str_len = path.length();
	int find_dot = path.find('.');
	string16 test_path = path.substr(0, find_dot);
	path = path.substr(find_dot + 1);

	if (test_path != (*list->GetText()))
		return NULL;

	GUI_ListItem* itr = (GUI_ListItem*)list->GetFirst();
	while (itr)
	{
		if (path == (*itr->GetText()))
		{
			return itr;
		}
		if (itr->GetCount() > 0)
		{
			GUI_ListItem* ret = GetItemByPathInternal(itr, path);
			if (ret != NULL)
				return ret;
		}
		itr = (GUI_ListItem*)list->GetNext(itr);
	}
	return NULL;
}

GUI_ListItem* GUI_List::GetItemByPath(string16 path)
{
	if (m_List.GetFirst() == NULL)
		return NULL;
	string16 full_path = path;
	GUI_ListItem* itr = (GUI_ListItem*)m_List.GetFirst();
	GUI_ListItem* ret = GetItemByPathInternal(itr, full_path);
	return ret;
}

void GUI_List::ClearList()
{
	while (m_List.GetFirst() != NULL)
		RemoveItemInternal(&m_List, (GUI_ListItem*)m_List.GetFirst());
}

void GUI_List::RemoveItem(GUI_ListItem* item)
{
	RemoveItemInternal(&m_List, item);
}

void GUI_List::RemoveAllItems()
{
	RemoveItemChildren(&m_List);
}

void GUI_List::RemoveItemChildren(GUI_ListItem* item)
{
	drgObjectBase* itemChild = item->GetLast();
	while (itemChild)
	{
		item->Remove(itemChild);
		itemChild->DelayedDelete();
		itemChild = item->GetLast();
	}
}

bool GUI_List::RemoveItemInternal(GUI_ListItem* list, GUI_ListItem* item)
{
	if (list->InList(item))
	{
		list->Remove(item);
		return true;
	}

	GUI_ListItem* itr = (GUI_ListItem*)list->GetFirst();
	while (itr)
	{
		if (itr->GetCount() > 0)
		{
			if (RemoveItemInternal(itr, item) == true)
				return true;
		}
		itr = (GUI_ListItem*)list->GetNext(itr);
	}
	return false;
}

GUI_ListItem* GUI_List::FindParent(GUI_ListItem* list, GUI_ListItem* item)
{
	if (list->InList(item))
		return list;

	GUI_ListItem* itr = (GUI_ListItem*)list->GetFirst();
	while (itr)
	{
		if (itr->GetCount() > 0)
		{
			GUI_ListItem* parent = FindParent(itr, item);
			if (parent != NULL)
				return parent;
		}
		itr = (GUI_ListItem*)list->GetNext(itr);
	}
	return NULL;
}

void GUI_List::RemoveItemAddNext(GUI_ListItem* item, GUI_ListItem* prev)
{
	if (prev == item)
		return;
	GUI_ListItem* parent = FindParent(&m_List, prev);
	if ((parent == NULL) || (parent == item))
		return;
	RemoveItem(item);
	parent->AddAfter(item, prev);
	UpdateSize();
}

void GUI_List::RemoveItemAddChild(GUI_ListItem* item, GUI_ListItem* parent)
{
	if (parent == item)
		return;
	RemoveItem(item);
	parent->AddHead(item);
	UpdateSize();
}

void GUI_List::UpdateSize()
{
	drgBBox rOffs;
	float fontsize = 22.0f;
	rOffs.minv.y = 0.0f;
	rOffs.minv.x = 5.0f;
	rOffs.maxv = rOffs.minv;
	m_Sens = rOffs;
	m_Sens.minv.x = 0.0f;
	if (m_Parent != NULL)
	{
		if (GetDrawContext() != NULL && GetDrawContext()->GetStyle() != NULL)
		{
			drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
			fontsize = (float)font->GetFontSize();
			if (m_FlagsList & DRG_LIST_TREE)
				AddBBoxInternalTree(&m_List, &rOffs, font, fontsize);
			else
				AddBBoxInternal(&m_List, &rOffs, font, fontsize);
		}
	}
	m_Sens.Add(&rOffs.maxv);
	if ((m_Parent != NULL) && (m_FlagsList & DRG_LIST_HFILL))
	{
		m_Sens.minv.x = DRG_MAX(m_Sens.minv.x, 0.0f);
		m_Sens.maxv.x = DRG_MAX(m_Sens.maxv.x, m_Parent->WidthView());
	}
	GUI_Widget::UpdateSize();
}

void GUI_List::AddBBoxInternal(GUI_ListItem* item, drgBBox* bbox, drgFont* font, float fontsize)
{
	GUI_ListItem* itr = (GUI_ListItem*)item->GetFirst();
	while (itr)
	{
		if (m_FlagsList & DRG_LIST_ICONS)
			bbox->maxv.x = DRG_MAX(bbox->maxv.x, bbox->minv.x + fontsize + font->StringSize(itr->GetText()->str()));
		else
			bbox->maxv.x = DRG_MAX(bbox->maxv.x, bbox->minv.x + font->StringSize(itr->GetText()->str()));
		bbox->minv.y += ITEM_HEIGHT(fontsize);
		bbox->maxv.y = DRG_MAX(bbox->maxv.y, bbox->minv.y);
		itr = (GUI_ListItem*)item->GetNext(itr);
	}
}

void GUI_List::AddBBoxInternalTree(GUI_ListItem* item, drgBBox* bbox, drgFont* font, float fontsize)
{
	GUI_ListItem* itr = (GUI_ListItem*)item->GetFirst();
	while (itr)
	{
		if (m_FlagsList & DRG_LIST_ICONS)
			bbox->maxv.x = DRG_MAX(bbox->maxv.x, bbox->minv.x + fontsize + fontsize + font->StringSize(itr->GetText()->str()));
		else
			bbox->maxv.x = DRG_MAX(bbox->maxv.x, bbox->minv.x + fontsize + font->StringSize(itr->GetText()->str()));
		bbox->minv.y += ITEM_HEIGHT(fontsize);
		if ((itr->GetCount() > 0) && (itr->m_FlagsListItem&DRG_LIST_ITM_EXPAND))
		{
			bbox->minv.x += TREE_INDENT_SIZE;
			AddBBoxInternalTree(itr, bbox, font, fontsize);
			bbox->minv.x -= TREE_INDENT_SIZE;
		}
		bbox->maxv.y = DRG_MAX(bbox->maxv.y, bbox->minv.y);
		itr = (GUI_ListItem*)item->GetNext(itr);
	}
}

void GUI_List::CheckForKeyUp(int key)
{
	switch (key)
	{
	case DRG_KEY_LSHIFT:
	case DRG_KEY_RSHIFT:
	case DRG_KEY_LCTRL:
	case DRG_KEY_RCTRL:
		m_bMultiSelect = false;
		break;
	default:
		break;
	}
}

void GUI_List::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	if (IsDisabled() || IsHidden())
		return;
	if (!IsFocused())
		return;

	if (!down)
	{
		CheckForKeyUp(key);
		return;
	}

	if (charval < 0)
	{
		switch (key)
		{
		case DRG_KEY_LSHIFT:
		case DRG_KEY_RSHIFT:
		case DRG_KEY_LCTRL:
		case DRG_KEY_RCTRL:
			m_bMultiSelect = true;
			break;
		default:
			break;
		}
	}

	return;
}

bool GUI_List::CallMouseDown(unsigned int winid, int button)
{
	if (m_Parent == NULL)
		return false;
	if (IsHidden())
		return false;
	if (button != DRG_MOUSE_BUTTON_LEFT)
		return false;

	m_State = GUI_LIST_STATE_DOWN;
	m_TimeOfDragStart = drgEngine::GetTotalTime();

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs))
		return false;

	drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
	rOffs.maxv.y = (rOffs.minv.y + ITEM_HEIGHT(font->GetFontSize()));

	GUI_ListItem* item = NULL;
	if (m_FlagsList & DRG_LIST_TREE)
		item = GetMouseOverItemInternalTree(&m_List, &rOffs, font, (float)font->GetFontSize());
	else
		item = GetMouseOverItemInternal(&m_List, &rOffs, font, (float)font->GetFontSize());

	if (item != NULL)
	{
		m_DragItem = NULL;
		rOffs.minv.x += (5.0f + (TREE_INDENT_SIZE*(item->m_Depth - 1)));
		rOffs.maxv.x = (rOffs.minv.x + (font->GetFontSize()*0.6f));
		if (drgInputMouse::IsCursorInside(&rOffs))
		{
			item->EnableExpand(!item->IsExpanded());
		}
		else
		{
			item->Select(true);
			m_DragItem = item;
		}
	}

	return true;
}

bool GUI_List::CallMouseClick(unsigned int winid, int button)
{
	if (m_Parent == NULL) {
		return false;
	}
	if (button != DRG_MOUSE_BUTTON_LEFT) {
		return false;
	}
	GUI_ListItem* itr = GetFirstItem();
	while (itr) {
		itr->m_IsDragReceiver = false;
		itr = GetNextItemRecursive(itr);
	}

	if (IsHidden()) {
		return false;
	}

	m_State = GUI_LIST_STATE_UP;

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs)) {
		return false;
	}
	SetFocused();

	//return CallMouseDoubleClick(winid, button);
	return false;
}

bool GUI_List::CallMouseDoubleClick(unsigned int winid, int button)
{
	if (m_Parent == NULL)
		return false;

	if (IsHidden())
		return false;

	if (button != DRG_MOUSE_BUTTON_LEFT)
		return false;

	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs))
		return false;

	drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
	rOffs.maxv.y = (rOffs.minv.y + ITEM_HEIGHT(font->GetFontSize()));
	GUI_ListItem* item;
	if (m_FlagsList & DRG_LIST_TREE)
		item = GetMouseOverItemInternalTree(&m_List, &rOffs, font, (float)font->GetFontSize());
	else
		item = GetMouseOverItemInternal(&m_List, &rOffs, font, (float)font->GetFontSize());

	if (item != NULL)
	{
		if (drgInputMouse::IsCursorInside(&rOffs))
		{
			RunCallBack(GUI_CB_LIST_ITM_DOUBLE_CLICKED);
		}
	}
	return false;
}

void GUI_List::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if (m_Parent == NULL) {
		return;
	}
	SetHovered(NULL);
	if (IsHidden()) {
		return;
	}
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if (!drgInputMouse::IsCursorInside(&rOffs)) {
		return;
	}
	drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
	rOffs.maxv.y = (rOffs.minv.y + (font->GetFontSize() + 2.0f));
	GUI_ListItem* item;
	if (m_FlagsList & DRG_LIST_TREE) {
		item = GetMouseOverItemInternalTree(&m_List, &rOffs, font, (float)font->GetFontSize());
	}
	else {
		item = GetMouseOverItemInternal(&m_List, &rOffs, font, (float)font->GetFontSize());
	}

	if (m_State == GUI_LIST_STATE_DOWN) {
		//drgPrintOut("now: %f, start: %f, delay: %f\n", drgEngine::GetTotalTime(), m_TimeOfDragStart, DRG_LIST_DRAG_DELAY_IN_CENTISECONDS);
		if (drgEngine::GetTotalTime() - m_TimeOfDragStart >= DRG_LIST_DRAG_DELAY_IN_CENTISECONDS)
		{
			// If I'm draggable, set me as the drag receiver
			if (m_FlagsList & DRG_LIST_DRAGABLE) {
				m_DragHover = NULL;
				SetDragBox();
				m_DragHover = item;
			}
		}
	}
	else if (item != NULL) {
		rOffs.minv.x += (5.0f + (TREE_INDENT_SIZE*(item->m_Depth - 1)));
		rOffs.maxv.x = (rOffs.minv.x + (font->GetFontSize()*0.6f));
		if (drgInputMouse::IsCursorInside(&rOffs))
			item->Hover(true);
	}
}

void GUI_List::CallMouseDrag(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	if (IsDisabled() || IsHidden())
		return;

	assert(m_Manager);

	if (IsCursorInBounds())
	{
		//drgPrintOut("now: %f, start: %f, delay: %f\n", drgEngine::GetTotalTime(), m_TimeOfDragStart, DRG_LIST_DRAG_DELAY_IN_CENTISECONDS);
		if (drgEngine::GetTotalTime() - m_TimeOfDragStart >= DRG_LIST_DRAG_DELAY_IN_CENTISECONDS)
		{
			m_Manager->SetReceiverWidget(this);

			//
			// Now we need to highlight the item we are over.
			//

			drgBBox rOffs(m_Sens);
			WidgetOffsetRect(&rOffs);

			drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
			rOffs.maxv.y = (rOffs.minv.y + (font->GetFontSize() + 2.0f));
			GUI_ListItem* item = NULL;
			if (m_FlagsList & DRG_LIST_TREE)
				item = GetMouseOverItemInternalTree(&m_List, &rOffs, font, (float)font->GetFontSize());
			else
				item = GetMouseOverItemInternal(&m_List, &rOffs, font, (float)font->GetFontSize());

			if (item)
			{
				GUI_ListItem* itr = GetFirstItem();
				while (itr)
				{
					if (itr == item)
					{
						itr->m_IsDragReceiver = true;
					}
					else
					{
						itr->m_IsDragReceiver = false;
					}

					itr = GetNextItemRecursive(itr);
				}
			}
		}
	}
	else if (m_Manager->GetReceiverWidget() == this)
	{
		m_Manager->SetReceiverWidget(NULL);

		GUI_ListItem* item = GetFirstItem();
		while (item)
		{
			if (item)
			{
				item->m_IsDragReceiver = false;
			}

			item = GetNextItem(item);
		}
	}
}

GUI_ListItem* GUI_List::GetMouseOverItemInternal(GUI_ListItem* item, drgBBox* bbox, drgFont* font, float fontsize)
{
	GUI_ListItem* itr = (GUI_ListItem*)item->GetFirst();
	while (itr)
	{
		if (drgInputMouse::IsCursorInside(bbox))
			return itr;
		bbox->minv.y += ITEM_HEIGHT(fontsize);
		bbox->maxv.y += ITEM_HEIGHT(fontsize);
		itr = (GUI_ListItem*)item->GetNext(itr);
	}
	return NULL;
}

GUI_ListItem* GUI_List::GetMouseOverItemInternalTree(GUI_ListItem* _item, drgBBox* bbox, drgFont* font, float fontsize)
{
	GUI_ListItem* itr = (GUI_ListItem*)_item->GetFirst();
	while (itr)
	{
		if (drgInputMouse::IsCursorInside(bbox))
			return itr;
		bbox->minv.y += ITEM_HEIGHT(fontsize);
		bbox->maxv.y += ITEM_HEIGHT(fontsize);
		if ((itr->GetCount() > 0) && (itr->m_FlagsListItem&DRG_LIST_ITM_EXPAND))
		{
			GUI_ListItem* item = GetMouseOverItemInternalTree(itr, bbox, font, fontsize);
			if (item != NULL)
				return item;
		}
		itr = (GUI_ListItem*)_item->GetNext(itr);
	}
	return NULL;
}

GUI_ListItem* GUI_List::GetNextItemRecursive(GUI_ListItem* item)
{
	if (item->GetCount() > 0)
	{
		return item->GetFirstItem();
	}
	else if (item->m_Next)
	{
		return (GUI_ListItem*)item->m_Next;
	}
	else if (item->m_Parent)
	{
		GUI_ListItem* parentToCheck = item->m_Parent;
		while (parentToCheck && parentToCheck->m_Next == NULL)
		{
			parentToCheck = parentToCheck->m_Parent;
		}

		if (parentToCheck == NULL)
			return NULL;
		else
		{
			return (GUI_ListItem*)parentToCheck->m_Next;
		}
	}
	else
	{
		return NULL;
	}
}

GUI_ListItem::GUI_ListItem()
{
	m_UserData = NULL;
	m_FlagsListItem = 0;
	m_Depth = -1;

	m_IsDragReceiver = false;
	m_Parent = NULL;
}

GUI_ListItem::~GUI_ListItem()
{
}

void GUI_ListItem::SetText(string16 text)
{
	m_Text = text;
	m_Owner->UpdateSize();
}

string16* GUI_ListItem::GetText()
{
	return &m_Text;
}

void GUI_ListItem::SetIcon(DEFAULT_TEXTURE icon)
{
	m_Icon = icon;
}

DEFAULT_TEXTURE GUI_ListItem::GetIcon()
{
	return m_Icon;
}

string16 GUI_ListItem::GetItemPath(char concatChar)
{
	return m_Owner->GetItemPath(this, concatChar);
}

void GUI_ListItem::Select(bool select)
{
	if (select == true)
		m_Owner->SetSelected(this);
	else if (m_Owner->GetSelected() == this)
		m_Owner->SetSelected(NULL);
}

bool GUI_ListItem::IsSelected()
{
	return (m_FlagsListItem & DRG_LIST_ITM_SELECTED) != 0;
}

void GUI_ListItem::SetUserData(drgObjectBase* data)
{
	m_UserData = data;
}

drgObjectBase* GUI_ListItem::GetUserData()
{
	return m_UserData;
}

void GUI_ListItem::Hover(bool hover)
{
	if (hover == true)
		m_Owner->SetHovered(this);
	else if (m_Owner->GetHovered() == this)
		m_Owner->SetHovered(NULL);
}

void GUI_ListItem::EnableExpand(bool enable)
{
	enable ? (m_FlagsListItem |= DRG_LIST_ITM_EXPAND) : (m_FlagsListItem &= (~DRG_LIST_ITM_EXPAND));

	if (m_Owner)
	{
		if (m_Owner->m_OnItemExpand)
		{
			m_Owner->m_OnItemExpand(this, m_Owner->m_OnItemExpandData);
		}
		m_Owner->UpdateSize();
	}
}

bool GUI_ListItem::IsExpanded()
{
	return (m_FlagsListItem&DRG_LIST_ITM_EXPAND);
}

unsigned int GUI_ListItem::GetNumChild()
{
	return GetCount();
}

GUI_ListItem* GUI_ListItem::GetFirstItem()
{
	return (GUI_ListItem*)GetFirst();
}

GUI_ListItem* GUI_ListItem::GetNextItem(GUI_ListItem* item)
{
	return (GUI_ListItem*)GetNext(item);
}

GUI_ListItem* GUI_ListItem::AddItem(string16 text, DEFAULT_TEXTURE icon, unsigned int flags)
{
	GUI_ListItem* item = new GUI_ListItem();
	item->UseAsScriptObject();
	item->m_Owner = m_Owner;
	item->m_Icon = icon;
	item->m_Text = text;
	item->m_Depth = m_Depth + 1;
	AddTail(item);
	item->m_Parent = this;
	m_Owner->UpdateSize();
	return item;
}

void GUI_ListItem::UpdateBBox()
{
	drgFont* font = m_Owner->GetDrawContext()->GetStyle()->GetDefaultFont();
}





GUI_DropList::GUI_DropList()
{
	m_FlagsList = 0;
	m_SubMenu = NULL;
	m_State = GUI_DROP_LIST_STATE_NORMAL;

	m_WidgetType = GUI_TYPE_DROP_LIST;
}

GUI_DropList::~GUI_DropList()
{
	Destroy();
}

void GUI_DropList::Init()
{
	m_PadTop = 2.0f;
	m_PadBot = 2.0f;
	m_PadLeft = 6.0f;
	m_PadRight = 6.0f;
	m_CallBack = NULL;
	m_CallBackType = GUI_CB_GENRAL;
	m_CallBackData = NULL;
}

void GUI_DropList::Reinit()
{
}

void GUI_DropList::Destroy()
{
	
}

void GUI_DropList::Draw(GUI_Drawing* draw)
{
	if(IsHidden())
		return;
	
	short top, bot, left, right;
	draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
	SetClipRectWithOffsets(draw, top, bot, left, right);

	draw->GetStyle()->DropListBackground(this, m_Selected);

	draw->GetRenderCache()->SetClipRect(top, bot, left, right);

	GUI_Widget::Draw(draw);
}

GUI_DropListItem* GUI_DropList::GetSelected()
{
	return m_Selected;
}

int GUI_DropList::GetSelectedIndex()
{
	int cur_item = 0;
	GUI_DropListItem* itr = GetFirstItem();
	while(itr)
	{
		if(itr == GetSelected())
		{
			return cur_item;
		}
		cur_item++;
		itr = GetNextItem(itr);
	}

	return -1;
}

void GUI_DropList::SetSelected(GUI_DropListItem* selected)
{
	if((selected != NULL) && (selected->m_Owner==this))
		m_Selected = selected;
	else
		m_Selected = NULL;
	RunCallBack(GUI_CB_LIST_CHANGE);
}

void GUI_DropList::SetSelected(int index)
{
	int cur_item = 0;
	GUI_DropListItem* itr = GetFirstItem();
	while(itr)
	{
		if(cur_item==index)
		{
			SetSelected(itr);
			return;
		}
		cur_item++;
		itr = GetNextItem(itr);
	}
}

unsigned int GUI_DropList::GetNumChild()
{
	return m_List.GetCount();
}

GUI_DropListItem* GUI_DropList::GetFirstItem()
{
	return (GUI_DropListItem*)m_List.GetFirst();
}

GUI_DropListItem* GUI_DropList::GetNextItem(GUI_DropListItem* item)
{
	return (GUI_DropListItem*)m_List.GetNext(item);
}

GUI_DropListItem* GUI_DropList::AddItem(string16 text, unsigned int flags)
{
	GUI_DropListItem* item = new GUI_DropListItem();
	item->UseAsScriptObject();
	item->m_Owner = this;
	item->m_Text = text;
	m_List.AddTail(item);
	if(m_Selected==NULL)
		m_Selected = item;
	UpdateSize();
	return item;
}

void GUI_DropList::ClearList()
{
}

void GUI_DropList::RemoveItem(GUI_DropListItem* item)
{
}

void GUI_DropList::UpdateSize()
{
	float fontsize = 22.0f;
	m_Sens.maxv.Set(m_Sens.minv.x+50.0f, m_Sens.minv.y, 0.0f);
	if(m_Parent != NULL)
	{
		if( GetDrawContext() && GetDrawContext()->GetStyle() )
		{
			drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();
			fontsize = (float)font->GetFontSize();
			GUI_DropListItem* itr = GetFirstItem();
			while(itr)
			{
				m_Sens.maxv.x = DRG_MAX(m_Sens.maxv.x, font->StringSize(itr->GetText()->str())+(fontsize*0.2f));
				itr = GetNextItem(itr);
			}
		}
	}
	m_Sens.maxv.x += (fontsize*1.5f);
	m_Sens.maxv.y = DRG_MAX(m_Sens.maxv.y, m_Sens.minv.y+(fontsize*1.5f));
	GUI_Widget::UpdateSize();
}

void GUI_DropList::MenuCallback(drgObjectBase* widget, void* data)
{
	GUI_DropListItem* item = (GUI_DropListItem*)data;
	item->m_Owner->m_State &= (~GUI_DROP_LIST_STATE_DROP);
	item->Select();
}

void GUI_DropList::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
}

bool GUI_DropList::CallMouseDown(unsigned int winid, int button)
{
	return false;
}

bool GUI_DropList::CallMouseClick(unsigned int winid, int button)
{
	if(m_Parent == NULL)
		return false;

	if(IsHidden() || (button != DRG_MOUSE_BUTTON_LEFT))
		return false;

	if( IsCursorInBounds() == false )
		return false;

	if( (m_State & GUI_DROP_LIST_STATE_DROP) == false )
	{
		if(m_SubMenu == NULL)
		{
			SetupSubMenu();
		}
		m_SubMenu->Show();

		m_State |= GUI_DROP_LIST_STATE_DROP;
		SetCurrentActive(m_SubMenu);
	}
	else
	{
		m_SubMenu->Hide();
		m_State &= ~GUI_DROP_LIST_STATE_DROP;
	}


	return false;
}

void GUI_DropList::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	SetHoverWidget();

	if(m_Parent == NULL)
		return;
	if(IsHidden())
		return;
	m_State &= (~GUI_DROP_LIST_STATE_HOVER);
	drgBBox rOffs(m_Sens);
	WidgetOffsetRect(&rOffs);
	if(!drgInputMouse::IsCursorInside(&rOffs))
		return;
	m_State |= GUI_DROP_LIST_STATE_HOVER;
}

void GUI_DropList::SetupSubMenu()
{
	if( GetDrawContext() == NULL )
		return;

	if( GetDrawContext()->GetStyle() == NULL )
		return;

	drgFont* font = GetDrawContext()->GetStyle()->GetDefaultFont();

	m_SubMenu = GUI_Menu::CreateMenu(font, true, 1, 0.0f);
	m_SubMenu->SetParent( this );
	GUI_DropListItem* itr = GetFirstItem();
	while(itr)
	{
		m_SubMenu->CreateMenuItem(*itr->GetText(), MenuCallback, itr);
		itr = GetNextItem(itr);
	}

	if( m_FlagsList & DRG_DROP_LIST_MENU_ABOVE )
	{
		float width = m_SubMenu->WidthBox();
		float height = m_SubMenu->HeightBox();
		m_SubMenu->SetPos( m_Sens.minv.x, m_Sens.minv.y - height );
		m_SubMenu->SetHeight( height );
		m_SubMenu->SetWidth( width );
	}
}


GUI_DropListItem::GUI_DropListItem()
{
	m_UserData = NULL;
}

GUI_DropListItem::~GUI_DropListItem()
{
}

void GUI_DropListItem::SetText(string16 text)
{
	m_Text = text;
	m_Owner->UpdateSize();
}

string16* GUI_DropListItem::GetText()
{
	return &m_Text;
}

void GUI_DropListItem::Select()
{
	m_Owner->SetSelected(this);
}

void GUI_DropListItem::SetUserData(drgObjectProp* data)
{
	m_UserData = data;
}

drgObjectProp* GUI_DropListItem::GetUserData()
{
	return m_UserData;
}





const int   GUI_Log::DEFAULT_GUI_LOG_OFFSET	= 20;
const float GUI_Log::DEFAULT_POS_X			= 10.0f;
const float GUI_Log::DEFAULT_TOP_POS_Y		= 10.0f;


GUI_Log::GUI_Log()
{
	m_LogStrings = NULL;
	m_DebugContainer = NULL;
}

GUI_Log::~GUI_Log()
{
	ClearAllMessages();
	SAFE_DELETE(m_LogStrings);
	SAFE_DELETE(m_DebugContainer);
}
		
void GUI_Log::Init(GUI_Container* container)
{
	m_DebugContainer = container;
	m_LogStrings = new drgObjectList();
}

void GUI_Log::LogMessage(string16 str)
{
    m_LogStrings->AddHead(m_DebugContainer->CreateChildLabel(str, 0));

    UpdateDebugTextPositioning();
}

void GUI_Log::LogWarning(string16 str)
{
	string16 string = "WARNING: ";
	string.concat(str);

    m_LogStrings->AddHead(m_DebugContainer->CreateChildLabel(string, 0));

    UpdateDebugTextPositioning();
}

void GUI_Log::LogError(string16 str)
{
	string16 string = "ERROR: ";
	string.concat(str);

    m_LogStrings->AddHead(m_DebugContainer->CreateChildLabel(string, 0));

    UpdateDebugTextPositioning();
}

void GUI_Log::ClearAllMessages()
{
    GUI_Label* label = NULL;
    drgObjectBase* obj = m_LogStrings->GetLast();
    drgObjectBase* nextObj = NULL;

    while (obj != NULL)
    {
        nextObj = m_LogStrings->GetPrev(obj);
        label = (GUI_Label*)obj;
        label->SetText("");
        SAFE_DELETE(label);
		SAFE_DELETE(obj);
        obj = nextObj;
    }

	UpdateDebugTextPositioning();
}

void GUI_Log::UpdateDebugTextPositioning()
{
    drgObjectBase* obj = m_LogStrings->GetFirst();
    GUI_Label* label = NULL;
    int nCurMessage = 0;

    while (obj != NULL)
    {
        label = (GUI_Label*)obj;
        //label->Hide();
        label->SetPos(DEFAULT_POS_X, (DEFAULT_TOP_POS_Y + (nCurMessage * DEFAULT_GUI_LOG_OFFSET)));
        //label->Show();

        nCurMessage++;

        obj = m_LogStrings->GetNext(obj);
    }
}
		
GUI_Log* GUI_Log::CreateChildLog(GUI_Widget* parent, unsigned int flags)
{
	GUI_Log* log = new GUI_Log();
	GUI_Container* container = (GUI_Container*)parent;
	log->UseAsScriptObject();
	log->SetParent(parent);
	log->Init(container);

	return log;
}




// Default values used for setting up a drgProperty Table
const float GUI_Table::DEFAULT_PROPERTY_ROW_HEIGHT			= 25.0f;
const float GUI_Table::DEFAULT_PROPERTY_ROW_VAR_HEIGHT		= 0.0f;
const float GUI_Table::DEFAULT_PROPERTY_ROW_PADDING			= 5.0f;
const float GUI_Table::DEFAULT_PROPERTY_COL_WIDTH			= 0.0f;
const float GUI_Table::DEFAULT_PROPERTY_COL_VAR_WIDTH[2]	= {0.5f, 1.0f};
const float GUI_Table::DEFAULT_PROPERTY_COL_PADDING			= 10.0f;

GUI_Table::GUI_Table()
{
	m_Cells = NULL;
	m_Rows = NULL;
	m_Columns = NULL;
	m_NumRows = 0;
	m_NumCols = 0;
	m_TableIsDirty = false;
	m_FlagsWidget |= DRG_WIDGET_RESIZE_EXPAND;	// By default we expand to our parent's width/height.

	m_CurrentNumCellCols = 0;
	m_CurrentNumCellRows = 0;
	m_WidgetType = GUI_TYPE_TABLE;
}

GUI_Table::~GUI_Table()
{
	Destroy();
}

GUI_Table::Row::Row()
{
	m_MinHeight = 0.0f;
	m_MaxHeight = FLT_MAX;
	m_CurrentHeight = 0.0f;
	m_Y = 0.0f;
	m_FixedHeight = 0.0f;
	m_VariableHeight = 0.0f;
	m_Padding = 0.0f;
}

GUI_Table::Column::Column()
{
	m_MinWidth = 0.0f;
	m_MaxWidth = FLT_MAX;
	m_CurrentWidth = 0.0f;
	m_X = 0.0f;
	m_FixedWidth = 0;
	m_VariableWidth = 0.0f;
	m_Padding = 0.0f;
}

GUI_Table::Cell::Cell()
{
	m_RowNum = 0;
	m_ColNum = 0;
	m_SpansRows = 0;
	m_SpansCols = 0;
	m_Container = NULL;
}


	// If you pass resizeTable as false, the table will not be expanded, and can't be accessed until ReallocateTable is called.
	// fixedHeight is in pixels, variableHeight is a percentage 0-1.  only one of these arguments is used.
void 
GUI_Table::AddRow( float fixedHeight, float variableHeight, float padding, bool resizeTable )
{
	if( variableHeight == 0.0f && fixedHeight == 0.0f )
	{
		assert( "Rows must have a fxed or variable non zero height" && 0 );
		return;
	}

	//
	// Handle all our dirty logic
	//

	if( m_TableIsDirty == false )
	{
		m_CurrentNumCellCols = m_NumCols;
		m_CurrentNumCellRows = m_NumRows;
		m_TableIsDirty = true;
	}

	//
	// Allocate a new row, and setup the new row.
	//

	Row** tempRows = m_Rows;
	m_Rows = (Row**)drgMemAllocZero( sizeof( Row* ) * ++m_NumRows );

	if( tempRows )
	{
		for( int currentRow = 0; currentRow < m_NumRows - 1; ++currentRow )
		{
			m_Rows[ currentRow ] = tempRows[ currentRow ];
		}
		SAFE_FREE( tempRows );
	}
	else
	{
		for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
		{
			m_Rows[ currentRow ] = new Row;
		}
	}

	m_Rows[ m_NumRows - 1 ] = new Row;
	Row* row = m_Rows[ m_NumRows - 1 ];
	row->m_MinHeight = 0.0f;
	row->m_MaxHeight = FLT_MAX;
	row->m_FixedHeight = fixedHeight;
	row->m_VariableHeight = variableHeight;
	row->m_Padding = padding;
	row->m_CurrentHeight = row->m_Y = 0;

	//
	// Only resize as necessary
	//

	if( resizeTable )
	{
		ReallocateTable( m_NumRows, m_NumCols );
		Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
	}
}

	// If you pass resizeTable as false, the table will not be expanded, and can't be accessed until ReallocateTable is called.
	// fixedWidth is in pixels, variableWidth is a percentage 0-1.  only one of these arguments is used.
void 
GUI_Table::AddCol( float fixedWidth, float variableWidth, float padding, bool resizeTable )
{
	if( fixedWidth == 0.0f && variableWidth == 0.0f )
	{
		assert( "Fixed width columns must have identical min and max widths" && 0 );
		return;
	}

	//
	// Handle all our dirty logic
	//

	if( m_TableIsDirty == false )
	{
		m_CurrentNumCellCols = m_NumCols;
		m_CurrentNumCellRows = m_NumRows;
		m_TableIsDirty = true;
	}

	//
	// Allocate a new row, and setup the new row.
	//

	Column** tempCols = m_Columns;
	m_Columns = (Column**)drgMemAllocZero( sizeof( Column* ) * ++m_NumCols );

	if( tempCols )
	{
		for( int currentCol = 0; currentCol < m_NumCols - 1; ++currentCol )
		{
			m_Columns[ currentCol ] = tempCols[ currentCol ];
		}
		SAFE_FREE( tempCols );
	}
	else
	{
		for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
		{
			m_Columns[ currentCol ] = new Column;
		}
	}

	m_Columns[ m_NumCols - 1 ] = new Column;
	Column* col = m_Columns[ m_NumCols - 1 ];
	col->m_MinWidth = 0.0f;
	col->m_MaxWidth = FLT_MAX;
	col->m_FixedWidth = fixedWidth;
	col->m_VariableWidth = variableWidth;
	col->m_Padding = padding;
	col->m_CurrentWidth = col->m_X = 0.0f;

	//
	// Now we need to resize the rows and cols for the table.
	//

	if( resizeTable )
	{
		ReallocateTable( m_NumRows, m_NumCols );
		Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
	}
}

void 
GUI_Table::RemoveRow( int rowNum, bool resizeTable )
{
	//
	// First we remove all our cells
	//

	for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
	{
		RemoveCell( rowNum, currentCol, false );

		//
		// We need to move back the other cells to fill in the missing spots.
		// The next time we reallocate the table, these will be cleaned up.
		//

		for( int currentRow = rowNum + 1; currentRow < m_NumRows; ++currentRow )
		{
			m_Cells[ currentRow - 1 ][ currentCol ] = m_Cells[ currentRow ][ currentCol ];
			m_Cells[ currentRow - 1 ][ currentCol ]->m_RowNum = currentRow - 1;
			m_Cells[ currentRow - 1 ][ currentCol ]->m_ColNum = currentCol;
		}
	}

	//
	// Now we need to remove our row, but unfortunately that means I have to 
	// manually resize the table. :/
	//

	Row** tempRows = m_Rows;
	m_Rows = (Row**) drgMemAllocZero( sizeof( Row* ) * (m_NumRows - 1)  );

	int indexOffset = 0;
	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		if( currentRow == rowNum )
		{
			indexOffset++;
			continue;
		}
		else
		{
			m_Rows[ currentRow - indexOffset ] = tempRows[ currentRow ];
		}
	}

	SAFE_FREE( tempRows );

	//
	// Handle the dirty logic, decrement the number of rows,
	// because we are removing data so we don't want to copy over the excess.
	//

	m_CurrentNumCellCols = m_NumCols;
	m_CurrentNumCellRows = --m_NumRows;
	m_TableIsDirty = true;

	//
	// Resize the table as necessary
	//

	if( resizeTable )
	{
		ReallocateTable( m_NumRows, m_NumCols );
		Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
	}
}

void 
GUI_Table::RemoveCol( int colNum, bool resizeTable )
{
	//
	// First we remove all our cells
	//

	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		RemoveCell( currentRow, colNum, false );

		//
		// We need to move back the other cells to fill in the missing spots.
		// The next time we reallocate the table, these will be cleaned up.
		//

		for( int currentCol = colNum + 1; currentCol < m_NumCols; ++currentCol )
		{
			m_Cells[ currentRow ][ currentCol - 1 ] = m_Cells[ currentRow ][ currentCol ];
			m_Cells[ currentRow ][ currentCol - 1 ]->m_RowNum = currentRow;
			m_Cells[ currentRow ][ currentCol - 1 ]->m_ColNum = currentCol - 1;
		}
	}

	//
	// Now we need to remove our row, but unfortunately that means I have to 
	// manually resize the table. :/
	//

	Column** tempCols = m_Columns;
	m_Columns = (Column**)drgMemAllocZero( sizeof( Column ) * ( m_NumCols - 1 ) );

	int indexOffset = 0;
	for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
	{
		if( currentCol == colNum )
		{
			indexOffset++;
			continue;
		}
		else
		{
			m_Columns[ currentCol - indexOffset ] = tempCols[ currentCol ];
		}
	}

	SAFE_FREE( tempCols );

	//
	// Handle the dirty logic, deccrement the number of columns,
	// because we are removing data so we don't want to copy over the excess.
	//

	m_CurrentNumCellCols = --m_NumCols;
	m_CurrentNumCellRows = m_NumRows;
	m_TableIsDirty = true;

	//
	// Resize the table as necessary
	//

	if( resizeTable )
	{
		ReallocateTable( m_NumRows, m_NumCols );
		Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
	}
}

void
GUI_Table::RemoveCell( int rowNum, int colNum, bool resizeTable )
{
	assert( rowNum < m_NumRows && rowNum >= 0 );
	assert( colNum < m_NumCols && colNum >= 0 );

	if( m_TableIsDirty == false )
	{
		m_CurrentNumCellCols = m_NumCols;
		m_CurrentNumCellRows = m_NumRows;
		m_TableIsDirty = true;
	}

	//
	// Kill our cell pointer and it's container.
	//

	Cell* cell = m_Cells[ rowNum ][ colNum ];
	if( cell )
	{
		if( cell->m_Container )
		{
			RemoveChild( cell->m_Container );
			cell->m_Container->Destroy();
			cell->m_Container->DelayedDelete();
		}

		delete cell;

		m_Cells[ rowNum ][ colNum ] = NULL;
	}
	
	//
	// Resize and reallocate as necessary
	//

	if( resizeTable )
	{
		ReallocateTable( m_NumRows, m_NumCols );
		Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
	}
}

void
GUI_Table::RemoveAllCells()
{
	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
		{
			RemoveCell( currentRow, currentCol, false );
		}
	}
}

void
GUI_Table::SetRowHeight( int rowNum, float fixedHeight, float variableHeight )
{
	//
	// Make sure we've got proper information.
	//

	if( variableHeight > 1.0f || variableHeight < 0.0f )
	{
		assert( false && "Tried to provide a variable height out of range." );
		return;
	}

	//
	// Now reset the dimensions. If we have any sort of variable height, ignore the fixed
	// height.
	//
	
	Row* row = GetRow( rowNum );
	if( row )
	{
		if( variableHeight > 0.0f )
		{
			row->m_FixedHeight = 0.0f;
			row->m_VariableHeight = variableHeight;
		}
		else
		{
			row->m_FixedHeight = fixedHeight;
			row->m_VariableHeight = 0.0f;
		}
	}

	Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
}

void
GUI_Table::SetColumnWidth( int colNum, float fixedWidth, float variableWidth )
{
	//
	// Make sure we've got proper information.
	//

	if( variableWidth > 1.0f || variableWidth < 0.0f )
	{
		assert( false && "Tried to provide a variable width out of range." );
		return;
	}

	//
	// Now reset the dimensions. If we have any sort of variable height, ignore the fixed
	// height.
	//
	
	Column* column = GetColumn( colNum );
	if( column )
	{
		if( variableWidth > 0.0f )
		{
			column->m_FixedWidth = 0.0f;
			column->m_VariableWidth = variableWidth;
		}
		else
		{
			column->m_FixedWidth = fixedWidth;
			column->m_VariableWidth = 0.0f;
		}
	}

	Resize( m_View.minv.y, m_View.maxv.y, m_View.minv.x, m_View.maxv.x );
}

GUI_Table::Row*
GUI_Table::GetRow( int rowNum )
{
	if( rowNum >= m_NumRows || rowNum < 0 )
	{
		assert( false && "Tried to access an invalid row" );
		return NULL;
	}

	return m_Rows[ rowNum ];
}

GUI_Table::Column*
GUI_Table::GetColumn( int colNum )
{
	if( colNum >= m_NumCols || colNum < 0 )
	{
		assert( false && "Tried to access an invalid column" );
		return NULL;
	}

	return m_Columns[ colNum ];
}

GUI_Container*
GUI_Table::CreateCell( int rowNum, int colNum, int spansRows, int spansCols )
{
	//
	// Don't allow table access if we haven't been properly resized.
	//

	assert( !m_TableIsDirty && 
		"Make sure your last AddRow/AddCol passed resizeTable = true or call ReallocateTable" );

	assert( rowNum < m_NumRows );
	assert( colNum < m_NumCols );

	//
	// Now we need to create the container for the cell
	//

	Cell* newCell = new Cell;
	GUI_Container* container = GUI_Container::CreateContainer( this, DRG_CONTAINER_ABRITRARY, 0 );

	//
	// Setup the cell class
	//

	newCell->m_Container = container;
	newCell->m_RowNum = rowNum;
	newCell->m_ColNum = colNum;
	newCell->m_SpansRows = spansRows;
	newCell->m_SpansCols = spansCols;
	container->SetPos( m_Columns[ colNum ]->m_X, m_Rows[ rowNum ]->m_Y );

	//
	// Now we need to determine our container's width
	//

	float containerWidth = 0.0f;
	float containerHeight = 0.0f;

	for( int currentCellRow = 0; currentCellRow < spansRows; ++currentCellRow )
	{
		for( int currentCellCol = 0; currentCellCol < spansCols; ++currentCellCol )
		{
			int rowIndex = rowNum + currentCellRow;
			int colIndex = colNum + currentCellCol;

			assert( rowIndex < m_NumRows );
			assert( colIndex < m_NumCols );

			m_Cells[ rowIndex ][ colIndex ] = newCell;

			containerWidth += m_Columns[ currentCellCol ]->m_CurrentWidth;
		}
		containerHeight += m_Rows[ currentCellRow ]->m_CurrentHeight;
	}

	container->SetWidth( containerWidth );
	container->SetHeight( containerHeight );
	
	AddChild( container );

	return container;
}

void 
GUI_Table::AddToCell( GUI_Widget* widget, int rowNum, int colNum )
{
	//
	// Don't allow table access if we haven't been properly resized.
	//

	assert( !m_TableIsDirty && 
		"Make sure your last AddRow/AddCol passed resizeTable = true or call ReallocateTable" );

	if( widget == NULL )
	{
		assert( widget );
		return;
	}

	if( m_Cells[ rowNum ][ colNum ] )
	{
		assert( m_Cells[ rowNum ][ colNum ]->m_Container );
		m_Cells[ rowNum ][ colNum ]->m_Container->AddChild( widget );
		
		//
		// Now resize the container
		//

		drgBBox* bounds = m_Cells[ rowNum ][ colNum ]->m_Container->GetView();
		m_Cells[ rowNum ][ colNum ]->m_Container->Resize( bounds->minv.y, bounds->maxv.y, bounds->minv.x, bounds->maxv.x );
	}
}

GUI_Container* 
GUI_Table::GetCell( int rowNum, int colNum )
{
	//
	// Don't allow table access if we haven't been properly resized.
	//

	assert( !m_TableIsDirty && 
		"Make sure your last AddRow/AddCol passed resizeTable = true or call ReallocateTable" );
	
	assert( rowNum < m_NumRows );
	assert( colNum < m_NumCols );

	if( m_Cells[ rowNum ][ colNum ] )
	{
		// Just return the first child in the container's list. Don't give them the container.
		return m_Cells[ rowNum ][ colNum ]->m_Container;
	}
	return NULL;
}

void 
GUI_Table::Init()
{
	Destroy();	// Clean up any memory that may have been allocated.

	assert( m_Parent != NULL );

	//
	// Make sure we have at least one row and one column.
	//

	m_NumRows = m_NumCols = 0;

	InitCollapseButton();
}

void 
GUI_Table::Draw(GUI_Drawing* draw)
{
	DrawTableOutline( draw );
	GUI_Container::Draw( draw );
}

void
GUI_Table::DrawTableOutline( GUI_Drawing* draw )
{

}

void
GUI_Table::Destroy()
{
	//
	// First we clean up the cells table
	//

	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		for( int currentCol = 0; currentCol < m_NumCols; ++ currentCol )
		{
			if( m_Cells[ currentRow ][ currentCol ] )
			{
				if( m_Cells[ currentRow ][ currentCol ]->m_Container )
				{
					GUI_Container* containerToDel = m_Cells[ currentRow ][ currentCol ]->m_Container;
					Cell* cell = m_Cells[ currentRow ][ currentCol ];

					//
					// Clear all the cell entries that refer to this.
					//

					for( int currentRowSpanned = 0; currentRowSpanned < cell->m_SpansRows; ++currentRowSpanned )
					{
						for( int currentColSpanned = 0; currentColSpanned < cell->m_SpansCols; ++currentColSpanned )
						{
							m_Cells[ currentRow + currentRowSpanned ][ currentCol + currentColSpanned ] = NULL;
						}
					}

					SAFE_DELETE( cell );
					RemoveChild( containerToDel );
					SAFE_DELETE( containerToDel );
				}
				SAFE_DELETE( m_Cells[ currentRow ][ currentCol ] )
			}
		}

		if( m_Cells[ currentRow ] )
		{
			SAFE_FREE( m_Cells[ currentRow ] );
		}
	}
	SAFE_FREE( m_Cells );

	//
	// Next we clean up our rows
	//
	
	SAFE_FREE( m_Rows );

	//
	// Finally we clean up our columns
	//

	SAFE_FREE( m_Columns );
}

void
GUI_Table::UpdateSize()
{
	m_View = m_Sens;
	if( m_TitleBar )
	{
		m_View.minv.y += m_TitleBar->HeightView();
	}

	if( m_NumRows > 0 && m_NumCols > 0 )
	{
		float width = m_View.maxv.x - m_View.minv.x;	
		float height = 0.0f;
		if(HasFlagWidget(DRG_WIDGET_RESIZE_VFILL))
		{
			height = m_View.maxv.y - m_View.minv.y;
		}
		else
		{
			height = GetSumOfRowHeights();
			height = DRG_MAX( height, HeightView());
			m_Sens.maxv.y = m_Sens.minv.y + height;
			m_View = m_Sens;
		}

		ResizeRows( height );
		ResizeColumns( width );
		ResizeCells();
	}
}

int 
GUI_Table::GetNumRows()
{
	return m_NumRows;
}

int 
GUI_Table::GetNumCols()
{
	return m_NumCols;
}

void
GUI_Table::SetRowMinHeight( int rowNum, float minHeight )
{
	assert( rowNum < m_NumRows && rowNum >= 0 );
	Row* row = GetRow( rowNum );

	row->m_MinHeight = minHeight;
	row->m_CurrentHeight = DRG_MAX( row->m_MinHeight, row->m_CurrentHeight );
	UpdateSize();
}

void
GUI_Table::SetRowMaxHeight( int rowNum, float maxHeight )
{
	assert( rowNum < m_NumRows && rowNum >= 0 );
	Row* row = GetRow( rowNum );

	row->m_MaxHeight = maxHeight;
	row->m_CurrentHeight = DRG_MIN( row->m_MaxHeight, row->m_CurrentHeight );
	UpdateSize();
}

void
GUI_Table::SetColumnMinWidth( int colNum, float minWidth )
{
	Column* col = GetColumn( colNum );

	col->m_MinWidth = minWidth;
	col->m_CurrentWidth = DRG_MAX( col->m_MinWidth, col->m_CurrentWidth );
	UpdateSize();
}

void
GUI_Table::SetColumnMaxWidth( int colNum, float maxWidth )
{
	Column* col = GetColumn( colNum );

	col->m_MinWidth = maxWidth;
	col->m_CurrentWidth = DRG_MIN( col->m_MaxWidth, col->m_CurrentWidth );
	UpdateSize();
}

float 
GUI_Table::GetSumOfRowHeights()
{
	float sum = 0.0f;
	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		if( m_Rows[ currentRow ] == NULL )
			continue;
		
		Row* row = m_Rows[ currentRow ];
		sum += row->m_CurrentHeight;
		sum += row->m_Padding;
	}
	return sum;
}

void GUI_Table::SetupPropertyTable(int nStartRows)
{
    this->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	for(int i = 0; i < nStartRows; i++)
	{
		this->AddRow(DEFAULT_PROPERTY_ROW_HEIGHT, DEFAULT_PROPERTY_ROW_VAR_HEIGHT, DEFAULT_PROPERTY_ROW_PADDING, false);
	}

    this->AddCol(DEFAULT_PROPERTY_COL_WIDTH, DEFAULT_PROPERTY_COL_VAR_WIDTH[0], DEFAULT_PROPERTY_COL_PADDING, false);
    this->AddCol(DEFAULT_PROPERTY_COL_WIDTH, DEFAULT_PROPERTY_COL_VAR_WIDTH[0], DEFAULT_PROPERTY_COL_PADDING, false);
    this->AddCol(DEFAULT_PROPERTY_COL_WIDTH, DEFAULT_PROPERTY_COL_VAR_WIDTH[1], DEFAULT_PROPERTY_COL_PADDING, true);
}

void
GUI_Table::ReallocateTable( int numRows, int numCols )
{
	Cell*** temp = m_Cells;
	m_NumRows = numRows;
	m_NumCols = numCols;

	//
	// Make sure we flag ourselves as clean
	//

	m_TableIsDirty = false;

	//
	// Allocate a new cell table
	//

	if(m_NumRows <= 0 || m_NumCols <= 0)
		return;

	m_Cells = (Cell***)drgMemAllocZero( sizeof( Cell** ) * m_NumRows );
	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		m_Cells[currentRow] = (Cell**)drgMemAllocZero(sizeof(Cell*)* m_NumCols);
	}

	//
	// Now copy over all the other widgets
	//

	if( temp )
	{
		for( int currentRow = 0; currentRow < m_CurrentNumCellRows; ++currentRow )
		{
			for( int currentCol = 0; currentCol < m_CurrentNumCellCols; ++currentCol )
			{
				m_Cells[ currentRow ][ currentCol ] = temp[ currentRow ][ currentCol ];
			}
		
			SAFE_FREE( temp[ currentRow ] );
		}
		SAFE_FREE( temp );
	}

	m_CurrentNumCellRows = m_NumRows;
	m_CurrentNumCellCols = m_NumCols;
} 


void
GUI_Table::ResizeRows( float height )
{
	//
	// First we are going to resize the fixed sized elements, and substract 
	// their width from our total width. While we're at it, we're going to get the total
	// percent requested from variable sized rows
	//

	if( height == 0.0f )
		height = 1.0f;

	float heightLeft = height;
	float pixelsRequested = 0.0f;

	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		if( m_Rows[ currentRow ]->m_VariableHeight == 0.0f )
		{
			// In a fixed size row, their min and max are the same.
			m_Rows[ currentRow ]->m_CurrentHeight = m_Rows[ currentRow ]->m_FixedHeight;	
			heightLeft -= m_Rows[ currentRow ]->m_FixedHeight + m_Rows[ currentRow ]->m_Padding;
		}
		else
		{
			pixelsRequested += m_Rows[ currentRow ]->m_VariableHeight * height + m_Rows[ currentRow ]->m_Padding;
		}
	}
	
	//
	// Now we need to equally distribute positioning for elements based on width left and 
	// total percent requested. We'll determine the position as we walk the list as well.
	//

	float normalizationFactor = height / pixelsRequested;
	float heightSoFar = 0.0f;

	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		bool renormalize = true;
		if( m_Rows[ currentRow ]->m_VariableHeight > 0.0f )
		{
			float newHeight = heightLeft * m_Rows[ currentRow ]->m_VariableHeight * normalizationFactor;
			
			// Make sure we still clamp our dimensions.
			if( newHeight < m_Rows[ currentRow ]->m_MinHeight )
			{
				newHeight = m_Rows[ currentRow ]->m_MinHeight;
				renormalize = true;
			}
			else if( newHeight > m_Rows[ currentRow ]->m_MaxHeight )
			{
				newHeight = m_Rows[ currentRow ]->m_MaxHeight;
				renormalize = true;
			}

			m_Rows[ currentRow ]->m_CurrentHeight = newHeight;
			heightLeft -= m_Rows[ currentRow ]->m_CurrentHeight;
		}

		//
		// Determine our y position as well.
		//

		m_Rows[ currentRow ]->m_Y = heightSoFar;
		heightSoFar += m_Rows[ currentRow ]->m_CurrentHeight + m_Rows[ currentRow ]->m_Padding;

		if( renormalize )
		{
			//
			// When we need to renormalize, we need to go through the list again
			// and determine what percentages are still being requested and make them 
			// take up the rest of the space remaining.
			//

			float spaceRequested = 0.0f;
			for( int currentRowRemaining = currentRow + 1; currentRowRemaining < m_NumRows; ++currentRowRemaining )
			{
				if( m_Rows[ currentRowRemaining ]->m_VariableHeight > 0.0f )
				{
					spaceRequested += m_Rows[ currentRowRemaining ]->m_VariableHeight * heightLeft + m_Rows[ currentRow ]->m_Padding;
				}
			}

			normalizationFactor = heightLeft / spaceRequested;
		}
	}
}

void
GUI_Table::ResizeColumns( float width )
{
	//
	// First we are going to resize the fixed sized elements, and substract 
	// their width from our total width. While we're at it, we're going to get the total
	// percent requested from variable sized rows
	//

	float widthLeft = width;
	float pixelsRequested = 0.0f;
	int numVariableCols = 0;

	for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
	{
		if( m_Columns[ currentCol ]->m_VariableWidth == 0.0f )
		{
			// In a fixed size row, their min and max are the same.
			m_Columns[ currentCol ]->m_CurrentWidth = m_Columns[ currentCol ]->m_FixedWidth;	
			widthLeft -= m_Columns[ currentCol ]->m_FixedWidth + m_Columns[ currentCol ]->m_Padding;
		}
		else
		{
			pixelsRequested += m_Columns[ currentCol ]->m_VariableWidth * width + m_Columns[ currentCol ]->m_Padding;
			numVariableCols++;
		}
	}
	
	//
	// Now we need to equally distribute positioning for elements based on width left and 
	// total percent requested. We'll determine the position as we walk the list as well.
	//

	float normalizationFactor = width / pixelsRequested;
	float widthSoFar = 0.0f;

	for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
	{
		bool renormalize = true;
		if( m_Columns[ currentCol ]->m_VariableWidth > 0.0f )
		{
			float newWidth = widthLeft * m_Columns[ currentCol ]->m_VariableWidth * normalizationFactor;
			
			// Make sure we still clamp our dimensions.
			if( newWidth < m_Columns[ currentCol ]->m_MinWidth )
			{
				newWidth = m_Columns[ currentCol ]->m_MinWidth;
				renormalize = true;
			}
			else if( newWidth > m_Columns[ currentCol ]->m_MaxWidth )
			{
				newWidth = m_Columns[ currentCol ]->m_MaxWidth;
				renormalize = true;
			}

			m_Columns[ currentCol ]->m_CurrentWidth = newWidth;
			widthLeft -= m_Columns[ currentCol ]->m_CurrentWidth + m_Columns[ currentCol ]->m_Padding;
		}

		//
		// Determine our y position as well.
		//

		m_Columns[ currentCol ]->m_X = widthSoFar;
		widthSoFar += m_Columns[ currentCol ]->m_CurrentWidth + m_Columns[ currentCol ]->m_Padding;

		if( renormalize )
		{
			//
			// When we need to renormalize, we need to go through the list again
			// and determine what percentages are still being requested and make them 
			// take up the rest of the space remaining.
			//

			float spaceRequested = 0.0f;
			for( int currentColRemaining = currentCol + 1; currentColRemaining < m_NumCols; ++currentColRemaining )
			{
				if( m_Columns[ currentColRemaining ]->m_VariableWidth > 0.0f )
				{
					spaceRequested += m_Columns[ currentColRemaining ]->m_VariableWidth * widthLeft + m_Columns[ currentColRemaining ]->m_Padding;
				}
			}

			normalizationFactor = widthLeft / spaceRequested;
		}
	}
}

void GUI_Table::ResizeCells()
{
	for( int currentRow = 0; currentRow < m_NumRows; ++currentRow )
	{
		if( m_Cells[ currentRow ] == NULL )
			continue;

		for( int currentCol = 0; currentCol < m_NumCols; ++currentCol )
		{
			Cell* cell = m_Cells[ currentRow ][ currentCol ];
		
			//
			// Make sure this is the first cell this cell spans
			//

			if( cell &&
				cell->m_Container &&
				cell->m_RowNum == currentRow &&
				cell->m_ColNum == currentCol )
			{
				cell->m_Container->SetPos( m_Columns[ currentCol ]->m_X, m_Rows[ currentRow ]->m_Y );

				//
				// Now we need to determine this cell's width and height
				//

				float newWidth = GetCellWidth( cell );
				float newHeight = GetCellHeight( cell );

				cell->m_Container->SetWidth( newWidth );
				cell->m_Container->SetHeight( newHeight );

				cell->m_Container->UpdateSize();
			}
		}
	}
}

float
GUI_Table::GetCellWidth( GUI_Table::Cell* cell )
{
	if( cell == NULL )
		return 0.0f;

	float width = 0.0f;
	for( int currentColSpanned = 0; currentColSpanned < cell->m_SpansCols; ++currentColSpanned )
	{
		assert( cell->m_ColNum + currentColSpanned < m_NumCols );
		width += m_Columns[ currentColSpanned + cell->m_ColNum ]->m_CurrentWidth + m_Columns[ currentColSpanned ]->m_Padding;
	}

	width -= m_Columns[ cell->m_ColNum + cell->m_SpansCols - 1 ]->m_Padding;
	return width;
}

float
GUI_Table::GetCellHeight( GUI_Table::Cell* cell )
{
	if( cell == NULL )
		return 0.0f;

	float height = 0.0f;
	for( int currentRowSpanned = 0; currentRowSpanned < cell->m_SpansRows; ++currentRowSpanned )
	{
		assert( cell->m_RowNum + currentRowSpanned < m_NumRows );
		height += m_Rows[ cell->m_RowNum + currentRowSpanned ]->m_CurrentHeight + m_Rows[ currentRowSpanned ]->m_Padding;
	}

	height -= m_Rows[ cell->m_RowNum + cell->m_SpansRows - 1 ]->m_Padding;
	return height;
}



