
#ifndef _DRG_GUI_H_
#define _DRG_GUI_H_

#include "core/object.h"
#include "render/gui/font.h"
#include "render/gui/gui_manager.h"

#define DRG_WIDGET_FOCUSABLE 0x00000001            // Can grab focus
#define DRG_WIDGET_FOCUSED 0x00000002              // Holds focus (optimization)
#define DRG_WIDGET_UNFOCUSED_MOTION 0x00000004     // All mousemotion events
#define DRG_WIDGET_UNFOCUSED_BUTTONUP 0x00000008   // All mousebuttonup events
#define DRG_WIDGET_UNFOCUSED_BUTTONDOWN 0x00000010 // All mousebuttondown events
#define DRG_WIDGET_DIRTY 0x00000020                // Dirty
#define DRG_WIDGET_CURRENT_ACTIVE 0x00000040       // This is the current active widget for the parent.
#define DRG_WIDGET_NOCLIPPING 0x00000100           // Don't clip me.
#define DRG_WIDGET_HIDE 0x00000200                 // Don't draw this widget
#define DRG_WIDGET_DISABLED 0x00000400             // Don't respond to input
#define DRG_WIDGET_DOCK_RECEIVER 0x00000800        // I can be docked in.
#define DRG_WIDGET_DOCKER 0x00001000               // I can be used as a docker control.
#define DRG_WIDGET_CATCH_TAB 0x00002000            // Catch tab key events
#define DRG_WIDGET_UNDERSIZE 0x00004000            // Size allocation failed
#define DRG_WIDGET_NOSPACING 0x00008000            // Disable spacings around widget; container-specific
#define DRG_WIDGET_UNFOCUSED_KEYDOWN 0x00010000    // All mousebuttondown events
#define DRG_WIDGET_UNFOCUSED_KEYUP 0x00020000      // All mousebuttondown events
#define DRG_WIDGET_DEBUG_RSENS 0x00040000          // Debug sensitivity rect
#define DRG_WIDGET_TABLE_EMBEDDABLE 0x00080000     // Usable in polled tables
#define DRG_WIDGET_UPDATE_WINDOW 0x00100000        // Request an update as soon as possible
#define DRG_WIDGET_RESIZE_HFILL 0x01000000         // Will fill my parent's bounds horizontally
#define DRG_WIDGET_RESIZE_VFILL 0x02000000         // Will fill my parent's bounds vertically
#define DRG_WIDGET_DEBUG_CLIP_RECT 0x04000000      // Debug: draw a box for the RenderCache clip rectangle
#define DRG_WIDGET_RESIZE_EXPAND (DRG_WIDGET_RESIZE_HFILL | DRG_WIDGET_RESIZE_VFILL)

#define DRG_WINDOW_MODAL 0x0000001         // Place in foreground
#define DRG_WINDOW_MAXIMIZED 0x0000002     // Window is maximized
#define DRG_WINDOW_BASE_WINDOW 0x0000004   // Window is the base window.
#define DRG_WINDOW_KEEPABOVE 0x0000008     // Keep window above
#define DRG_WINDOW_KEEPBELOW 0x0000010     // Keep window below
#define DRG_WINDOW_DENYFOCUS 0x0000020     // Widgets cannot gain focus
#define DRG_WINDOW_NOTITLE 0x0000040       // Disable the titlebar
#define DRG_WINDOW_NOBORDERS 0x0000080     // Disable the window borders
#define DRG_WINDOW_NOHRESIZE 0x0000100     // Disable horizontal resize
#define DRG_WINDOW_NOVRESIZE 0x0000200     // Disable vertical resize
#define DRG_WINDOW_NOCLOSE 0x0000400       // Disable close button
#define DRG_WINDOW_NOMINIMIZE 0x0000800    // Diable the minimize button
#define DRG_WINDOW_NOMAXIMIZE 0x0001000    // Disable maximize button
#define DRG_WINDOW_CASCADE 0x0002000       // For DRG_WindowSetPosition()
#define DRG_WINDOW_MINSIZEPCT 0x0004000    // Set minimum size in %
#define DRG_WINDOW_NOBACKGROUND 0x0008000  // Don't fill the background
#define DRG_WINDOW_FOCUSONATTACH 0x0020000 // Automatic focus on attach
#define DRG_WINDOW_HMAXIMIZE 0x0040000     // Keep maximized horizontally
#define DRG_WINDOW_VMAXIMIZE 0x0080000     // Keep maximized vertically
#define DRG_WINDOW_NOMOVE 0x0100000        // Disallow movement of window
#define DRG_WINDOW_MODKEYEVENTS 0x0400000  // Generate key{up,down} events for keypresses on modifier keys
#define DRG_WINDOW_DETACHING 0x0800000     // Window is being detached
#define DRG_WINDOW_POPUP 0x1000000         // "Popup" style (WM-dependent)
#define DRG_WINDOW_DIALOG 0x2000000        // "Dialog" style (WM-dependent)
#define DRG_WINDOW_CHILD 0x4000000         // Window is a child of the base window

#define DRG_WINDOW_NORESIZE (DRG_WINDOW_NOHRESIZE | DRG_WINDOW_NOVRESIZE)
#define DRG_WINDOW_NOBUTTONS (DRG_WINDOW_NOCLOSE | DRG_WINDOW_NOMINIMIZE | DRG_WINDOW_NOMAXIMIZE)
#define DRG_WINDOW_PLAIN (DRG_WINDOW_NOTITLE | DRG_WINDOW_NOBORDERS)

#define DRG_PANE_HOMOGENOUS 0x001 // Divide space evenly
#define DRG_PANE_HFILL 0x002      // Expand to fill available width
#define DRG_PANE_VFILL 0x004      // Expand to fill available height
#define DRG_PANE_FRAME 0x008      // Display a frame by default
#define DRG_PANE_NOCLIPPING 0x010 // Don't set a clipping rectangle over the window area
#define DRG_PANE_PAN_X 0x020      // X is pannable
#define DRG_PANE_PAN_Y 0x040      // Y is pannable
#define DRG_PANE_PANNING 0x080    // Panning in progress
#define DRG_PANE_BY_MOUSE 0x100   // Panning with mouse allowed
#define DRG_PANE_SCROLL_X 0x200   // X is scrollable
#define DRG_PANE_SCROLL_Y 0x400   // Y is scrollable
#define DRG_PANE_SHOW_NAME 0x800  // name is visible
#define DRG_PANE_SCROLL_XY (DRG_PANE_SCROLL_X | DRG_PANE_SCROLL_Y)
#define DRG_PANE_PAN_XY (DRG_PANE_PAN_X | DRG_PANE_PAN_Y)
#define DRG_PANE_EXPAND (DRG_PANE_HFILL | DRG_PANE_VFILL)

#define DRG_LABEL_HFILL 0x01     // Fill horizontal space
#define DRG_LABEL_VFILL 0x02     // Fill vertical space
#define DRG_LABEL_NOMINSIZE 0x04 // No minimum enforced size
#define DRG_LABEL_PARTIAL 0x10   // Partial mode (RO)
#define DRG_LABEL_REGEN 0x20     // Regenerate surface at next draw
#define DRG_LABEL_FRAME 0x80     // Draw visible frame
#define DRG_LABEL_EXPAND (AG_LABEL_HFILL | AG_LABEL_VFILL)

#define DRG_ICON_REGEN_LABEL 0x01 // Update text label
#define DRG_ICON_DND 0x02         // Drag-and-drop in progress
#define DRG_ICON_DBLCLICKED 0x04  // Double click flag
#define DRG_ICON_BGFILL 0x08      // Enable background fill

#define DRG_BUTTON_STICKY 0x0002    // Toggle state
#define DRG_BUTTON_MOUSEOVER 0x0004 // Mouse overlaps
#define DRG_BUTTON_REPEAT 0x0008    // Repeat button-pushed event
#define DRG_BUTTON_HFILL 0x0010     // Fill available width
#define DRG_BUTTON_VFILL 0x0020     // Fill available height
#define DRG_BUTTON_INVSTATE 0x0400  // Invert value of "state" binding
#define DRG_BUTTON_KEYDOWN 0x0800   // Got `key-down' before `key-up'
#define DRG_BUTTON_NO_BG 0x1000     // Catch tab key events
#define DRG_BUTTON_EXPAND (DRG_BUTTON_HFILL | DRG_BUTTON_VFILL)

#define SLIDER_BAR_OFFSET 15

#define DRG_SCROLLBAR_EXPAND 0x01
#define DRG_SCROLLBAR_AUTOSIZE 0x10 // Automatically size control
#define DRG_SCROLLBAR_AUTOHIDE 0x20 // Show/hide widget based on range

#define GUI_SPINNER_CLAMP_VALUES 0x00000001

#define DRG_TITLEBAR_NO_CLOSE		0x01
#define DRG_TITLEBAR_NO_MAXIMIZE	0x04

#define DRG_LIST_HFILL 0x001    // Fill horizontal space
#define DRG_LIST_VFILL 0x002    // Fill vertical space
#define DRG_LIST_ICONS 0x004    // Show icons
#define DRG_LIST_TREE 0x008     // List is a tree
#define DRG_LIST_DRAGABLE 0x010 // Dragable items
#define DRG_LIST_EXPAND (DRG_LIST_HFILL | DRG_LIST_VFILL)

#define DRG_LIST_ITM_EXPAND 0x01
#define DRG_LIST_ITM_SELECTED 0x02
#define DRG_LIST_ITM_HOVER 0x04

#define DRG_LIST_DRAG_DELAY_IN_CENTISECONDS (025.0f) // because drgEngine::GetTotalTime() returns centiseconds for some reason

#define DRG_DROP_LIST_HFILL 0x001      // Fill horizontal space
#define DRG_DROP_LIST_VFILL 0x002      // Fill vertical space
#define DRG_DROP_LIST_MENU_ABOVE 0x004 // Create the menu above me.
#define DRG_DROP_LIST_EXPAND (DRG_LIST_HFILL | DRG_LIST_VFILL)

