
#include "render/gui/gui_manager.h"
#include "render/gui/gui.h"
#include "render/gui/gui_draw.h"
#include "render/render_cache.h"
#include "system/engine.h"
#include "core/input.h"
#include "util/math_def.h"
#include "render/gui/style.h"

#define GUI_HOVER_TEXT_DELAY (0.5f)
#define DOUBLE_CLICK_TIME (0.25f)
#define NUM_DRAW_NODES (1)

//===================
// Public Methods
//===================

// Constructor
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		It constructs. Don't put anything that can fail in here.
GUI_Manager::GUI_Manager()
{
	m_SelectedWidget = NULL;
	m_FocusedWidget = NULL;
	m_HoverWidget = NULL;
	m_DragWidget = NULL;
	m_DragWindow = NULL;
	m_CurrentModal = NULL;
	m_Windows = NULL;
	m_DragWidgetPreviousParent = NULL;
	m_ReceiverWidget = NULL;
	m_NumWindows = 0;

	m_HasBeenSetAsScriptObject = false;
	m_ClickStarted = false;
	m_ClickFinished = false;
	m_DoubleClickMouseButton = 0;
	m_TimeSinceHoverWidgetMouseEnter = 0.0f;
	m_TimeSinceLastClick = 0.0f;
	m_DoubleClickTimer = DOUBLE_CLICK_TIME;
	m_DoubleClickWinID = 0;
	m_TopLayer = 0;
	m_TopWindowLayer = 0;

	m_DragAnchorOffset.Set(0.0f, 0.0f);
}

//
// Destructor
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will release all resources used by this object.
//

GUI_Manager::~GUI_Manager()
{
	Destroy();
}

//
// Init
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will initialize the object, making it ready for use.
//

void GUI_Manager::Init()
{
	AddEvents();
}

//
// Destroy
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will release all resources used by this object.
//

void GUI_Manager::Destroy()
{
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		m_Windows[currentWindow]->SetManager(NULL);
	}

	SAFE_FREE_ARRAY_OF_NEWS(m_Windows, m_NumWindows);

	m_SelectedWidget = NULL;
	m_FocusedWidget = NULL;
	m_DragWidget = NULL;
	m_CurrentModal = NULL;

	RemoveEvents();
}

//
// CreateNativeWindow
// Inputs:
//		string16* title: The title of the new window
//		int width: The width of the window
//		int height: The height of the window
//		drgVec2* pos: The position of the resultant window.
//		unsigned int windowFlags: The flags for this window.
//		GUI_Style* style: The style for this window
// Outputs:
//		GUI_Window*: The window created
// Description:
//		Will create and initialize the window used for drag objects.
//

GUI_Window *GUI_Manager::CreateNativeWindow(string16 *title, int width, int height, drgVec2 *pos, unsigned int windowFlags, GUI_Style *style)
{
	GUI_Window *window = GUI_Window::CreateNativeWindow(title, width, height, pos, windowFlags, style);
	return window;
}

// DestroyNativeWindow
// Inputs:
//		GUI_Window* window: The window to destroy
// Outputs:
//		None
// Description:
//		Will destroy a window that was used for a drag object
void GUI_Manager::DestroyNativeWindow(GUI_Window *window)
{
	RemoveWindow(window);
	if (window)
	{
		window->DelayedDelete();
	}
}

//
// AddWindow
// Inputs:
//		GUI_Window* window: A pointer to a window to add for management.
// Outputs:
//		None
// Description:
//		Will add a new window to the list of windows to be managed.
//
void GUI_Manager::AddWindow(GUI_Window *window)
{
	if (HasWindow(window))
	{
		return;
	}

	// Make sure we have a window here. Should it also have to be a native window...?
	if (window == NULL)
	{
		drgPrintError("Tried to add a NULL window to be managed");
	}

	// We need to allocate a new list for our new friend!
	GUI_Window **temp = m_Windows;
	m_Windows = (GUI_Window **)drgMemAlloc(sizeof(GUI_Window *) * ++m_NumWindows);

	for (int currentWindow = 0; currentWindow < m_NumWindows - 1; ++currentWindow)
	{
		m_Windows[currentWindow] = temp[currentWindow];
	}

	SAFE_FREE(temp);

	// Add the window
	m_Windows[m_NumWindows - 1] = window;

	// Now we set the manager for the window
	window->SetManager(this);
}

