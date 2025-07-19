#ifndef RENDER_PANE_H
#define RENDER_PANE_H

#include "render/gui/gui.h"

class GUI_RenderPane : public GUI_Widget
{
public:
    GUI_RenderPane();
    virtual ~GUI_RenderPane();

    virtual void Init();
    virtual void Reinit();
    virtual void Destroy();
    virtual void Draw(GUI_Drawing *draw);
    virtual void UpdateSize();

    virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
    virtual bool CallMouseDown(unsigned int winid, int button);
    virtual void CallMouseUp(unsigned int winid, int button);
    virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
    virtual void CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel);

    static GUI_RenderPane *CreateRenderPane(GUI_Widget *parent, unsigned int flags);

private:
    drgTexture *m_Texture;
    bool m_MouseDown;
};

#endif // RENDER_PANE_H