#define GUI_DROP_LIST_STATE_NORMAL 0x00
#define GUI_DROP_LIST_STATE_DROP 0x01
#define GUI_DROP_LIST_STATE_HOVER 0x02

#define MAX_LOG_LENGTH 256

enum GUI_CALLBACK_TYPE
{
    GUI_CB_GENRAL,
    GUI_CB_WINDOW_CLOSE,
    GUI_CB_MENU_CLICK,
    GUI_CB_BUTTON_DOWN,
    GUI_CB_BUTTON_CLICK,
    GUI_CB_BUTTON_UP,
    GUI_CB_BUTTON_IN,
    GUI_CB_BUTTON_OUT,
    GUI_CB_POINTER_DOWN,
    GUI_CB_POINTER_CLICK,
    GUI_CB_POINTER_UP,
    GUI_CB_SCREEN_MOVE,
    GUI_CB_SCREEN_UP,
    GUI_CB_SCREEN_DOWN,
    GUI_CB_TAB_CHANGE,
    GUI_CB_INPUT_CHANGE,
    GUI_CB_INPUT_FOCUS_LOST,
    GUI_CB_SCROLL_CHANGE,
    GUI_CB_LIST_CHANGE,
    GUI_CB_LIST_ITM_DOUBLE_CLICKED,
    GUI_CB_LIST_DROP,
    GUI_CB_SLIDER_CHANGE,
    GUI_CB_MAX
};

enum DEFAULT_COLOR
{
    BG_COLOR,
    FRAME_COLOR,
    LINE_COLOR,
    TEXT_COLOR,
    FOCUS_COLOR,
    FOCUS_FOREGROUND_COLOR,
    TEXT_FOCUS_COLOR,
    TEXT_FOCUS_BG_COLOR,
    TEXT_FOCUS_RECT_COLOR,
    TEXT_HIGHLIGHT_COLOR,
    ICON_COLOR,
    WINDOW_BG_COLOR,
    WINDOW_HI_COLOR,
    WINDOW_LO_COLOR,
    TITLEBAR_FOCUSED_COLOR,
    TITLEBAR_UNFOCUSED_COLOR,
    TITLEBAR_CAPTION_COLOR,
    TITLEBAR_DRAG_COLOR,
    BUTTON_COLOR,
    BUTTON_HOVER_COLOR,
    BUTTON_DISABLED_COLOR,
    BUTTON_DOWN_COLOR,
    BUTTON_TXT_COLOR,
    DISABLED_COLOR,
    CHECKBOX_COLOR,
    CHECKBOX_TXT_COLOR,
    GRAPH_BG_COLOR,
    GRAPH_XAXIS_COLOR,
    HSVPAL_CIRCLE_COLOR,
    HSVPAL_TILE1_COLOR,
    HSVPAL_TILE2_COLOR,
    MENU_COLOR,
    MENU_OVER_COLOR,
    MENU_SEL_COLOR,
    MENU_OPTION_COLOR,
    MENU_TXT_COLOR,
    MENU_SEP1_COLOR,
    MENU_SEP2_COLOR,
    NOTEBOOK_BG_COLOR,
    NOTEBOOK_TAB_COLOR,
    NOTEBOOK_SEL_COLOR,
    NOTEBOOK_TXT_COLOR,
    NOTEBOOK_TXT_SEL_COLOR,
    DOCKING_INDICATION_COLOR,
    DOCKING_RECEIVER_COLOR,
    RADIO_SEL_COLOR,
    RADIO_OVER_COLOR,
    RADIO_HI_COLOR,
    RADIO_LO_COLOR,
    RADIO_TXT_COLOR,
    SCROLLBAR_COLOR,
    SCROLLBAR_BTN_COLOR,
    SCROLLBAR_ARR1_COLOR,
    SCROLLBAR_ARR2_COLOR,
    SEPARATOR_LINE1_COLOR,
    SEPARATOR_LINE2_COLOR,
    PROGRESSBAR_COLOR,
    PROGRESSBAR_FILL_COLOR,
    TABLEVIEW_COLOR,
    TABLEVIEW_HEAD_COLOR,
    TABLEVIEW_HTXT_COLOR,
    TABLEVIEW_CTXT_COLOR,
    TABLEVIEW_LINE_COLOR,
    TABLEVIEW_SEL_COLOR,
    TEXTBOX_COLOR,
    TEXTBOX_FOCUS_COLOR,
    TEXTBOX_TXT_COLOR,
    TEXTBOX_TXT_FOCUS_COLOR,
    TEXTBOX_CURSOR_COLOR,
    TEXTBOX_CURSOR_FOCUS_COLOR,
    TLIST_TXT_COLOR,
    TLIST_BG_COLOR,
    TLIST_LINE_COLOR,
    TLIST_SEL_COLOR,
    TLIST_TEXT_SEL_COLOR,
    TLIST_DRAG_COLOR,
    TLIST_RECEIVER_COLOR,
    MAPVIEW_GRID_COLOR,
    MAPVIEW_CURSOR_COLOR,
    MAPVIEW_TILE1_COLOR,
    MAPVIEW_TILE2_COLOR,
    MAPVIEW_MSEL_COLOR,
    MAPVIEW_ESEL_COLOR,
    TILEVIEW_TILE1_COLOR,
    TILEVIEW_TILE2_COLOR,
    TILEVIEW_TEXTBG_COLOR,
    TILEVIEW_TEXT_COLOR,
    TRANSPARENT_COLOR,
    HSVPAL_BAR1_COLOR,
    HSVPAL_BAR2_COLOR,
    PANE_COLOR,
    PANE_DIV_COLOR,
    PANE_BORDER_COLOR,
    MAPVIEW_RSEL_COLOR,
    MAPVIEW_ORIGIN_COLOR,
    TABLE_COLOR,
    TABLE_LINE_COLOR,
    STATUS_BAR_BG_COLOR,
    STATUS_BAR_BG_WARN_COLOR,
    STATUS_BAR_BG_ERROR_COLOR,
    HOVER_TEXT_BG_COLOR,
    HOVER_TEXT_COLOR,
    FIXED_BG_COLOR,
    FIXED_BOX_COLOR,
    TEXT_DISABLED_COLOR,
    MENU_TXT_DISABLED_COLOR,
    PROGRESS_BAR_COLOR,
    WINDOW_BORDER_COLOR,
    PROXY_SEL_COLOR,
    SLIDER_BACKGROUND,
    SLIDER_TAB,
    LAST_COLOR,
};

enum DEFAULT_TEXTURE
{
    ICON_NONE = 0,
    ICON_ARRW_UP,
    ICON_ARRW_UP_UP,
    ICON_ARRW_UP_DOWN,
    ICON_ARRW_DOWN,
    ICON_ARRW_DOWN_UP,
    ICON_ARRW_DOWN_DOWN,
    ICON_ARRW_LEFT,
    ICON_ARRW_LEFT_UP,
    ICON_ARRW_LEFT_DOWN,
    ICON_ARRW_RIGHT,
    ICON_ARRW_RIGHT_UP,
    ICON_ARRW_RIGHT_DOWN,
    ICON_REFRESH,
    ICON_REFRESH_UP,
    ICON_REFRESH_DOWN,
    ICON_DIR_UP,
    ICON_DIR_UP_UP,
    ICON_DIR_UP_DOWN,
    ICON_CLOSE,
    ICON_CLOSE_UP,
    ICON_CLOSE_DOWN,
    ICON_MAXIMIZE,
    ICON_MAXIMIZE_UP,
    ICON_MAXIMIZE_DOWN,
    ICON_MINIMIZE,
    ICON_MINIMIZE_UP,
    ICON_MINIMIZE_DOWN,
    ICON_CHECK,      // off
    ICON_CHECK_UP,   // off & mouseover
    ICON_CHECK_DOWN, // on
    ICON_FOLDER,
    ICON_FILE,
    ICON_ASSET,
    ICON_FONT,
    ICON_TEXT,
    ICON_ZIP,
    ICON_PICTURE,
    ICON_AUDIO,
    ICON_ANIM,
    ICON_SCREEN,
    ICON_PARTICLE,
    ICON_VIDEO,
    ICON_SCRIPT,
    ICON_CODE,
    ICON_CODE_H,
    ICON_CODE_CS,
    ICON_CODE_CPP,
    ICON_CODE_C,
    ICON_CODE_JS,
    ICON_CODE_CSS,
    ICON_CUBE,
    ICON_GEAR,
    TEXTURE_TAB_FOCUSED,
    TEXTURE_TAB_UNFOCUSED,
    TEXTURE_TEXTBOX_1LINE, // single line textbox
    TEXTURE_TEXTBOX_1LINE_FOCUSED,
    TEXTURE_TEXTBOX_1LINE_DISABLED,
    TEXTURE_DIVIDER_HORIZONTAL,
    TEXTURE_DIVIDER_VERTICAL,
    TEXTURE_MENU_SEPARATOR_HORIZONTAL,
    TEXTURE_BORDER_TOP,
    TEXTURE_BORDER_LEFT,
    TEXTURE_BORDER_BOTTOM,
    TEXTURE_BORDER_RIGHT,
    TEXTURE_GENERIC_BUTTON_BG,
    TEXTURE_GENERIC_BUTTON_BG_DISABLED,
    TEXTURE_GENERIC_BUTTON_BG_DOWN,
    TEXTURE_TITLE_BAR,
    TEXTURE_WINDOW_BACKGROUND,
    ICON_LAST,
};