//
// HasWindow
// Inputs:
//		GUI_Window* window: The window to search for
// Outputs:
//		None
// Description:
//		Will return whether or not the provided window is currently being managed
//

bool GUI_Manager::HasWindow(GUI_Window *window)
{
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow] == window)
			return true;
	}

	return false;
}

//
// RemoveWindow
// Inputs:
//		GUI_Window* window: The pointer to the window to remove.
// Outputs:
//		None
// Description:
//		Will remove a window from being managed, if it's been found.
//

void GUI_Manager::RemoveWindow(GUI_Window *window)
{
	if (HasWindow(window) == false)
	{
		return;
	}

	//
	// We need to allocate a new list to get rid of an old friend
	//

	int indexOffset = 0;
	GUI_Window **temp = m_Windows;

	m_Windows = (GUI_Window **)drgMemAlloc(sizeof(GUI_Window *) * --m_NumWindows);

	for (int currentWindow = 0; currentWindow < m_NumWindows + 1; ++currentWindow)
	{
		if (temp[currentWindow] == window)
		{
			indexOffset++;
			window->SetManager(NULL);
		}
		else
		{
			m_Windows[currentWindow - indexOffset] = temp[currentWindow];
		}
	}

	SAFE_FREE(temp);
}

//
// GetBaseWindow
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will return the base window that this manager is managing.
//

GUI_Window *GUI_Manager::GetBaseWindow()
{
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow] && m_Windows[currentWindow]->HasFlagWin(DRG_WINDOW_BASE_WINDOW))
		{
			return m_Windows[currentWindow];
		}
	}
	return NULL;
}

// Draw
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will draw all the windows, and all special rendering like dragging feedback.
void GUI_Manager::Draw()
{
	// Draw all the windows.
	DRG_PROFILE_FUNK(GUI_Manager_Draw, 0xFFFFFF00);
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->Draw();
		}
	}
}

// Update
// Inputs:
//		float deltaTime: The time since the last frame.
// Outputs:
//		None
// Description:
//		Will update the manager for this frame.
void GUI_Manager::Update(float deltaTime)
{
	DRG_PROFILE_FUNK(GUI_Manager_Update, 0xFFFFFF00);
	UpdateHoverTimer(deltaTime);
	UpdateDoubleClickTimer(deltaTime);
}

// CallKeyClick
// Inputs:
//		unsigned int winid: The GUID for the object to call.
//		int key: The key clicked
//		int charval: The char that was clicked
//		bool down: Is the key currently down?
// Outputs:
//		None
// Description:
//		This function will be called on any key clicks.
void GUI_Manager::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	// Give the input to our focused widget or modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		m_CurrentModal->CallKeyClick(winid, key, charval, down);
		return;
	}

	if (m_FocusedWidget != NULL)
	{
		m_FocusedWidget->CallKeyClick(winid, key, charval, down);
	}

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallKeyClick(winid, key, charval, down);
		}
	}
}

// CallMouseDown
// Inputs:
//		unsigned int winid: The GUID for the window that called this
//		int button: The mouse button that is down
// Outputs:
//		bool: Was the mouse down handled?
// Description:
//		This function will be called whenever the mouse is down
bool GUI_Manager::CallMouseDown(unsigned int winid, int button)
{
	// Don't let input go past our modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		return m_CurrentModal->CallMouseDown(winid, button);
	}

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallMouseDown(winid, button);
		}
	}

	// Now let's see if we've started a double click
	if (m_ClickStarted)
	{
		CheckDoubleClick(winid, button);
	}
	else
	{
		StartDoubleClick(winid, button);
	}

	return false;
}

