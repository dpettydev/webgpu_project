#ifndef DRAGO_DRAGO_H
#define DRAGO_DRAGO_H

#include "render/camera.h"
#include "render/gui/gui.h"
#include "game/render_pane.h"

class Drago
{
public:
    virtual void Init();
    virtual void Update(double dtime);
    virtual void CleanUp();

    static Drago& GetApp()
    {
        return m_Instance;
    }
private:
    drgCamera m_Camera;
    GUI_Window* m_VoxelWindow;
    GUI_Label* m_FpsLabel;
    GUI_RenderPane* m_RenderPane;

    static Drago m_Instance;

};

#endif // DRAGO_DRAGO_H