enum GUI_WIDGET_TYPE // IF YOU CHANGE THIS, UPDATE THE drgWidget.cs's VERSION!
{
    GUI_TYPE_WIDGET = 0,
    GUI_TYPE_CONTAINER,
    GUI_TYPE_PANE,
    GUI_TYPE_RENDER_PANE,
    GUI_TYPE_TAB_CONTAINER,
    GUI_TYPE_DIV_CONTAINER,
    GUI_TYPE_BUTTON,
    GUI_TYPE_ICON,
    GUI_TYPE_LABEL,
    GUI_TYPE_LIST,
    GUI_TYPE_DROP_LIST,
    GUI_TYPE_MENU,
    GUI_TYPE_TABLE,
    GUI_TYPE_SCROLL,
    GUI_TYPE_TITLE_BAR,
    GUI_TYPE_WINDOW,
    GUI_TYPE_SLIDER,
    GUI_TYPE_RADIOBUTTON,
    GUI_TYPE_PROGRESSBAR,
    GUI_TYPE_GRAPH,
    GUI_TYPE_SPINNER,
    GUI_TYPE_COLORPICKER,
    GUI_TYPE_SCENEVIEWER,
    GUI_TYPE_PROPERTY,
    GUI_TYPE_INT_PROPERTY,
    GUI_TYPE_ENUM_PROPERTY,
    GUI_TYPE_FLOAT_PROPERTY,
    GUI_TYPE_BOOLEAN_PROPERTY,
    GUI_TYPE_POINT_3_PROPERTY,
    GUI_TYPE_POINT_2_PROPERTY,
    GUI_TYPE_COLOR_PROPERTY,
    GUI_TYPE_STRING16_PROPERTY,
    GUI_TYPE_TRANSFORM_PROPERTY,
    GUI_TYPE_ASSET_PROPERTY,
    GUI_TYPE_TEXTURE_ASSET_PROPERTY,
    GUI_TYPE_FONT_ASSET_PROPERTY,
    GUI_TYPE_MODEL_ASSET_PROPERTY,
    GUI_TYPE_PARTICLE_ASSET_PROPERTY,
    GUI_TYPE_ASSET_REFERENCE_PROPERTY,
    GUI_TYPE_OBJECT_REFERENCE_PROPERTY,
    GUI_TYPE_TEXTURE_ASSET_REFERENCE_PROPERTY,
    GUI_TYPE_FONT_ASSET_REFERENCE_PROPERTY,
    GUI_TYPE_PARTICLE_ASSET_REFERENCE_PROPERTY,
    GUI_TYPE_MODEL_ASSET_REFERENCE_PROPERTY,
    GUI_TYPE_TEXTURE_LIST_PROPERTY,
    GUI_TYPE_KEYFRAME_PROPERTY,
    GUI_TYPE_OBJECT_KEYFRAME_PROPERTY,
    GUI_TYPE_KEYFRAME_LIST_PROPERTY,
    GUI_TYPE_OBJECT_KEYFRAME_LIST_PROPERTY,
};

enum DRG_CONTAINER_TYPE
{
    DRG_CONTAINER_HORIZ,    // We split our parent horizontally
    DRG_CONTAINER_VERT,     // We split our parent vertically
    DRG_CONTAINER_ABRITRARY // We can placed anywhere.
};

enum DRG_CONTAINER_RESIZE_TYPE
{
    DRG_CONTAINER_RESIZE_HORIZONTAL_ALIGN, // All children will have equal width and extend the horizontal bounds of this container
    DRG_CONTAINER_RESIZE_VERTICAL_ALIGN,   // All children will have equal height and extend the vertical bounds of this container
    DRG_CONTAINER_RESIZE_ARBITRARY,        // Children will not be resized if the container is resized.
};

enum DRG_CONTAINER_CHILD_TYPE
{
    DRG_CONTAINER_CHILD_STANDARD = 0,
    DRG_CONTAINER_CHILD_DIV,
    DRG_CONTAINER_CHILD_TAB,
    DRG_CONTAINER_CHILD_PANE,
};

enum DRG_CONTAINER_DIV_POS
{
    DRG_CONTAINER_DIV_LEFT = 0,
    DRG_CONTAINER_DIV_RIGHT = 1,
    DRG_CONTAINER_DIV_TOP = 0,
    DRG_CONTAINER_DIV_BOTTOM = 1
};

enum DRG_WM_OPERATION
{
    DRG_WINOP_NONE,
    DRG_WINOP_MOVE,
    DRG_WINOP_LRESIZE,
    DRG_WINOP_RRESIZE,
    DRG_WINOP_HRESIZE
};

enum BTN_TYPE
{
    BTN_TYPE_NORMAL,
    BTN_TYPE_CHECK,
    BTN_TYPE_RADIO
};

enum DRG_BUTTON_STATE
{
    DRG_BUTTON_STATE_UP = 0,
    DRG_BUTTON_STATE_DOWN = 1,
};

enum GUI_SliderState
{
    GUI_SLIDER_STATE_NONE,
    GUI_SLIDER_STATE_DEC,
    GUI_SLIDER_STATE_INC,
    GUI_SLIDER_STATE_SLIDE
};

enum GUI_ScrollType
{
    GUI_SCROLLBAR_HORIZ,
    GUI_SCROLLBAR_VERT
};

enum GUI_ScrollState
{
    GUI_SCROLLBAR_STATE_NONE,
    GUI_SCROLLBAR_STATE_DEC,
    GUI_SCROLLBAR_STATE_INC,
    GUI_SCROLLBAR_STATE_SCROLL
};

enum GUI_LIST_STATE
{
    GUI_LIST_STATE_DOWN,
    GUI_LIST_STATE_UP,
    GUI_LIST_STATE_MAX
};

class GUI_Widget;
class GUI_Window;
class GUI_Pane;
class GUI_Container;
class GUI_DivContainer;
class GUI_TabContainer;
class GUI_Menu;
class GUI_MenuItem;
class GUI_Label;
class GUI_Input;
class GUI_TitleBar;
class GUI_Button;
class GUI_Icon;
class GUI_Scroll;
class GUI_Slider;
class GUI_List;
class GUI_ListItem;
class GUI_DropList;
class GUI_DropListItem;
class GUI_ProgressBar;
class GUI_StatusBar;
class GUI_Table;
// class GUI_RadioButtonList;
// class GUI_Graph;
class GUI_Spinner;
class GUI_Log;

class GUI_ObjectList : public drgObjectList
{
public:
    GUI_ObjectList() {};
    ~GUI_ObjectList() { CleanUp(); };
};

class GUI_Widget : public drgObjectProp
{
public:
    virtual ~GUI_Widget();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void DrawDebug(GUI_Drawing *draw);
    virtual void DrawClipRectDebug(GUI_Drawing *draw, drgBBox clipRectOld, drgBBox clipRectNew);
    virtual void SelectSubWidget(GUI_Widget *widget);
    virtual void SetPos(float x, float y);
    virtual void GetNativePosView(drgVec2 *pos);
    virtual void GetNativePosBox(drgVec2 *pos);
    virtual void SetHoverWidget();

    virtual void Move(float x, float y);
    virtual void Resize(float top, float bot, float left, float right);
    virtual void ResizeRel(float top, float bot, float left, float right);
    virtual void OnParentResize();
    virtual void UpdateSize();

    virtual bool AddBBox(drgBBox *bbox);
    virtual void SetCurrentActive(GUI_Widget *active);
    virtual bool IsCursorInBounds();
    virtual bool IsPointerInBounds(int pointerIndex);

    virtual GUI_Window *GetParentWindow();
    virtual GUI_Pane *GetParentPane();
    virtual drgVec3 GetScrollOffset();
    virtual drgVec3 GetPaneClipMax();
    virtual void SetPaneClipMax(drgVec3 max);
    virtual GUI_Drawing *GetDrawContext();

    virtual void DragDraw(GUI_Drawing *draw);
    virtual void ReceiverDraw(GUI_Drawing *draw);
    virtual void DragDone();

    virtual bool IsPointInBounds(drgVec2 *point);
    virtual bool IsPointInGlobalBounds(drgVec2 *point);
    virtual bool IsChildOf(GUI_Widget *widget);

    virtual void ReceiveWidget(GUI_Widget *widget);

    virtual void SetParent(GUI_Widget *parent);
    virtual GUI_Widget *GetParent();

    virtual void SetClipRectWithOffsets(GUI_Drawing *draw, short top, short bottom, short left, short right);

    void Redraw();
    void WidgetOffsetRect(drgBBox *bbox);
    virtual int GetLayer();

    virtual void SetHoverText(const char *text);
    virtual void SetHoverText(const string16 text);
    virtual void GetHoverText(string16 *text_output);

    virtual float GetBorderWidth();

    virtual void SetUserData(drgObjectBase *data);
    virtual drgObjectBase *GetUserData();

    virtual void AddReceiverGUID(unsigned int receiverGUID);
    virtual void RemoveReceiverGUID(unsigned int receiverGUID);
    virtual void ClearReceiverGUIDs();
    virtual bool IsValidReceiver(GUI_Widget *widget);

    inline bool HasFlagWidget(unsigned int flag)
    {
        return ((flag & m_FlagsWidget) != 0);
    }

    inline void AddFlagWidget(unsigned int flag)
    {
        m_FlagsWidget |= flag;
    }

    inline void RemoveFlagWidget(unsigned int flag)
    {
        m_FlagsWidget &= ~flag;
    }

    inline void SetColor(drgColor *color)
    {
        m_Color = *color;
    }

    inline void SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
    {
        m_Color.Set(r, g, b, a);
    }

    inline drgColor *GetColor()
    {
        return &m_Color;
    }

    inline float PosXView()
    {
        return m_View.minv.x;
    }

    inline float PosYView()
    {
        return m_View.minv.y;
    }

    inline float WidthView()
    {
        return (m_View.maxv.x - m_View.minv.x);
    }

    inline float HeightView()
    {
        return (m_View.maxv.y - m_View.minv.y);
    }

    inline float PosXBox()
    {
        return m_Sens.minv.x;
    }

    inline float PosYBox()
    {
        return m_Sens.minv.y;
    }