// CallMouseUp
// Inputs:
//		unsigned int winid: The GUID for the window that called this
//		int button: The mouse button that was released
// Outputs:
//		bool: Was the mouse release handled?
// Description:
//		This function will be called whenever the mouse is release.
bool GUI_Manager::CallMouseClick(unsigned int winid, int button)
{
	// Drop our drag widget
	if (m_DragWidget != NULL)
	{
		OnDragDone();
	}

	// Don't let input go past our modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		return m_CurrentModal->CallMouseClick(winid, button);
	}

	// Unfocus our widget
	if (NULL != m_FocusedWidget)
	{
		m_FocusedWidget->SetUnFocused();
	}

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallMouseClick(winid, button);
		}
	}

	if (m_ClickStarted)
	{
		m_ClickFinished = true;
	}

	return false;
}

// CallMouseUp
// Inputs:
//		unsigned int winid: The window that this event was called for.
//		int button: The button released
// Outputs:
//		None
// Description:
//		Will allow the window to handle a mouse button being released.
void GUI_Manager::CallMouseUp(unsigned int winid, int button)
{
	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallMouseUp(winid, button);
		}
	}
}

// CallMouseDoubleClick
// Inputs:
//		unsigned int winid: The window that this event was called for.
//		int button: The button pressed
// Outputs:
//		bool: Was the click handled?
// Description:
//		Will allow the window to handle a double click
bool GUI_Manager::CallMouseDoubleClick(unsigned int winid, int button)
{
	// Don't let input go past our modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
		return m_CurrentModal->CallMouseDoubleClick(winid, button);

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallMouseDoubleClick(winid, button);
		}
	}

	return true;
}

// CallMouseWheel
// Inputs:
//		unsigned int winid: the GUID for the window that called this
//		float pos: The position of the wheel
//		float rel: The relative movement:
// Outputs:
//		None:
// Description:
//		Will provide information when a scroll wheel has been changed.
void GUI_Manager::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	// Don't let input go past our modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		m_CurrentModal->CallMouseWheel(winid, pos, rel);
		return;
	}
	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallMouseWheel(winid, pos, rel);
		}
	}
}

// CallMouseMove
// Inputs:
//		unsigned int winid: The GUID for the window that called this
//		drgVec2* pos: The position of the cursor in screen space
//		drgVec2* rel: The distance moved relative to it's previous position.
// Outputs:
//		None
// Description:
//		This function will be called whenever the mouse moved.
void GUI_Manager::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	m_HoverWidget = NULL;

	// Let our modal handle any mouse moves.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		m_CurrentModal->CallMouseMove(winid, pos, rel);
		return;
	}

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallMouseMove(winid, pos, rel);
		}
	}
}

// CallMouseDrag
// Inputs:
//		unsigned int winid: The GUID for the window that called this
//		drgVec2* pos: The position of the cursor in screen space
//		drgVec2* rel: The distance moved relative to it's previous position.
// Outputs:
//		None
// Description:
//		This function will be called whenever the mouse moved.
void GUI_Manager::CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
	// We need to make sure that we reset the receiver widget each new drag.
	m_ReceiverWidget = NULL;

	// If we have a drag widget, move the drag window.
	if (m_DragWindow != NULL && m_DragWindow->GetGUID() == winid && HasDragWidget())
	{
		drgVec2 *globalPos = drgInputMouse::GetGlobalPos();
		m_DragWindow->SetPos(globalPos->x - m_DragAnchorOffset.x, globalPos->y + m_DragAnchorOffset.y);
	}

	// Let our modal handle any mouse drags.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		m_CurrentModal->CallMouseDrag(winid, pos, rel);
		return;
	}

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			// All windows may be interested in mouse drag.
			m_Windows[currentWindow]->CallMouseDrag(m_Windows[currentWindow]->GetGUID(), pos, rel);
		}
	}
}

// CallPointerDown
// Inputs:
//		unsigned int winid: The id for the window to handle the event
//		int pointerIndex: The pointer that was pressed down
// Outputs:
//		bool: Was the event handled?
// Description: Will allow a window to handle a pointer down event
bool GUI_Manager::CallPointerDown(unsigned int winid, int pointerIndex)
{
	// Don't let input go past our modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		return m_CurrentModal->CallPointerDown(winid, pointerIndex);
	}
	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallPointerDown(winid, pointerIndex);
		}
	}

	return false;
}

