
#ifndef _GUI_STYLE_H_
#define _GUI_STYLE_H_

#include "render/gui/gui.h"
#include "render/gui/gui_draw.h"


class GUI_Style
{
public:
	GUI_Style();
	virtual ~GUI_Style();

	virtual void InitDefault(bool forceInit = false);
	virtual void SetBorderWidth(float width = 1.0f);
	virtual float GetBorderWidth();
	virtual float GetCollapsedHeight();
	virtual void Window(GUI_Window *win);
	virtual void Label(GUI_Label *label, drgFont *font, bool selected = false);
	virtual void Titlebar(GUI_TitleBar *tbar, GUI_Label *label, bool pressed);
	virtual void ButtonBackground(GUI_Button *btn, bool pressed);
	virtual void IconFill(GUI_Widget *wid, DEFAULT_TEXTURE icon);
	virtual void MenuBackground(GUI_Menu *menu);
	virtual void MenuItemBackground(GUI_MenuItem *item, bool pressed);
	virtual void MenuItemSeparator(GUI_MenuItem *item);
	virtual void DropListBackground(GUI_DropList *list, GUI_DropListItem *selected);
	virtual void CollapseButtonIcon(drgBBox bbox, bool collapsed = false, bool hover = false);
	virtual void TextboxBackground(GUI_Input *ibox);
	virtual void TextboxText(GUI_Input *ibox, unsigned short *text, int startpos, int endpos, int offset, int cursr, bool focused, int selectionStart, int selectionEnd);
	virtual void Frame(GUI_Container *contain);
	virtual void ScrollbarVert(GUI_Scroll *scrollbar, int state, float min, float max);
	virtual void ScrollbarHoriz(GUI_Scroll *scrollbar, int state, float min, float max);
	virtual void Divider(GUI_DivContainer *div, bool moving);
	virtual void TabBar(GUI_TabContainer *tab);
	virtual void ReceieverTab(GUI_TabContainer *tab);
	virtual void Border(GUI_Container *contain);
	virtual void ProgressBar(GUI_ProgressBar *progressbar, int state, float nProgress);
	virtual void StatusBarBackground(GUI_StatusBar *statusBar, DRG_DEBUG_LEVEL statusLevel = DRG_DEBUG_LEVEL_DEFAULT);
	virtual void Slider(GUI_Slider *slider, int state, float minArea, float maxArea, float minSlide, float maxSlide, int barHeight = 1);
	virtual void HoverTextBox(GUI_Widget *widget, string16 text, drgVec2 mouseCursorPosition);
	virtual void CollapsedTitle(GUI_Widget *widget, GUI_TitleBar *titlebar, string16 *text, GUI_Button *collapseButton);
	// void(*CheckboxButton)(void *, int state, int size);
	// void(*ConsoleBackground)(void *, AG_Color bg);
	// void(*FixedPlotterBackground)(void *, int showAxis, Uint32 yOffs);
	// void(*NotebookBackground)(void *, AG_Rect r);
	// void(*NotebookTabBackground)(void *, AG_Rect r, int idx, int isSelected);
	// void(*PaneVertDivider)(void *, int x, int y, int w, int isMoving);
	// void(*RadioGroupBackground)(void *, AG_Rect r);
	// void(*RadioButton)(struct ag_radio *, int x, int y, int selected, int over);
	// void(*ProgressBarBackground)(void *);
	// void(*SliderBackgroundHoriz)(void *);
	// void(*SliderBackgroundVert)(void *);
	// void(*SliderControlHoriz)(void *, int, int);
	// void(*SliderControlVert)(void *, int, int);
	// void(*SeparatorHoriz)(struct ag_separator *);
	// void(*SeparatorVert)(struct ag_separator *);
	// void(*TableBackground)(void *, AG_Rect r);
	// void(*TableColumnHeaderBackground)(void *, int idx, AG_Rect r, int isSelected);
	// void(*TableSelectedColumnBackground)(void *, int idx, AG_Rect r);
	// void(*TableRowBackground)(void *, AG_Rect r, int isSelected);
	// void(*ListBackground)(void *, AG_Rect r);
	// void(*ListItemBackground)(void *, AG_Rect r, int isSelected);
	// void(*TreeSubnodeIndicator)(void *, AG_Rect r, int isExpanded);

	void ViewBox(GUI_Widget *widget, drgColor *col);
	void SensBox(GUI_Widget *widget, drgColor *col);

	inline drgColor GetDefaultColor(DEFAULT_COLOR color)
	{
		return m_DefaultColors[color];
	}

	inline drgTexture *GetDefaultIcon(DEFAULT_TEXTURE icon)
	{
		return m_DefaultIcon[icon];
	}

	virtual inline drgFont *GetDefaultFont()
	{
		return m_DefaultFont;
	}

	virtual inline drgFont *GetInputFont()
	{
		return m_DefaultFont;
	}

	inline void SetDrawInfo(GUI_Drawing *draw)
	{
		m_DrawInfo = draw;
	}

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Style"; }

protected:
	drgColor m_DefaultColors[LAST_COLOR];
	drgFont *m_DefaultFont;
	GUI_Drawing *m_DrawInfo;
	bool m_Initialized;
	float m_BorderWidth;

	static float m_CursorTimer;
	static drgTexture *m_DefaultIcon[ICON_LAST];

	virtual void InitDefaultColors();
	virtual void InitDefaultIcons();
	virtual void InitDefaultFont(const char *fontFileRelativePath, const char *fontName);

	friend class GUI_Drawing;
};

#endif // _GUI_STYLE_H_