    inline float WidthBox()
    {
        return (m_Sens.maxv.x - m_Sens.minv.x);
    }

    inline float HeightBox()
    {
        return (m_Sens.maxv.y - m_Sens.minv.y);
    }

    inline void SetFocused()
    {
        m_Manager->SetFocusedWidget(this);
    }

    inline void SetUnFocused()
    {
        m_Manager->SetFocusedWidget(NULL);
    }

    inline bool IsFocused()
    {
        return (m_Manager->GetFocusedWidget() == this);
    }

    inline void SetDragBox()
    {
        m_Manager->SetDragWidget(this);
    }

    inline bool IsDisabled()
    {
        return ((m_FlagsWidget & DRG_WIDGET_DISABLED) != 0);
    }

    inline void SetDisabled(bool disabled)
    {
        m_FlagsWidget = disabled ? m_FlagsWidget | DRG_WIDGET_DISABLED : m_FlagsWidget & ~DRG_WIDGET_DISABLED;
    }

    inline void Hide()
    {
        m_FlagsWidget |= DRG_WIDGET_HIDE;
    }

    inline void Show()
    {
        m_FlagsWidget &= (~DRG_WIDGET_HIDE);
    }

    inline bool IsHidden()
    {
        return ((m_FlagsWidget & DRG_WIDGET_HIDE) != 0);
    }

    inline void SetSelected()
    {
        if (m_Parent != NULL)
            m_Parent->SelectSubWidget(this);

        m_Manager->SetSelectedWidget(this);
    }

    inline bool IsSelected()
    {
        return (m_Manager->GetSelectedWidget() == this);
    }

    inline bool IsParentSelected()
    {
        if (m_Parent != NULL)
        {
            return (m_Manager->GetSelectedWidget() == m_Parent);
        }
        return false;
    }

    inline unsigned int GetGUID()
    {
        return m_GUID;
    }

    inline float PadTop()
    {
        return m_PadTop;
    }

    inline float PadBottom()
    {
        return m_PadTop;
    }

    inline float PadLeft()
    {
        return m_PadLeft;
    }

    inline float PadRight()
    {
        return m_PadLeft;
    }

    inline GUI_WIDGET_TYPE GetWidgetType()
    {
        assert(m_WidgetType != GUI_TYPE_WIDGET);
        return m_WidgetType;
    }

    inline float GetPercentLeft() { return m_LeftPercent; }
    inline float GetPercentRight() { return m_RightPercent; }
    inline float GetPercentUp() { return m_TopPercent; }
    inline float GetPercentDown() { return m_BottomPercent; }

    inline float GetMinWidth() { return m_MinWidth; }
    inline float GetMinHeight() { return m_MinHeight; }
    inline float GetMaxWidth() { return m_MaxWidth; }
    inline float GetMaxHeight() { return m_MaxHeight; }

    inline void SetPercentLeft(float left) { m_LeftPercent = left; }
    inline void SetPercentRight(float right) { m_LeftPercent = right; }
    inline void SetPercentTop(float top) { m_LeftPercent = top; }
    inline void SetPercentBottom(float bottom) { m_LeftPercent = bottom; }

    inline void SetMinWidth(float width) { m_MinWidth = width; }
    inline void SetMinHeight(float height) { m_MinHeight = height; }
    inline void SetMaxWidth(float width) { m_MaxWidth = width; }
    inline void SetMaxHeight(float height) { m_MaxHeight = height; }

    // This won't take effect until you call Resize();
    inline void SetWidth(float width) { m_Sens.maxv.x = m_Sens.minv.x + width; }

    // This won't take effect until you call Resize();
    inline void SetHeight(float height) { m_Sens.maxv.y = m_Sens.minv.y + height; }

    inline void SetCallBack(drgCallbackObj callback, void *data)
    {
        m_CallBack = callback;
        m_CallBackData = data;
    }

    inline void RunCallBack(GUI_CALLBACK_TYPE type = GUI_CB_GENRAL)
    {
        if (m_CallBack != NULL)
        {
            m_CallBackType = type;
            m_CallBack(this, m_CallBackData);
        }
    }

    inline bool HasCallBack()
    {
        return m_CallBack != NULL;
    }

    inline void RunCallBack(drgCallbackObj callback, void *data, GUI_CALLBACK_TYPE type = GUI_CB_GENRAL)
    {
        if (callback != NULL)
        {
            m_CallBackType = type;
            callback(this, data);
        }
    }

    inline GUI_CALLBACK_TYPE GetTypeCB()
    {
        return m_CallBackType;
    }

    inline drgBBox *GetView()
    {
        return &m_View;
    }

    inline drgBBox *GetBox()
    {
        return &m_Sens;
    }

    inline unsigned int GetFlags()
    {
        return m_FlagsWidget;
    }

    inline void SetFlags(unsigned int flags)
    {
        m_FlagsWidget = flags;
    }

    void SetManager(GUI_Manager *manager)
    {
        if (manager != NULL)
            manager->UseAsScriptObjectLazy();

        if (manager == NULL && m_Manager)
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
        m_Manager = manager;
    }

    GUI_Manager *GetManager()
    {
        if (m_Manager != NULL)
            m_Manager->UseAsScriptObjectLazy();

        return m_Manager;
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Widget"; }

protected:
    GUI_Widget();

    virtual void WidgetOffsetRectInt(drgBBox *bbox);

    drgBBox m_View; // Computed view coordinates
    drgBBox m_Sens; // Rectangle of sensitivity (i.e., to cursor events), in view coords
    drgColor m_Color;
    float m_PadTop;
    float m_PadBot;
    float m_PadLeft;
    float m_PadRight;

    // Percentages of parent
    float m_LeftPercent;
    float m_RightPercent;
    float m_TopPercent;
    float m_BottomPercent;

    float m_MinWidth;
    float m_MinHeight;
    float m_MaxWidth;
    float m_MaxHeight;

    unsigned int m_FlagsWidget;
    unsigned int m_GUID; // not to be confused with the "guid" attribute or the c# function drgObject.GetGUID, this is only for widgets
    GUI_Widget *m_Parent;
    drgCallbackObj m_CallBack;
    GUI_CALLBACK_TYPE m_CallBackType;
    void *m_CallBackData;

    string16 m_HoverText;

    GUI_WIDGET_TYPE m_WidgetType;

    drgObjectBase *m_UserData;

    unsigned int m_NumReceiverGUIDs;
    unsigned int *m_ReceiverGUIDs;

    friend class GUI_Container;
    friend class GUI_ObjectList;
    friend class GUI_Window;
    friend class GUI_Pane;
    friend class GUI_Manager;
    friend class GUI_List;

    GUI_Manager *m_Manager;
};

class GUI_Container : public GUI_Widget
{
public:
    virtual ~GUI_Container();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();

    virtual void Draw(GUI_Drawing *draw);

    virtual void InitCollapseButton();
    virtual void CallMouseDownCollapseButton(unsigned int winid, int button);
    virtual void AccordionDrawChildren(GUI_Drawing *draw);

    virtual void SelectSubWidget(GUI_Widget *widget);

    virtual void DragDone();
    virtual void ReceiverDraw(GUI_Drawing *drawing);
    virtual void ReceiveWidget(GUI_Widget *widget);

    virtual void UpdateSize();
    virtual bool AddBBox(drgBBox *bbox);
    virtual void OnParentResize();

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual bool CallMouseDoubleClick(unsigned int winid, int button);
    virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual void CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual bool CallPointerDown(unsigned int winid, int pointerIndex);
    virtual bool CallPointerClick(unsigned int winid, int pointerIndex);
    virtual void CallPointerUp(unsigned int winid, int pointerIndex);
    virtual void CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel);

    void ContainerSetType(DRG_CONTAINER_TYPE type);
    void SetContainerResizeType(DRG_CONTAINER_RESIZE_TYPE type);
    DRG_CONTAINER_RESIZE_TYPE GetContainerResizeType() { return m_ContainerResizeType; }
    bool IsContainer();

    virtual void AddChild(GUI_Widget *widget);
    virtual void AddChildAfter(GUI_Widget *toAdd, GUI_Widget *toAddAfter);
    virtual void RemoveChild(GUI_Widget *widget);
    virtual void RemoveAllChildren();
    virtual void DrawChildren(GUI_Drawing *draw);

    GUI_DivContainer *CreateChildDivContainer(unsigned int flags, DRG_CONTAINER_TYPE type, DRG_CONTAINER_CHILD_TYPE pane0, DRG_CONTAINER_CHILD_TYPE pane1);
    GUI_TabContainer *CreateChildTabContainer(unsigned int flags);
    GUI_Pane *CreateChildPane(string16 name, unsigned int flags);
    GUI_Label *CreateChildLabel(string16 text, unsigned int flags);
    GUI_Log *CreateChildLog(unsigned int flags);
    GUI_Input *CreateChildInput(unsigned int flags);
    GUI_Spinner *CreateChildSpinner(int width, int height, double startVal, double min, double max, int flags);
    GUI_ProgressBar *CreateChildProgressBar(int width, int height, float curVal, float completeVal, const char *label, unsigned int flags);
    GUI_Icon *CreateChildIcon(unsigned int width, unsigned int height, unsigned int flags);
    GUI_Icon *CreateChildIcon(char *filename, unsigned int flags);
    GUI_Button *CreateChildButton(string16 text, unsigned int width, unsigned int height, drgCallbackObj callback, void *cbdata, unsigned int flags);
    GUI_Button *CreateChildButton(char *tex, char *tex_up, char *tex_down, drgCallbackObj callback, void *cbdata, unsigned int flags);
    GUI_Button *CreateChildButton(DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down, drgCallbackObj callback, void *cbdata, unsigned int flags);
    GUI_Button *CreateChildCheckBox(drgCallbackObj callback, void *cbdata, unsigned int flags);
    GUI_Button *CreateChildRadioButton(drgCallbackObj callback, void *cbdata, unsigned int flags);
    // GUI_RadioButtonList*		CreateChildRadioButtonList(unsigned int flags);
    GUI_Scroll *CreateChildScroll(GUI_ScrollType type, float val, float min, float max, int flags);
    GUI_Slider *CreateChildSlider(int width, int height, float val, float min, float max, int flags);
    GUI_List *CreateChildList(unsigned int flags);
    GUI_DropList *CreateChildDropList(unsigned int flags);
    GUI_Container *CreateChildContainer(DRG_CONTAINER_TYPE type, unsigned int flags);
    GUI_Table *CreateChildTable(unsigned int flags);
    GUI_StatusBar *CreateChildStatusBar();
    GUI_Menu *CreateChildMenu();