// CallPointerClick
// Inputs:
//		unsigned int winid: The id for the window to handle the event
//		int pointerIndex: The pointer that was released
// Outputs:
//		bool: Was the event handled?
// Description: Will allow a window to handle a pointer click event
bool GUI_Manager::CallPointerClick(unsigned int winid, int pointerIndex)
{
	// Drop our drag widget
	if (m_DragWidget != NULL)
	{
		OnDragDone();
	}
	// Don't let input go past our modal dialog.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		return m_CurrentModal->CallPointerClick(winid, pointerIndex);
	}
	// Unfocus our widget
	if (NULL != m_FocusedWidget)
	{
		m_FocusedWidget->SetUnFocused();
	}
	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			if (m_Windows[currentWindow]->CallPointerClick(winid, pointerIndex))
				return true;
		}
	}
	return false;
}

// CallPointerUp
// Inputs:
//		unsigned int winid: The id for the window to handle the event
//		int pointerIndex: The pointer that was released
// Outputs:
//		None
// Description: Will allow a window to handle a pointer up event
void GUI_Manager::CallPointerUp(unsigned int winid, int pointerIndex)
{
	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallPointerUp(winid, pointerIndex);
		}
	}
}

// CallPointerMove
// Inputs:
//		unsigned int winid: The GUID for the window that called this
//		drgVec2* pos: The position of the pointer in screen space
//		drgVec2* rel: The distance moved relative to it's previous position.
//		int pointerIndex
// Outputs:
//		None
// Description:
//		This function will be called whenever the pointer moved.
void GUI_Manager::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel)
{
	m_HoverWidget = NULL;

	// Let our modal handle any mouse moves.
	if (m_CurrentModal != NULL && m_CurrentModal->GetGUID() == winid)
	{
		m_CurrentModal->CallPointerMove(winid, pointerIndex, pos, rel);
		return;
	}

	// Now give our child windows a chance to handle it
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow])
		{
			m_Windows[currentWindow]->CallPointerMove(winid, pointerIndex, pos, rel);
		}
	}
}

// CallResize
// Inputs:
//		unsigned int guid: The guid for the window to handle the callback
//		drgVec2Int* size: The new size for the window
// Outputs:
//		None
// Description:
//		An event that is called when a window is resized.
void GUI_Manager::CallResize(unsigned int winid, drgVec2 *size)
{
	for (int currentWindow = 0; currentWindow < m_NumWindows; ++currentWindow)
	{
		if (m_Windows[currentWindow] && m_Windows[currentWindow]->GetGUID() == winid)
		{
			m_Windows[currentWindow]->CallResize(winid, size);
		}
	}
}

// SetSelectedWidget
// Inputs:
//		GUI_Widget* widget: The widget to select
// Outputs:
//		None:
// Description:
//		Will set the provided widget as the selected widget
void GUI_Manager::SetSelectedWidget(GUI_Widget *widget)
{
	m_SelectedWidget = widget;
}

// SetFocusedWidget
// Inputs:
//		GUI_Widget* widget: The widget to focus
// Outputs:
//		None
// Description:
//		Will set the provided widget as the focused widget.
void GUI_Manager::SetFocusedWidget(GUI_Widget *widget)
{
	m_FocusedWidget = widget;
}

void GUI_Manager::SetHoverWidget(GUI_Widget *widget)
{
	if (m_HoverWidget != NULL && widget != NULL && widget->GetLayer() < m_HoverWidget->GetLayer())
		return;

	// int oldL = m_HoverWidget == NULL ? -1 : m_HoverWidget->GetLayer();
	// int newL = widget == NULL ? -1 : widget->GetLayer();
	// drgPrintOut("SetHoverWidget. old layer: %i, new layer: %i \n", oldL, newL);

	if (m_HoverWidget == NULL || widget == NULL)
		m_TimeSinceHoverWidgetMouseEnter = 0.0f;

	m_HoverWidget = widget;
}

