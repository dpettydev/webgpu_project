
#include "core/system.h"
#include "game/drago.h"

const unsigned int kScreenWidth = 800;
const unsigned int kScreenHeight = 600;

static void HandleButtonCB(drgObjectBase *obj, void *data)
{
    GUI_Button *button = (GUI_Button *)obj;
    GUI_Container *container = (GUI_Container *)button->GetParent();
    if (button->GetState() == DRG_BUTTON_STATE_DOWN)
    {
        drgPrintOut("Button Down!\r\n");
    }
}

Drago Drago::m_Instance;
void DragoAppInterfaceInit()
{
    Drago::GetApp().Init();
}
void DragoAppInterfaceCleanUp()
{
    Drago::GetApp().CleanUp();
}
void DragoAppInterfaceUpdate(double dtime)
{
    Drago::GetApp().Update(dtime);
}
AppInterfaceNative g_drago_app(DragoAppInterfaceInit, DragoAppInterfaceCleanUp, DragoAppInterfaceUpdate);

void Drago::Init()
{
    m_VoxelWindow = GUI_Window::CreateBaseWindow("Drago Engine", kScreenWidth, kScreenHeight);
    m_VoxelWindow->SetMinWidth(640.0f);
    m_VoxelWindow->SetMinHeight(360.0f);
    m_VoxelWindow->SetMaxWidth(2560.0f);
    m_VoxelWindow->SetMaxHeight(1480.0f);
    m_VoxelWindow->SetColor(32, 32, 64);
    m_VoxelWindow->AddFlagsWin(DRG_WINDOW_NOBACKGROUND);

    m_RenderPane = GUI_RenderPane::CreateRenderPane(m_VoxelWindow, DRG_NO_FLAGS);
	m_VoxelWindow->AddChild(m_RenderPane);

    m_FpsLabel = m_VoxelWindow->CreateChildLabel("FPS:", DRG_NO_FLAGS);
    m_FpsLabel->SetPos(10, 40);

    GUI_Window *test_win = m_VoxelWindow->CreateChildWindow("Test Window", 200, 200, DRG_WINDOW_CHILD);
    test_win->SetPos(50, 50);

    GUI_Button *button1 = test_win->CreateChildButton(ICON_CHECK, ICON_CHECK_UP, ICON_CHECK_DOWN, HandleButtonCB, NULL, DRG_BUTTON_STICKY);
    button1->SetPos(50, 50);
    GUI_Button *button2 = test_win->CreateChildButton(ICON_ARRW_LEFT, ICON_ARRW_LEFT_UP, ICON_ARRW_LEFT_DOWN, HandleButtonCB, NULL, DRG_BUTTON_STICKY);
    button2->SetPos(50, 80);
    GUI_Button *button3 = test_win->CreateChildButton("Select", 100, 30, HandleButtonCB, NULL, 0);
    button3->SetPos(100, 50);

    //     // Set up camera
    //     m_Camera.Init(1.0472f, 800.0f / 600.0f, 0.1f, 1000.0f, CAMERA_MODE::CAMERA_FREE);
    //     m_Camera.position = drgVec3(5.0f, 5.0f, 5.0f);
    //     m_Camera.target = drgVec3(2.0f, 2.0f, 2.0f);
    //     m_Camera.UpdateView();
    //     m_Camera.UpdateProjection();
    //     m_Camera.projection.print("m_Camera projection");

    //     m_Camera.Init(1.0472f, 800.0f / 600.0f, 0.1f, 5.0f, CAMERA_MODE::CAMERA_ORBITAL);
    //     // m_Camera.position = drgVec3(1.5,1.5,-1.5);
    //     m_Camera.position = drgVec3(0, 0, 5);
    //     m_Camera.target = drgVec3(1.5f, 1.5f, 1.5f);
    //     m_Camera.up = drgVec3(0.0f, 1.0f, 0.0f);
    //     printf("Camera: fov=%f, aspect=%f, near=%f, far=%f\n",
    //            m_Camera.fov, m_Camera.aspect, m_Camera.near_plane, m_Camera.far_plane);
    //     printf("Position: (%f, %f, %f), Target: (%f, %f, %f), Up: (%f, %f, %f)\n",
    //            m_Camera.position.x, m_Camera.position.y, m_Camera.position.z,
    //            m_Camera.target.x, m_Camera.target.y, m_Camera.target.z,
    //            m_Camera.up.x, m_Camera.up.y, m_Camera.up.z);

    //     // Ensure matrices are updated
    //     m_Camera.UpdateView();
    //     m_Camera.UpdateProjection();

    //     // Set up camera and screen
    //     // setup_camera(&camera, m_Camera.position, m_Camera.target);

    //     drgCamera::SetDefaultCamera(&m_Camera);
}

void Drago::Update(double dtime)
{
    char fps_st[32];
    sprintf(fps_st, "FPS: %i", (int)drgEngine::GetFPS());
    m_FpsLabel->SetText(fps_st);

    //     m_Camera.ProcessMovement(dtime);
}

void Drago::CleanUp()
{
}
