
#include "game/render_pane.h"
#include "render/gui/gui_draw.h"
#include "render/render_cache.h"
#include "render/texture.h"
#include "core/input.h"
#include "svo_renderer.h"

drgSVORenderer svo_render;
drgCamera drgcamera;
unsigned int *drgoctree = NULL;
unsigned int node_count;

GUI_RenderPane::GUI_RenderPane()
{
    m_MouseDown = false;
    m_Texture = NULL;
    m_WidgetType = GUI_TYPE_RENDER_PANE;
}

GUI_RenderPane::~GUI_RenderPane()
{
    Destroy();
}

void GUI_RenderPane::Init()
{
    m_PadTop = 0.0f;
    m_PadBot = 0.0f;
    m_PadLeft = 0.0f;
    m_PadRight = 0.0f;

    drgoctree = get_octree_test(&node_count);
    drgcamera.Init(1.0472f, 800.0f / 600.0f, 0.1f, 5.0f, CAMERA_MODE::CAMERA_ORBITAL);
    // drgcamera.position = drgVec3(1.5,1.5,-1.5);
    drgcamera.position = drgVec3(0, 0, 5);
    drgcamera.target = drgVec3(1.5f, 1.5f, 1.5f);
    drgcamera.up = drgVec3(0.0f, 1.0f, 0.0f);
    printf("Camera: fov=%f, aspect=%f, near=%f, far=%f\n",
           drgcamera.fov, drgcamera.aspect, drgcamera.near_plane, drgcamera.far_plane);
    printf("Position: (%f, %f, %f), Target: (%f, %f, %f), Up: (%f, %f, %f)\n",
           drgcamera.position.x, drgcamera.position.y, drgcamera.position.z,
           drgcamera.target.x, drgcamera.target.y, drgcamera.target.z,
           drgcamera.up.x, drgcamera.up.y, drgcamera.up.z);

    // Ensure matrices are updated
    drgcamera.UpdateProjection();
    drgcamera.UpdateView();
}

void GUI_RenderPane::Reinit()
{
}

void GUI_RenderPane::Destroy()
{
}

void GUI_RenderPane::Draw(GUI_Drawing *draw)
{
    if (IsHidden())
    {
        return;
    }

    unsigned int width = (m_View.maxv.x - m_View.minv.x);
    unsigned int height = (m_View.maxv.y - m_View.minv.y);
    if ((width != 0) && (height != 0))
    {
        unsigned int svo_idx = 0;
        unsigned char *pixels_out = (unsigned char *)m_Texture->m_Dib->GetPixelPtr(0, 0);
        unsigned char *pixels_in = (unsigned char *)svo_render.getbuffer();
        unsigned int idx = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x, idx += 4)
            {
                if ((x < 800) && (y < 600))
                {
                    svo_idx = ((y * 800) + x) * 4;
                    pixels_out[idx] = pixels_in[svo_idx];
                    pixels_out[idx + 1] = pixels_in[svo_idx + 1];
                    pixels_out[idx + 2] = pixels_in[svo_idx + 2];
                    pixels_out[idx + 3] = pixels_in[svo_idx + 3];
                }
                else
                {
                    int red = DRG_CLAMP_NORM(((float)x) / 800.0f) * 255.0f;
                    int blue = DRG_CLAMP_NORM(((float)y) / 800.0f) * 255.0f;
                    pixels_out[idx] = red;
                    pixels_out[idx + 1] = 0x00;
                    pixels_out[idx + 2] = blue;
                    pixels_out[idx + 3] = 0xFF;
                }
            }
        }
        m_Texture->UpdateData(false);
    }

    svo_render.render(drgcamera);

    short top, bot, left, right;
    draw->GetRenderCache()->GetClipRect(&top, &bot, &left, &right);
    SetClipRectWithOffsets(draw, top, bot, left, right);

    draw->DrawRect(this, m_View, &m_Color, m_Texture);

    draw->GetRenderCache()->SetClipRect(top, bot, left, right);

    GUI_Widget::Draw(draw);
}

void GUI_RenderPane::UpdateSize()
{
    m_View = m_Sens;

    unsigned int width = (m_View.maxv.x - m_View.minv.x);
    unsigned int height = (m_View.maxv.y - m_View.minv.y);
    if ((width == 0) || (height == 0))
    {
        return;
    }

    if (m_Texture == NULL)
    {
        drgColor col(128, 128, 128, 255);
        m_Texture = drgTexture::CreateTexture("GUI_RenderPane", DRG_FMT_A8R8G8B8, width, height, &col);
    }
    else if ((width != m_Texture->GetWidth()) || (height != m_Texture->GetHeight()))
    {
        m_Texture->m_Dib->Init(1, width, height);
        unsigned char *pixels = (unsigned char *)m_Texture->m_Dib->GetPixelPtr(0, 0);
        unsigned int idx = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x, idx += 4)
            {
                int red = DRG_CLAMP_NORM(((float)x) / 800.0f) * 255.0f;
                int blue = DRG_CLAMP_NORM(((float)y) / 800.0f) * 255.0f;
                pixels[idx] = red;
                pixels[idx + 1] = 0x00;
                pixels[idx + 2] = blue;
                pixels[idx + 3] = 0xFF;
            }
        }
        m_Texture->UpdateData(true);
    }
}

void GUI_RenderPane::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
}

bool GUI_RenderPane::CallMouseDown(unsigned int winid, int button)
{
 	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

   if (button == DRG_MOUSE_BUTTON_LEFT)
    {
        m_MouseDown = true;
    }
    return true;
}

void GUI_RenderPane::CallMouseUp(unsigned int winid, int button)
{
    if (button == DRG_MOUSE_BUTTON_LEFT)
    {
        m_MouseDown = false;
    }
}

void GUI_RenderPane::CallMouseWheel(unsigned int winid, float pos, float rel)
{
    drgcamera.UpdateMouseScroll(rel);
}

void GUI_RenderPane::CallMouseMove(unsigned int winid, drgVec2 *pos, drgVec2 *rel)
{
    if (m_MouseDown)
    {
        drgcamera.UpdateMouseMove(rel->x, rel->y);
    }
}

GUI_RenderPane *GUI_RenderPane::CreateRenderPane(GUI_Widget *parent, unsigned int flags)
{
    GUI_RenderPane *render_pane = new GUI_RenderPane();
    render_pane->UseAsScriptObject();
    render_pane->Init();
    render_pane->SetParent(parent);
    render_pane->m_FlagsWidget |= flags;
    render_pane->m_FlagsWidget |= DRG_WIDGET_RESIZE_EXPAND;
    render_pane->UpdateSize();
    return render_pane;
}