// SetDragWidget
// Inputs:
//		GUI_Widget* widget: The widget to drag
// Outputs:
//		None
// Description:
//		Will set the provided widget as the drag widget
void GUI_Manager::SetDragWidget(GUI_Widget *widget)
{
	m_DragWidget = widget;
	OnDragStart(widget);
}

// SetReceieverContainer
// Inputs:
//		GUI_Container* container: The container to be the receiever
// Outputs:
//		None
// Description:
//		Will set the current container that could receive a widget
void GUI_Manager::SetReceiverWidget(GUI_Widget *widget)
{
	if (widget == NULL)
	{
		m_ReceiverWidget = NULL;
		return;
	}

	if (m_DragWidget && widget->IsChildOf(m_DragWidget) == false)
	{
		m_ReceiverWidget = widget;
	}
}

// SetModalWindow
// Inputs:
//		GUI_Window* window: The window to set as the modal window
// Outputs:
//		None
// Description:
//		Will set the provided window as the current modal dialog.
void GUI_Manager::SetModalWindow(GUI_Window *window)
{
	if (m_CurrentModal != NULL)
		m_CurrentModal->SetFlagsWin(m_CurrentModal->GetFlagsWin() & ~DRG_WINDOW_MODAL);
	m_CurrentModal = window;
	if (window != NULL)
		window->SetFlagsWin(m_CurrentModal->GetFlagsWin() | DRG_WINDOW_MODAL);
}

//
// GetDragWidget
// Inputs:
//		None
// Outputs:
//		GUI_Widget*: The current drag widget
// Description:
//		Will return the current widget being dragged.
//

GUI_Widget *GUI_Manager::GetDragWidget()
{
	return m_DragWidget;
}

//
// GetSelectedWidget
// Inputs:
//		None
// Outputs:
//		GUI_Widget*: The current selected widget
// Description:
//		Will return the currently selected widget
//

GUI_Widget *GUI_Manager::GetSelectedWidget()
{
	return m_SelectedWidget;
}

GUI_Widget *GUI_Manager::GetHoverWidget()
{
	return m_HoverWidget;
}

// GetFocusedWidget
// Inputs:
//		None
// Outputs:
//		GUI_Widget*: The currently focused widget
// Description:
//		Will return the currently focused widget
GUI_Widget *GUI_Manager::GetFocusedWidget()
{
	return m_FocusedWidget;
}

// GetReceiverWidget
// Inputs:
//		None
// Outputs:
//		GUI_Container*: The receiver container
// Description:
//		Will return the current receiver container
GUI_Widget *GUI_Manager::GetReceiverWidget()
{
	return m_ReceiverWidget;
}

// GetModalWindow
// Inputs:
//		None
// Outputs:
//		GUI_Window*: The current modal window
// Description:
//		Will return the current modal window
GUI_Window *GUI_Manager::GetModalWindow()
{
	return m_CurrentModal;
}

// GetDragWindow
// Inputs:
//		None
// Outputs:
//		GUI_Window*: The current drag window
// Description:
//		Will return the current drag window
GUI_Window *GUI_Manager::GetDragWindow()
{
	return m_DragWindow;
}

//=====================
// Protected Methods
//=====================

//
// AddEvents
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will register us for all pertinent events.
void GUI_Manager::AddEvents()
{
	//
	// Add our events.
	//

	drgEvent::Add(DRG_EVENT_MOUSE_DOWN, this);
	drgEvent::Add(DRG_EVENT_MOUSE_CLICK, this);
	drgEvent::Add(DRG_EVENT_MOUSE_UP, this);
	drgEvent::Add(DRG_EVENT_KEY_DOWN, this);
	drgEvent::Add(DRG_EVENT_KEY_UP, this);
	drgEvent::Add(DRG_EVENT_MOUSE_MOVE, this);
	drgEvent::Add(DRG_EVENT_MOUSE_DRAG, this);
	drgEvent::Add(DRG_EVENT_POINTER_DOWN, this);
	drgEvent::Add(DRG_EVENT_POINTER_UP, this);
	drgEvent::Add(DRG_EVENT_POINTER_CLICK, this);
	drgEvent::Add(DRG_EVENT_POINTER_MOVE, this);
}