    virtual GUI_Property *CreateChildProperty(drgProperty *prop);
    virtual GUI_Property *CreateChildPropertyWithIndex(drgObjectProp *objWithProperties, int propertyIndex);

    void CreateTitleBar(unsigned int flags, string16 titleText);
    void RemoveTitleBar();

    inline void SetDockRange(float range) { m_DockRange = range; }
    inline float GetDockRange() { return m_DockRange; }
    inline GUI_TitleBar *GetTitleBar() { return m_TitleBar; }

    virtual void SetCanCollapse(bool canCollapse);
    virtual void SetCollapseTitle(const char *title);
    virtual void SetCollapseTitle(string16 title);
    virtual float GetCollapsedHeight();
    inline bool IsCollapsed() { return m_CanCollapse ? m_IsCollapsed : false; }

    inline GUI_Widget *GetFirst() { return (GUI_Widget *)m_Children.GetFirst(); }
    inline GUI_Widget *GetLast() { return (GUI_Widget *)m_Children.GetLast(); }
    inline GUI_Widget *GetNext(GUI_Widget *widget) { return (GUI_Widget *)m_Children.GetNext((drgObjectProp *)widget); }
    inline GUI_Widget *GetPrev(GUI_Widget *widget) { return (GUI_Widget *)m_Children.GetPrev((drgObjectProp *)widget); }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Container"; }
    static GUI_Container *CreateContainer(GUI_Widget *parent, DRG_CONTAINER_TYPE type, unsigned int flags);

protected:
    static void HandleCollapseButtonDown(drgObjectBase *obj, void *data);

    GUI_Container();
    virtual void ResizeChildren();
    virtual void ResizeChildrenHorizontally();
    virtual void ResizeChildrenVertically();
    virtual void ResizeChildrenArbitrarily();
    virtual bool IndicateDockingLocations(GUI_Widget *dragWidget, drgVec2 *mousePos, drgBBox *boxToDraw);
    virtual bool IndicateHorizontalDockingLocations(GUI_Widget *dragWidget, drgVec2 *mousePos, drgBBox *boxToDraw);
    virtual bool IndicateVerticalDockingLocations(GUI_Widget *dragWidget, drgVec2 *mousePos, drgBBox *boxToDraw);
    virtual void ReceiveWidgetHorizontal(GUI_Widget *dragWidget, drgVec2 *localMousePos);
    virtual void ReceiveWidgetVertical(GUI_Widget *dragWidget, drgVec2 *localMousePos);
    virtual void UpdateTitleBarSize();
    virtual void CheckForReceievers();
    virtual void UpdateMinMaxValues();
    virtual void UpdateMinMaxHorizontal();
    virtual void UpdateMinMaxVertical();
    virtual void UpdateMinMaxArbitrary();
    virtual bool CanReceiveWidget(GUI_Widget *widget, drgVec2 *localDropPos);
    virtual void SetCollapsed(bool collapsed);

    virtual void OffsetFrameForBorder();

    DRG_CONTAINER_TYPE m_Type;
    DRG_CONTAINER_RESIZE_TYPE m_ContainerResizeType;
    GUI_ObjectList m_Children;
    GUI_TitleBar *m_TitleBar;
    GUI_Menu *m_Menu;
    bool m_CanCollapse;
    bool m_IsCollapsed;
    GUI_Button *m_CollapseButton;
    string16 m_CollapseTitle;
    float m_CollapsedHeight;
    float m_UnCollapsedHeight;

    float m_DockRange; // The distance from each edge to check.

    friend class GUI_Table;
};

class GUI_DivContainer : public GUI_Container
{
public:
    virtual ~GUI_DivContainer();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void ReceiverDraw(GUI_Drawing *drawing);
    virtual void ReceiveWidget(GUI_Widget *widget);
    virtual void UpdateSize();

    virtual void AccordionDrawChildren(GUI_Drawing *draw);

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual bool CallMouseDoubleClick(unsigned int winid, int button);
    virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual void CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual bool CallPointerDown(unsigned int winid, int pointerIndex);
    virtual bool CallPointerClick(unsigned int winid, int pointerIndex);
    virtual void CallPointerUp(unsigned int winid, int pointerIndex);
    virtual void CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel);

    GUI_Container *GetContainer(DRG_CONTAINER_DIV_POS pos);
    void SetContainer(DRG_CONTAINER_DIV_POS pos, GUI_Container *container);
    void EnableContainer(DRG_CONTAINER_DIV_POS pos, bool enable);
    void GetDividerBox(drgBBox *bbox);

    virtual void AddChild(GUI_Widget *widget);
    virtual void RemoveChild(GUI_Widget *widget);

    inline float GetDivSize()
    {
        return m_DivSize;
    }

    inline float GetMidValue()
    {
        return m_DivAmnt;
    }

    inline void SetMidValue(float val)
    {
        m_DivAmnt = DRG_CLAMP_NORM(val);
        UpdateSize();
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_DivContainer"; }

protected:
    GUI_DivContainer();

    static GUI_DivContainer *CreateDivContainer(GUI_Widget *parent, DRG_CONTAINER_TYPE type, unsigned int flags);
    virtual bool IndicateDockingLocations(GUI_Widget *dragWidget, drgVec2 *mousePos, drgBBox *boxToDraw);
    virtual bool CanReceiveWidget(GUI_Widget *widget, drgVec2 *localDropPos);

    float ClampDivAmount(float divAmount);
    void SetupDivPaneClippingRects(int paneIndex, GUI_Drawing *draw, short *top, short *bottom, short *left, short *right);
    virtual void UpdateMinMaxValues();

    unsigned int m_FlagsContainer;
    DRG_CONTAINER_TYPE m_Type;
    GUI_Container *m_DivPane[2];
    bool m_DivPaneEnable[2];
    float m_DivAmnt;
    float m_DivSize;

    friend class GUI_Container;
};

class GUI_TabContainer : public GUI_Container
{
public:
    virtual ~GUI_TabContainer();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void ReceiverDraw(GUI_Drawing *draw);
    virtual void ReceiveWidget(GUI_Widget *widget);
    virtual void UpdateSize();

    virtual void AddChild(GUI_Widget *widget);
    virtual void RemoveChild(GUI_Widget *widget);

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual bool CallMouseDoubleClick(unsigned int winid, int button);
    virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual void CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual bool CallPointerDown(unsigned int winid, int pointerIndex);
    virtual bool CallPointerClick(unsigned int winid, int pointerIndex);
    virtual void CallPointerUp(unsigned int winid, int pointerIndex);
    virtual void CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel);

    GUI_Container *CreateChildTab(string16 name, unsigned int flags, GUI_Container *container);

    inline bool IsPaneSelected(GUI_Pane *pane) { return (pane == m_CurPane); }
    inline int GetTotalTabs() { return m_Children.GetCount(); }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_TabContainer"; }

protected:
    GUI_TabContainer();
    void ResizeForTabButtons();
    void OnTabDragStart(GUI_Pane *pane);
    void SetCurPane(GUI_Pane *pane);
    void GetTabButtonBounds(GUI_Pane *pane, drgBBox *outBounds);
    void GetTabBarBounds(drgBBox *barBox);

    virtual void CheckForReceievers();
    virtual void UpdateMinMaxValues();

    static GUI_TabContainer *CreateTabContainer(GUI_Widget *parent, unsigned int flags);

    unsigned int m_FlagsContainer;
    GUI_Pane *m_CurPane;
    bool m_TabIsPressed;
    drgVec2 m_DragMovement;
    float m_DragThreshold;

    friend class GUI_Container;
    friend class GUI_Style; // For getting the button bounds.
};

class GUI_Pane : public GUI_Container
{
public:
    virtual ~GUI_Pane();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void UpdateSize();

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual bool CallMouseDoubleClick(unsigned int winid, int button);
    virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual bool CallPointerDown(unsigned int winid, int pointerIndex);
    virtual bool CallPointerClick(unsigned int winid, int pointerIndex);
    virtual void CallPointerUp(unsigned int winid, int pointerIndex);
    virtual void CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *pos, drgVec2 *rel);

    virtual void OffsetFrameForBorder();
    virtual void UpdateBorderWidth(GUI_Drawing *draw);
    virtual float GetBorderWidth();
    virtual float GetCollapsedHeight();

    virtual drgVec3 GetScrollOffset();
    virtual drgVec3 GetPaneClipMax();
    virtual void SetPaneClipMax(drgVec3 max);
    virtual GUI_Pane *GetParentPane();

    void SetHomogenous(int enable);

    void AddFlagPane(unsigned int flag) { m_FlagsPane |= flag; }

    inline string16 *GetName() { return &m_Name; }
    virtual void SetName(string16 name);

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Pane"; }

protected:
    GUI_Pane();

    virtual void UpdateScrollbars();
    virtual void StackChildren();
    virtual void WidgetOffsetRectInt(drgBBox *bbox);
    static GUI_Pane *CreatePane(unsigned int flags);

    unsigned int m_FlagsPane;
    string16 m_Name;
    drgVec3 m_ScrollOffset;
    GUI_Scroll *m_ScrollH;
    GUI_Scroll *m_ScrollV;
    float m_BorderWidth;
    drgVec3 m_PaneClipMax;

    friend class GUI_Container;
    friend class GUI_TabContainer;
};

