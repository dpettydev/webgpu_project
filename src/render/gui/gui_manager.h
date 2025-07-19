#ifndef _GUI_MANAGER_H_
#define _GUI_MANAGER_H_

#include "core/object.h"

// Forward Declarations
class GUI_Widget;
class GUI_Window;
class GUI_Container;
class drgVec2;
class GUI_Style;
class GUI_Drawing;

// Class Declarations
class GUI_Manager : public drgObjectProp
{
public:    // Members
protected: // Members
    GUI_Widget *m_DragWidget;
    GUI_Widget *m_SelectedWidget;
    GUI_Widget *m_FocusedWidget;
    GUI_Widget *m_HoverWidget; // top-layer mouseover
    GUI_Window *m_CurrentModal;
    GUI_Window *m_DragWindow;
    GUI_Window *m_DragWidgetPreviousParent;
    GUI_Widget *m_ReceiverWidget;
    drgVec2 m_DragAnchorOffset;

    bool m_HasBeenSetAsScriptObject;
    float m_TimeSinceHoverWidgetMouseEnter;
    float m_TimeSinceLastClick;
    float m_DoubleClickTimer;
    int m_DoubleClickMouseButton;
    unsigned int m_DoubleClickWinID;
    bool m_ClickStarted;
    bool m_ClickFinished;
    int m_TopLayer;
    int m_TopWindowLayer;

    int m_NumWindows;
    GUI_Window **m_Windows;

public: // Methods
    GUI_Manager();
    ~GUI_Manager();

    void Init();
    void Destroy();

    void AddWindow(GUI_Window *window);
    bool HasWindow(GUI_Window *window);
    void RemoveWindow(GUI_Window *window);

    GUI_Window *GetBaseWindow();

    GUI_Window *CreateNativeWindow(string16 *title, int width, int height, drgVec2 *pos = NULL, unsigned int windowFlags = 0, GUI_Style *style = NULL);

    void DestroyNativeWindow(GUI_Window *window);

    void Draw();
    void Update(float deltaTime);

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
    virtual void CallResize(unsigned int winid, drgVec2 *size);

    void SetSelectedWidget(GUI_Widget *widget);
    void SetFocusedWidget(GUI_Widget *widget);
    void SetHoverWidget(GUI_Widget *widget);
    void SetDragWidget(GUI_Widget *widget);
    void SetReceiverWidget(GUI_Widget *widget);
    void SetModalWindow(GUI_Window *window);
    inline void SetTopLayer(int layer) { m_TopLayer = layer; }
    inline void SetTopWindowLayer(int layer) { m_TopWindowLayer = layer; }

    inline bool HasSelectedWidget() { return m_SelectedWidget != NULL; }
    inline bool HasDragWidget() { return m_DragWidget != NULL; }
    inline bool HasFocusedWidget() { return m_FocusedWidget != NULL; }
    inline bool HasHoverWidget() { return m_HoverWidget != NULL; }
    inline bool HasReceiverWidget() { return m_ReceiverWidget != NULL; }
    inline bool HasModalWindow() { return m_CurrentModal != NULL; }

    GUI_Widget *GetDragWidget();
    GUI_Widget *GetFocusedWidget();
    GUI_Widget *GetSelectedWidget();
    GUI_Widget *GetHoverWidget();
    GUI_Widget *GetReceiverWidget();
    GUI_Window *GetModalWindow();
    GUI_Window *GetDragWindow();
    inline int GetTopLayer() { return m_TopLayer; }
    inline int GetTopWindowLayer() { return m_TopWindowLayer; }
    void DrawHoverText(GUI_Drawing *draw);

    virtual void UseAsScriptObjectLazy(); // because it must occur after engine init and dll load

    virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Manager"; }

protected: // Methods
    void AddEvents();
    void RemoveEvents();

    // Events
    void OnDragDone();
    void OnDragStart(GUI_Widget *widget);

    void UpdateHoverTimer(float deltaTime);
    void UpdateDoubleClickTimer(float deltaTime);
    void StartDoubleClick(unsigned int winid, int button);
    void CheckDoubleClick(unsigned int winid, int button);

    GUI_Window *CreateNewDragWindow(GUI_Widget *dragWidget, drgVec2 *globalPos);
};

#endif