//
// RemoveEvents
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will un-register us for all pertinent events.
//

void GUI_Manager::RemoveEvents()
{
	//
	// Remove us from the list of events.
	//

	drgEvent::Remove(DRG_EVENT_MOUSE_DOWN, this);
	drgEvent::Remove(DRG_EVENT_MOUSE_CLICK, this);
	drgEvent::Remove(DRG_EVENT_MOUSE_UP, this);
	drgEvent::Remove(DRG_EVENT_KEY_DOWN, this);
	drgEvent::Remove(DRG_EVENT_KEY_UP, this);
	drgEvent::Remove(DRG_EVENT_MOUSE_MOVE, this);
	drgEvent::Remove(DRG_EVENT_MOUSE_DRAG, this);
	drgEvent::Remove(DRG_EVENT_POINTER_DOWN, this);
	drgEvent::Remove(DRG_EVENT_POINTER_UP, this);
	drgEvent::Remove(DRG_EVENT_POINTER_CLICK, this);
	drgEvent::Remove(DRG_EVENT_POINTER_MOVE, this);
}

//
// OnDragStart
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will handle when a new drag widget has been set
//

void GUI_Manager::OnDragStart(GUI_Widget *widget)
{
	if (widget == NULL)
		return;

	m_DragWidget = widget;

	// Is the new drag widget a docker?
	if (m_DragWidget->HasFlagWidget(DRG_WIDGET_DOCKER))
	{
		// Determine the location for the window when dragging.
		m_DragAnchorOffset.Set(0.0f, 0.0f);
		m_DragWidget->GetNativePosView(&m_DragAnchorOffset);
		drgVec2 *globalPos = drgInputMouse::GetGlobalPos();

		m_DragAnchorOffset.x = globalPos->x - m_DragAnchorOffset.x;
		m_DragAnchorOffset.y = globalPos->y - m_DragAnchorOffset.y;

		// If we are being dragged from the base window, then we need to create a new window.
		GUI_Window *window = m_DragWidget->GetParentWindow();
		m_DragWidgetPreviousParent = window; // Remember to store the previous parent.

		if (window && window->HasFlagWin(DRG_WINDOW_BASE_WINDOW))
		{
			m_DragWindow = CreateNewDragWindow(m_DragWidget, globalPos);
		}
		else
		{ // We are just dragging an already made dialog window.
			m_DragWindow = window;
		}

		m_DragWindow->SetPos(globalPos->x - m_DragAnchorOffset.x, globalPos->y + m_DragAnchorOffset.y);
		m_DragWidget->Show();
	}
}