class GUI_Window : public GUI_Container
{
public:
    virtual ~GUI_Window();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void Resize(float top, float bot, float left, float right);
    virtual void ResizeRel(float top, float bot, float left, float right);
    virtual void UpdateSize();
    virtual void OnParentResize();
    virtual void SetCurrentActive(GUI_Widget *active);
    virtual void SetPos(float x, float y);
    virtual GUI_Window *GetParentWindow();
    virtual GUI_Drawing *GetDrawContext();

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDoubleClick(unsigned int winid, int button);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual void CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual bool CallPointerDown(unsigned int winid, int pointerIndex);
    virtual bool CallPointerClick(unsigned int winid, int pointerIndex);
    virtual void CallPointerUp(unsigned int winid, int pointerIndex);
    virtual void CallPointerMove(unsigned int winid, int pointerIndex, drgVec2 *position, drgVec2 *rel);
    virtual void CallResize(unsigned int winid, drgVec2 *size);

    virtual bool IsCursorInBounds();

    virtual void GetNativePosView(drgVec2 *pos);
    virtual void GetNativePosBox(drgVec2 *pos);
    virtual GUI_Style *GetStyle();
    virtual void SetStyle(GUI_Style *style);

    virtual void OnCloseButtonPressed();

    void Draw();
    void Maximize();
    void SetModal(GUI_Window *window);

    bool WindowSelectedWM(DRG_WM_OPERATION op);

    GUI_Window *CreateChildWindow(string16 title, unsigned int width, unsigned int height, unsigned int flags);
    GUI_Menu *CreateMenu();

    virtual int GetWindowLayer();
    virtual int GetLayer();

    inline GUI_TitleBar *GetTitleBar()
    {
        return m_TitleBar;
    }

    inline GUI_Menu *GetMenu()
    {
        return m_Menu;
    }

    inline float GetBorderBot()
    {
        return m_BorderBot;
    }

    inline float GetBorderSide()
    {
        return m_BorderSide;
    }

    inline float GetBorderReSize()
    {
        return m_ResizeCtrl;
    }

    inline bool HasFlagWin(unsigned int flag)
    {
        return ((flag & m_FlagsWin) != 0);
    }

    inline void AddFlagsWin(unsigned int flags)
    {
        m_FlagsWin |= flags;
    }

    inline unsigned int GetFlagsWin()
    {
        return m_FlagsWin;
    }

    inline void SetFlagsWin(unsigned int flags)
    {
        m_FlagsWin = flags;
    }

    inline void SetCloseCallBack(drgCallbackObj callback, void *data)
    {
        m_CloseCallback = callback;
        m_CloseCallbackData = data;
    }

    static GUI_Window *CreateBaseWindow(string16 title, unsigned int width, unsigned int height, GUI_Style *style = NULL);
    static GUI_Window *CreateNativeWindow(string16 *title, unsigned int width, unsigned int height, drgVec2 *pos, unsigned int windowFlags = 0, GUI_Style *style = NULL);

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Window"; }

protected:
    GUI_Window();

    virtual void WidgetOffsetRectInt(drgBBox *bbox);

    GUI_TitleBar *m_TitleBar;
    GUI_Menu *m_Menu;
    GUI_Widget *m_CurrentActive;
    float m_BorderBot; // Bottom border
    float m_BorderSide;
    float m_ResizeCtrl;
    void *m_NativeData;
    GUI_Drawing *m_DrawContext;
    unsigned int m_WinOp;
    unsigned int m_FlagsWin;
    drgBBox m_Saved;
    drgCallbackObj m_CloseCallback;
    void *m_CloseCallbackData;

    friend class drgEngine;
    friend class RenderContext;
    friend class GUI_TitleBar;
    friend class GUI_Style;
};

class GUI_Label : public GUI_Widget
{
public:
    GUI_Label();
    virtual ~GUI_Label();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void UpdateSize();

    void SetText(string16 *text);
    void SetText(char *text);
    void SetText(const char *text);
    string16 *GetText();

    virtual void SetClipRectWithOffsets(GUI_Drawing *draw, short top, short bot, short left, short right);

    bool IsMouseOverParent();

    static GUI_Label *CreateLabel(GUI_Widget *parent, drgFont *font, string16 text, unsigned int flags);

    inline drgFont *GetFont() { return m_Font; }
    inline void SetFont(drgFont *font)
    {
        m_Font = font;
        assert(font);
        UpdateSize();
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Label"; }

protected:
    string16 m_Text;
    drgFont *m_Font;
    unsigned int m_FlagsLabel;

    friend class GUI_Container;
};

class GUI_Icon : public GUI_Widget
{
public:
    virtual ~GUI_Icon();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Icon"; }

    void SetTexture(char *filename);
    void SetTexture(drgTexture *texture);

protected:
    GUI_Icon();

    unsigned int m_FlagsIcon;
    drgTexture *m_Texture;

    friend class GUI_Container;
};

class GUI_Button : public GUI_Widget
{
public:
    virtual ~GUI_Button();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    void SetTitle(string16 *title);
    void SetIcon(DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down);
    void SetTexture(char *filename);

    virtual void SetClipRectWithOffsets(GUI_Drawing *draw, short top, short bot, short left, short right);

    // expands the left and right bounds of the button based on the width of the text.  the vairables 'left' and 'right' are modified.
    void AdjustWidth(float *left, float *right);

    inline DRG_BUTTON_STATE GetState()
    {
        return m_State;
    }

    inline void SetState(DRG_BUTTON_STATE state)
    {
        m_State = state;
    }

    inline bool HasFlagButton(unsigned int flag)
    {
        return ((flag & m_FlagsButton) != 0);
    }

    inline void RemoveFlagButton(unsigned int flag)
    {
        m_FlagsButton &= ~(flag);
    }

    static GUI_Button *CreateChildButton(GUI_Widget *parent, DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down, drgCallbackObj callback, void *cbdata, unsigned int flags);
    static GUI_Button *CreateChildRadioButton(GUI_Widget *parent, DEFAULT_TEXTURE icon, DEFAULT_TEXTURE icon_up, DEFAULT_TEXTURE icon_down, drgCallbackObj callback, void *cbdata, unsigned int flags);

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Button"; }

protected:
    GUI_Button();

    unsigned int m_FlagsButton;
    BTN_TYPE m_Type;
    DRG_BUTTON_STATE m_State;
    GUI_Label *m_Label;
    drgTexture *m_Textures[3];
    DEFAULT_TEXTURE m_Icons[3];

    friend class GUI_Container;
};

// single-line editable textbox
class GUI_Input : public GUI_Widget
{
public:
    static const int INVALID_HIGHLIGHT_POS;

    GUI_Input();
    virtual ~GUI_Input();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    virtual void SetUnFocused();

    void SetText(string16 *text);
    void SetText(char *text);
    void SetText(const char *text);
    string16 *GetText();

    virtual bool IsFocused()
    {
        assert(m_Manager);
        return m_Manager->GetFocusedWidget() == this;
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Input"; }

    virtual void NormalizeHighlight();
    virtual void ReplaceHighlightedText(int charval);
    virtual void ReplaceHighlightedText(string16 text = string16());
    virtual void SetReplaceModeHighlight();

protected:
    void CheckForKeyUp(int key);

    string16 m_Text;
    unsigned int m_FlagsInput;
    int m_MaxLength;
    int m_StartPos;
    int m_EndPos;
    int m_CurPos;
    int m_Offset;
    int m_HighlightPos;
    int m_TextOffset;
    bool m_UpdatePos;
    bool m_bShiftDown;
    bool m_bReplaceMode;

    drgVec2 *m_pos;
    drgVec2 *m_rel;

    friend class GUI_Container;
    friend class GUI_Spinner;
};

class GUI_TitleBar : public GUI_Widget
{
public:
    virtual ~GUI_TitleBar();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void DragDraw(GUI_Drawing *draw);
    virtual void DragDone();
    virtual void Resize(float top, float bot, float left, float right);
    virtual void ResizeRel(float top, float bot, float left, float right);

    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    void SetTitle(string16 *title);

    inline bool IsPressed()
    {
        return m_Pressed;
    }

    inline string16 *GetTitle()
    {
        return m_Label->GetText();
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_TitleBar"; }

protected:
    GUI_TitleBar();
    void CreateMaximizeButton();
    void CreateCloseButton();

    static GUI_TitleBar *CreateTitleBar(GUI_Widget *parent, drgFont *font, unsigned int flags, string16 *title);
    static void BtnCallBckClose(drgObjectBase *widget, void *data);

    bool m_Pressed;
    unsigned int m_FlagsTitleBar;
    GUI_Label *m_Label;
    GUI_Button *m_BtnClose;
    GUI_Button *m_BtnMax;
    drgVec2 m_DragMovement;
    float m_DragThreshold;

    friend class GUI_Window;
    friend class GUI_Container;
};

class GUI_MenuItem : public GUI_Widget
{
public:
    virtual ~GUI_MenuItem();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual bool CallMouseClick(unsigned int winid, int button);

    virtual int GetLayer();

    inline string16 *GetText()
    {
        return m_Label->GetText();
    }

    inline bool IsMouseOver()
    {
        return m_MouseOver;
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_MenuItem"; }

protected:
    GUI_MenuItem();

    bool m_MouseOver;
    bool m_Seperator;
    GUI_Label *m_Label;
    GUI_Menu *m_SubMenu;

    friend class GUI_Menu;
    friend class GUI_Window;
};

class GUI_Menu : public GUI_Widget
{
public:
    virtual ~GUI_Menu();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    float OwnerTitleHeight();
    float GetItemsHeight();
    float GetItemsWidth();