// OnDragDone
// Inputs:
//		None
// Outputs:
//		None
// Description:
//		Will handle when a drag widget has been dropped.
void GUI_Manager::OnDragDone()
{
	assert(m_DragWidget); // How did we finish dragging with no drag widget?

	if (m_DragWindow)
	{
		drgVec2 globalPos(0.0f, 0.0f);
		m_DragWindow->GetNativePosView(&globalPos);

		if (m_ReceiverWidget != NULL)
		{
			// Drop into the receiver widget
			m_DragWindow->RemoveChild(m_DragWidget);
			m_ReceiverWidget->ReceiveWidget(m_DragWidget);

			// Make sure the entire window resizes, since this may change mins/maxes for
			// just about everyone.
			GUI_Window *window = m_ReceiverWidget->GetParentWindow();
			if (window)
			{
				window->UpdateSize();
			}

			// Clean up after ourselves
			DestroyNativeWindow(m_DragWindow);
			m_DragWindow = NULL;
			m_ReceiverWidget = NULL;
		}
		else
		{
			// If the drag widget is already in a separate window, just move the window.
			if (m_DragWidgetPreviousParent && m_DragWidgetPreviousParent->HasFlagWin(DRG_WINDOW_BASE_WINDOW))
			{
				GUI_Style *style = NULL;
				if (m_DragWindow)
					style = m_DragWindow->GetStyle();

				// If the widget is not over a receiver widget and its parent window is not a dialog window,
				// then we need to create a new window for it and store it there.
				GUI_Window *window = CreateNativeWindow(NULL,
														(int)m_DragWidget->WidthBox(),
														(int)m_DragWidget->HeightBox(),
														&globalPos,
														DRG_WINDOW_DIALOG | DRG_WINDOW_NOTITLE,
														style);

				m_DragWindow->RemoveChild(m_DragWidget);

				m_DragWidget->SetPos(0.0f, 0.0f);
				m_DragWidget->SetWidth(m_DragWidget->WidthBox());
				m_DragWidget->SetHeight(m_DragWidget->HeightBox());
				m_DragWidget->UpdateSize();

				window->SetMinWidth(m_DragWidget->GetMinWidth());
				window->SetMinHeight(m_DragWidget->GetMinHeight());

				window->AddChild(m_DragWidget);

				DestroyNativeWindow(m_DragWindow);
				m_DragWindow = NULL;
			}
			else if (m_DragWidgetPreviousParent && m_DragWidgetPreviousParent->HasFlagWin(DRG_WINDOW_DIALOG))
			{
				m_DragWindow = NULL;
			}
		}

		m_DragWidget->DragDone();
		m_DragWidget = NULL;
		m_DragWidgetPreviousParent = NULL;
	}
	else if (m_ReceiverWidget)
	{
		m_ReceiverWidget->ReceiveWidget(m_DragWidget);

		// Make sure the entire window resizes, since this may change mins/maxes for
		// just about everyone.
		GUI_Window *window = m_ReceiverWidget->GetParentWindow();
		if (window)
		{
			window->UpdateSize();
		}

		m_DragWidget->DragDone();
		m_DragWidget = NULL;
		m_DragWidgetPreviousParent = NULL;
		m_ReceiverWidget = NULL;
	}
}

// CreateNewDragWindow
// Inputs:
//		GUI_Widget* dragWidget: The new dragWidget
//		drgVec2* globalPos: The global position to position the window at.
// Outputs:
//		GUI_Window*: The new drag window created
// Description:
//		Will create a new drag window, and parent the drag widget appropiately
GUI_Window *GUI_Manager::CreateNewDragWindow(GUI_Widget *dragWidget, drgVec2 *globalPos)
{
	if (dragWidget == NULL)
	{
		drgPrintError("Can't create a drag window without a valid drag widget\r\n");
		return NULL;
	}

	float dragWidgetWidth = dragWidget->WidthBox();
	float dragWidgetHeight = dragWidget->HeightBox();

	assert(dragWidget->GetParentWindow());
	GUI_Style *style = dragWidget->GetParentWindow()->GetStyle();

	// First we need to create a borderless window to represent the "drag window"
	GUI_Window *dragWindow = CreateNativeWindow(NULL,
												(int)dragWidgetWidth,
												(int)dragWidgetHeight,
												drgInputMouse::GetGlobalPos(),
												DRG_WINDOW_NOBORDERS | DRG_WINDOW_CHILD,
												style);

	// Make sure to capture the mouse so that the cursor stays within the bounds of the window.
	drgEngine::NativeMouseCapture(dragWindow);

	// As of the time of this comment, all GUI_Widgets' parents are GUI_Containers.
	// We need to remove the drag widget from it's previous parent.
	GUI_Container *container = (GUI_Container *)dragWidget->GetParent();
	if (container)
	{
		container->RemoveChild(dragWidget);
	}

	// Reposition the widget for the new window.
	dragWidget->SetPos(0.0f, 0.0f);
	dragWidget->SetWidth(dragWidgetWidth);
	dragWidget->SetHeight(dragWidgetHeight);
	dragWidget->UpdateSize();

	// Make the widget expand so that it resizes with the window.
	dragWidget->SetFlags(m_DragWidget->GetFlags() | DRG_WIDGET_RESIZE_EXPAND);

	// Make sure the drag window doesn't get smaller than the widget
	dragWindow->SetMinWidth(dragWidget->GetMinWidth());
	dragWindow->SetMinHeight(dragWidget->GetMinHeight());

	// Make sure that we keep the same resize type as the old container, so we can maintain appearances.
	dragWindow->SetContainerResizeType(container->GetContainerResizeType());
	dragWindow->AddChild(dragWidget);

	return dragWindow;
}