    GUI_Menu *CreateSubMenu(string16 text);
    GUI_MenuItem *CreateMenuItem(string16 text, drgCallbackObj callback, void *data = NULL);
    void CreateMenuSeperator();

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Menu"; }

protected:
    GUI_Menu();

    static GUI_Menu *CreateMenu(drgFont *font, bool popup, int lines, float offx);

    GUI_Window *m_Owner;
    GUI_Container *m_ContainParent;
    GUI_Menu *m_ParentMenu;
    drgFont *m_Font;
    GUI_ObjectList m_MenuItems;
    bool m_PopUp;
    float m_ItemMaxSpace;
    float m_ItemSpaceX;
    float m_ItemSpaceY;
    float m_ItemSepSize;

    friend class GUI_MenuItem;
    friend class GUI_DropList;
    friend class GUI_Container;
    friend class GUI_Window;
};

class GUI_Slider : public GUI_Widget
{
public:
    GUI_Slider();
    virtual ~GUI_Slider();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void UpdateSize();

    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    inline void SetMin(float min)
    {
        m_Min = min;
        SetValue(m_Value);
    }

    inline void SetMax(float max)
    {
        m_Max = max;
        SetValue(m_Value);
    }

    inline void SetLengthScale(float scale)
    {
        m_Scale = scale;
    }

    inline float GetLengthScale()
    {
        return m_Scale;
    }

    inline float GetValue()
    {
        return m_Value;
    }

    inline void SetValue(float value)
    {
        m_Value = DRG_CLAMP(value, m_Min, m_Max);
        UpdateSliderPos();
        RunCallBack(GUI_CB_SCROLL_CHANGE);
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Slider"; }

protected:
    void UpdateSliderPos();
    void CalcSliderValue(drgVec2 *pos);
    void IncrementSlider();
    void DecrementSlider();

    float GetSliderBoxWidth();

    static GUI_Slider *CreateChildSlider(GUI_Widget *parent, int width, int height, float val, float min, float max, int flags);
    static void BtnCallBckInc(drgObjectBase *widget, void *data);
    static void BtnCallBckDec(drgObjectBase *widget, void *data);

    unsigned int m_FlagsSlider;
    float m_Value;
    float m_Scale;
    float m_Min;
    float m_Max;
    float m_Size;
    float m_CountIncDec;
    float m_CountIncAmt;
    GUI_SliderState m_State;
    drgBBox m_SliderArea;
    drgBBox m_SliderBox;
    GUI_Button *m_BtnInc;
    GUI_Button *m_BtnDec;

    friend class GUI_Container;
};

class GUI_Scroll : public GUI_Widget
{
public:
    virtual ~GUI_Scroll();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void UpdateSize();

    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    inline void SetMin(float min)
    {
        m_Min = min;
        SetValue(m_Value);
        UpdateSize();
    }

    inline void SetMax(float max)
    {
        m_Max = max;
        SetValue(m_Value);
        UpdateSize();
    }

    inline void SetLengthScale(float scale)
    {
        m_Scale = scale;
        UpdateSize();
    }

    inline float GetLengthScale()
    {
        return m_Scale;
    }

    inline float GetValue()
    {
        return m_Value;
    }

    inline void SetValue(float value)
    {
        m_Value = DRG_CLAMP(value, m_Min, m_Max);
        UpdateScrollPos();
        RunCallBack(GUI_CB_SCROLL_CHANGE);
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Scroll"; }

protected:
    GUI_Scroll();

    void UpdateScrollPos();
    void CalcScrollValue(drgVec2 *pos);
    void IncrementScroll();
    void DecrementScroll();

    static GUI_Scroll *CreateChildScroll(GUI_Widget *parent, GUI_ScrollType type, float val, float min, float max, int flags);
    static void BtnCallBckInc(drgObjectBase *widget, void *data);
    static void BtnCallBckDec(drgObjectBase *widget, void *data);

    unsigned int m_FlagsScroll;
    float m_Value;
    float m_Scale;
    float m_Min;
    float m_Max;
    float m_Size;
    float m_CountIncDec;
    float m_CountIncAmt;
    GUI_ScrollType m_Type;
    GUI_ScrollState m_State;
    drgBBox m_ScrollArea;
    drgBBox m_ScrollBox;
    GUI_Button *m_BtnInc;
    GUI_Button *m_BtnDec;

    friend class GUI_Container;
    friend class GUI_Pane;
    friend class GUI_List;
};

class GUI_Spinner : public GUI_Container
{
public:
    GUI_Spinner();
    virtual ~GUI_Spinner();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);

    inline void SetMin(double nMin)
    {
        m_dMinValue = nMin;
    }

    inline void SetMax(double nMax)
    {
        m_dMinValue = nMax;
    }

    inline double GetSpinnerValue()
    {
        return m_dSpinnerValue;
    }

    inline double GetSpinnerStepValue()
    {
        return m_dStepValue;
    }

    inline void SetSpinnerValue(double nValue)
    {
        m_dSpinnerValue = nValue;
    }

    inline void SetSpinnerStepValue(double nValue)
    {
        m_dStepValue = nValue;
    }

    inline void AddFlagSpinner(unsigned int flag) { m_FlagsSpinner |= flag; }
    inline void RemoveFlagSpinner(unsigned int flag) { m_FlagsSpinner &= ~flag; }
    inline bool HasFlagSpinner(unsigned int flag) { return (m_FlagsSpinner & flag) != 0; }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Spinner"; }

protected:
    static GUI_Spinner *CreateChildSpinner(GUI_Widget *parent, int width, int height, double startVal, double min, double max, int flags);
    static void BtnCBSpinnerInc(drgObjectBase *widget, void *data);
    static void BtnCBSpinnerDec(drgObjectBase *widget, void *data);
    static void OnInputChanged(drgObjectBase *object, void *data);

    void IncrementSpinner();
    void DecrementSpinner();

    unsigned int m_FlagsSpinner;

    GUI_Input *m_InputBox;
    GUI_Button *m_ButtonUp;
    GUI_Button *m_ButtonDown;
    double m_dSpinnerValue;
    double m_dMinValue;
    double m_dMaxValue;
    double m_dStepValue;

    friend class GUI_Container;
};

class GUI_ProgressBar : public GUI_Widget
{
public:
    virtual ~GUI_ProgressBar();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void Update();

    void SetTitle(string16 *title);
    void SetTexture(char *filenameBG, char *filenameProgress);

    inline void SetValues(float nCurrentValue, float nCompleteValue)
    {
        m_nCurrentVal = nCurrentValue;
        m_nCompleteVal = nCompleteValue;
    }

    inline void UpdateCurrent(float nCurrentValue)
    {
        m_nCurrentVal = nCurrentValue;
        Update();
    }

    inline float GetProgress()
    {
        Update();

        return m_nProgress;
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_ProgressBar"; }

protected:
    GUI_ProgressBar();

    static GUI_ProgressBar *CreateChildProgressBar(GUI_Widget *parent, int width, int height, float curVal, float completeVal, const char *label, unsigned int flags);

    float m_nCurrentVal;
    float m_nProgress;
    float m_nCompleteVal;
    GUI_Label *m_Label;
    drgTexture *m_Textures[2];

    friend class GUI_Container;
};

class GUI_StatusBar : public GUI_Widget
{
public:
    static const int STATUS_BUFFER_LEN = 4096 + 1024; // 5kb
    static GUI_StatusBar *CreateStatusBar(GUI_Widget *parent, drgFont *font);

    GUI_StatusBar();
    virtual ~GUI_StatusBar();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);

    virtual void HandleChangeStatus(DRG_DEBUG_LEVEL debugLevel, const char *message, int size);
    virtual void HandleChangeStatus(int debugLevel, const char *message, int size);
    virtual void Clear();

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_StatusBar"; }

protected:
    char *m_CurrentStatus;
    DRG_DEBUG_LEVEL m_CurrentStatusLevel;

    GUI_Label *m_Label;
    drgFont *m_Font;
};

class drgSelectObject : public drgObjectBase
{
public:
    drgSelectObject(drgObjectBase *object);

    drgObjectBase *m_Object;
};

class GUI_ListItem : public drgObjectList
{
public:
    void SetText(string16 text);
    string16 *GetText();
    void SetIcon(DEFAULT_TEXTURE icon);
    DEFAULT_TEXTURE GetIcon();
    string16 GetItemPath(char concatChar = '.');

    void EnableExpand(bool enable);
    bool IsExpanded();
    void Select(bool select);
    bool IsSelected();
    virtual void SetUserData(drgObjectBase *data);
    virtual drgObjectBase *GetUserData();
    unsigned int GetNumChild();
    GUI_ListItem *GetFirstItem();
    GUI_ListItem *GetNextItem(GUI_ListItem *item);
    GUI_ListItem *AddItem(string16 text, DEFAULT_TEXTURE icon, unsigned int flags);

private:
    GUI_ListItem();
    ~GUI_ListItem();

    void UpdateBBox();
    void Hover(bool hover);

    unsigned int m_FlagsListItem;
    int m_Depth;
    string16 m_Text;
    DEFAULT_TEXTURE m_Icon;
    drgBBox m_BBox;
    GUI_List *m_Owner;
    GUI_ListItem *m_Parent;
    drgObjectBase *m_UserData;
    bool m_IsDragReceiver;

    friend class GUI_List;
};

class GUI_List : public GUI_Widget
{
public:
    GUI_List();
    virtual ~GUI_List();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void ReceiverDraw(GUI_Drawing *draw);
    virtual void UpdateSize();
    virtual void DragDraw(GUI_Drawing *draw);
    virtual void DragDone();

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual bool CallMouseDoubleClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);
    virtual void CallMouseDrag(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    GUI_ListItem *GetSelected();
    void SetSelected(GUI_ListItem *selected);

    bool IsMultiSelect();
    drgObjectList *GetSelectedList();

    unsigned int GetNumChild();
    GUI_ListItem *GetFirstItem();
    GUI_ListItem *GetNextItem(GUI_ListItem *item);
    GUI_ListItem *AddItem(string16 text, DEFAULT_TEXTURE icon, unsigned int flags);
    void RemoveItem(GUI_ListItem *item);
    void RemoveItemAddNext(GUI_ListItem *item, GUI_ListItem *prev);
    void RemoveItemAddChild(GUI_ListItem *item, GUI_ListItem *parent);
    void RemoveAllItems();
    void RemoveItemChildren(GUI_ListItem *item);
    GUI_ListItem *FindParent(GUI_ListItem *list, GUI_ListItem *item);
    string16 GetItemPath(GUI_ListItem *item, char concatChar = '.');
    GUI_ListItem *GetItemByPath(string16 path);
    void ClearList();

    void SetItemExpandCallback(drgCallbackObj callback, void *callbackData)
    {
        m_OnItemExpand = callback;
        m_OnItemExpandData = callbackData;
    }

    GUI_ListItem *GetDragItem()
    {
        return m_DragItem;
    }

    GUI_ListItem *GetDragHoverItem()
    {
        return m_DragHover;
    }

    inline void EnableIcons(bool enable) { enable ? (m_FlagsList |= DRG_LIST_ICONS) : (m_FlagsList &= (~DRG_LIST_ICONS)); };
    inline void EnableTree(bool enable) { enable ? (m_FlagsList |= DRG_LIST_TREE) : (m_FlagsList &= (~DRG_LIST_TREE)); };

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_List"; }

protected:
    bool m_bMultiSelect;
    unsigned int m_FlagsList;
    GUI_LIST_STATE m_State;
    GUI_ListItem m_List;
    GUI_ListItem *m_Selected;
    GUI_ListItem *m_Hover;
    GUI_ListItem *m_DragItem;
    GUI_ListItem *m_DragHover;
    drgObjectList *m_SelectedObjs;
    float m_TimeOfDragStart;

    // Callback info
    drgCallbackObj m_OnItemExpand;
    void *m_OnItemExpandData;

    GUI_ListItem *GetHovered();
    void SetHovered(GUI_ListItem *hovered);

    void CheckForKeyUp(int key);

    void DrawInternal(GUI_ListItem *item, GUI_Drawing *draw, drgBBox *bbox_icon, drgBBox *bbox_txt, drgFont *font, drgColor textColor, drgColor textSelectedColor, drgColor iconcolor, float fontsize, int level);
    void DrawInternalTree(GUI_ListItem *item, GUI_Drawing *draw, drgBBox *bbox_arw, drgBBox *bbox_icon, drgBBox *bbox_txt, drgFont *font, drgColor textColor, drgColor textSelectedColor, drgColor iconcolor, float fontsize, int level);
    void AddBBoxInternal(GUI_ListItem *item, drgBBox *bbox, drgFont *font, float fontsize);
    void AddBBoxInternalTree(GUI_ListItem *item, drgBBox *bbox, drgFont *font, float fontsize);
    GUI_ListItem *GetMouseOverItemInternal(GUI_ListItem *item, drgBBox *bbox, drgFont *font, float fontsize);
    GUI_ListItem *GetMouseOverItemInternalTree(GUI_ListItem *item, drgBBox *bbox, drgFont *font, float fontsize);
    bool RemoveItemInternal(GUI_ListItem *list, GUI_ListItem *item);
    bool GetItemPathInternal(GUI_ListItem *list, string16 *path, GUI_ListItem *item, char concatChar = '.');
    GUI_ListItem *GetItemByPathInternal(GUI_ListItem *list, string16 path);

    GUI_ListItem *GetFirstItemRecursive();
    GUI_ListItem *GetNextItemRecursive(GUI_ListItem *item);

    friend class GUI_Container;
    friend class GUI_ListItem;
};

class GUI_DropListItem : public drgObjectProp
{
public:
    void SetText(string16 text);
    string16 *GetText();

    void Select();
    void SetUserData(drgObjectProp *data);
    drgObjectProp *GetUserData();

private:
    GUI_DropListItem();
    ~GUI_DropListItem();

    string16 m_Text;
    GUI_DropList *m_Owner;
    drgObjectProp *m_UserData;

    friend class GUI_DropList;
};

class GUI_DropList : public GUI_Widget
{
public:
    GUI_DropList();
    virtual ~GUI_DropList();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void UpdateSize();

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual bool CallMouseClick(unsigned int winid, int button);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    GUI_DropListItem *GetSelected();
    int GetSelectedIndex();
    void SetSelected(GUI_DropListItem *selected);
    void SetSelected(int index);

    unsigned int GetNumChild();
    GUI_DropListItem *GetFirstItem();
    GUI_DropListItem *GetNextItem(GUI_DropListItem *item);
    GUI_DropListItem *AddItem(string16 text, unsigned int flags);
    void RemoveItem(GUI_DropListItem *item);
    void ClearList();

    inline bool IsDroped()
    {
        return ((m_State & GUI_DROP_LIST_STATE_DROP) != 0);
    }

    inline bool IsHovered()
    {
        return ((m_State & GUI_DROP_LIST_STATE_HOVER) != 0);
    }

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_DropList"; }

protected:
    unsigned int m_FlagsList;
    unsigned int m_State;
    drgObjectList m_List;
    GUI_Menu *m_SubMenu;
    GUI_DropListItem *m_Selected;

    static void MenuCallback(drgObjectBase *widget, void *data);

    virtual void SetupSubMenu();

    friend class GUI_Container;
    friend class GUI_DropListItem;
};

class GUI_Log : public GUI_Widget
{
public:
    virtual ~GUI_Log();

    void Init(GUI_Container *container);

    void LogMessage(string16 str);
    void LogWarning(string16 str);
    void LogError(string16 str);
    void ClearAllMessages();

    static GUI_Log *CreateChildLog(GUI_Widget *parent, unsigned int flags);

protected:
    GUI_Log();

    void UpdateDebugTextPositioning();

    unsigned int m_Flags;
    drgObjectList *m_LogStrings;
    GUI_Container *m_DebugContainer;

    static const int DEFAULT_GUI_LOG_OFFSET;
    static const float DEFAULT_POS_X;
    static const float DEFAULT_TOP_POS_Y;

    friend class GUI_Container;
};

class GUI_Table : public GUI_Container
{
public:
	struct Row
	{
		float m_Y;
		float m_VariableHeight;
		float m_FixedHeight;
		float m_MinHeight;
		float m_MaxHeight;
		float m_CurrentHeight;
		float m_Padding;

		Row();
	};

	struct Column
	{
		float m_X;
		float m_VariableWidth;
		float m_FixedWidth;
		float m_MinWidth;
		float m_MaxWidth;
		float m_CurrentWidth;
		float m_Padding;

		Column();
	};

	struct Cell
	{
		int m_RowNum;
		int m_ColNum;
		int m_SpansRows;
		int m_SpansCols;
		GUI_Container* m_Container;

		Cell();
	};

	virtual ~GUI_Table();

	// If you pass resizeTable as false, the table will not be expanded, and can't be accessed until ReallocateTable is called.
	void AddRow( float fixedHeight, float variableHeight, float padding = 15.0f, bool resizeTable = true );
	void AddCol( float fixedWidth, float variableWidth, float padding = 15.0f, bool resizeTable = true );

	void RemoveRow( int rowNum, bool resizeTable = true );
	void RemoveCol( int colNum, bool resizeTable = true );
	void RemoveCell( int rowNum, int colNum, bool resizeTable = true );
	void RemoveAllCells();

	void SetRowHeight( int rowNum, float fixedHeight, float variableHeight );
	void SetColumnWidth( int colNum, float fixedWidth, float variableWidth );

	void SetRowMinHeight( int rowNum, float minHeight );
	void SetColumnMinWidth( int colNum, float minWidth );

	void SetRowMaxHeight( int rowNum, float maxHeight );
	void SetColumnMaxWidth( int colNum, float maxWidth );

	Column* GetColumn( int columnNum );
	Row*	GetRow( int rowNum );

	GUI_Container* CreateCell( int rowNum, int colNum, int spansRows = 1, int spansCols = 1 );
	void AddToCell( GUI_Widget* widget, int rowNum, int colNum );
	GUI_Container* GetCell( int rowNum, int colNum );
	
	virtual void Init();
	virtual void Destroy();

	virtual void Draw(GUI_Drawing* draw);
	virtual void UpdateSize();

	virtual const char* GetScriptClass() {return "Vertigo.GUI.GUI_Table";}

	int GetNumRows();
	int GetNumCols();

	float GetSumOfRowHeights();

	void SetupPropertyTable(int nStartRows);

	static const float DEFAULT_PROPERTY_ROW_HEIGHT;
	static const float DEFAULT_PROPERTY_ROW_VAR_HEIGHT;
	static const float DEFAULT_PROPERTY_ROW_PADDING;
	static const float DEFAULT_PROPERTY_COL_WIDTH;
	static const float DEFAULT_PROPERTY_COL_VAR_WIDTH[2];
	static const float DEFAULT_PROPERTY_COL_PADDING;

protected:

	Cell***			m_Cells;	//2D array of widget pointers.
	Row**			m_Rows;
	Column**		m_Columns;
	int				m_NumRows;
	int				m_NumCols;
	int				m_CurrentNumCellRows;	// These are needed to track when we need to resize the table.
	int				m_CurrentNumCellCols;
	bool			m_TableIsDirty;
	
	void ReallocateTable( int numRows, int numCols );
	void ResizeRows( float height );
	void ResizeColumns( float width );
	void ResizeCells();

	float GetCellWidth( Cell* cell );
	float GetCellHeight( Cell* cell );

	virtual void DrawTableOutline( GUI_Drawing* draw );

	GUI_Table();

	friend class GUI_Container;
};


#endif // _DRG_GUI_H_