// UpdateHoverTimer
// Inputs:
//		float deltaTime: The time since the last frame
// Outputs:
//		None
// Description:
//		Will update the amount of time that has passed since the mouse started being over a widget that is considered the top-layer widget at that position
//

void GUI_Manager::UpdateHoverTimer(float deltaTime)
{
	if (HasHoverWidget())
	{
		m_TimeSinceHoverWidgetMouseEnter += deltaTime;
	}
}

void GUI_Manager::DrawHoverText(GUI_Drawing *draw)
{
	GUI_Widget *hoverWidget = GetHoverWidget();
	if (m_TimeSinceHoverWidgetMouseEnter >= GUI_HOVER_TEXT_DELAY)
	{
		string16 hoverText;
		hoverWidget->GetHoverText(&hoverText);
		if (!hoverText.empty())
		{
			GUI_Window *widgetWin = hoverWidget->GetParentWindow();
			if (widgetWin == NULL)
				widgetWin = GetBaseWindow();

			short top, bot, left, right;
			draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
			short newTop = (short)widgetWin->PosYView();
			short newBot = newTop + (short)widgetWin->HeightView();
			short newLeft = (short)widgetWin->PosXView();
			short newRight = newLeft + (short)widgetWin->WidthView();
			draw->GetRenderCache()->SetClipRect(newTop, newBot, newLeft, newRight);

			draw->GetStyle()->HoverTextBox(hoverWidget, hoverText, *drgInputMouse::GetPos());

			draw->GetRenderCache()->SetClipRect(top, bot, left, right);
		}
		else
		{
			m_TimeSinceHoverWidgetMouseEnter = 0.0f;
		}
	}
}

//
// UpdateDoubleClickTimer
// Inputs:
//		float deltaTime: The time since the last frame
// Outputs:
//		None
// Description:
//		Will update the time for a double click
//

void GUI_Manager::UpdateDoubleClickTimer(float deltaTime)
{
	if (m_ClickStarted)
	{
		m_TimeSinceLastClick += deltaTime;
		if (m_TimeSinceLastClick >= m_DoubleClickTimer)
		{
			m_ClickStarted = false;
		}
	}
}

//
// StartDoubleClick
// Inputs:
//		unsigned int winid: The window this is intended for
//		int button: The button used
// Outputs:
//		None
// Description:
//		Will start testing for a double click
//

void GUI_Manager::StartDoubleClick(unsigned int winid, int button)
{
	m_TimeSinceLastClick = 0.0f;
	m_DoubleClickWinID = winid;
	m_DoubleClickMouseButton = button;

	m_ClickStarted = true;
	m_ClickFinished = false;
}

//
// CheckDoubleClick
// Inputs:
//		unsigned int winid: The window this is intended for
//		int button: The button pressed
// Outputs:
//		None
// Description:
//		Will check to see if a double click has happened successfully.
//

void GUI_Manager::CheckDoubleClick(unsigned int winid, int button)
{
	if (m_TimeSinceLastClick < m_DoubleClickTimer &&
		m_ClickFinished &&
		winid == m_DoubleClickWinID &&
		button == m_DoubleClickMouseButton)
	{
		CallMouseDoubleClick(winid, button);
	}
	else
	{
		m_ClickStarted = false;
		m_ClickFinished = false;
	}
}

// Late initialization for script object memory, because GUI_Manager init happens before the engine is fully initialized
void GUI_Manager::UseAsScriptObjectLazy()
{
	if (m_HasBeenSetAsScriptObject == false)
	{
		UseAsScriptObject();
		m_HasBeenSetAsScriptObject = true;
	}
}
