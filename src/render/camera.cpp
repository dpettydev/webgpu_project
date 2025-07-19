
#include "render/camera.h"
#include <GLFW/glfw3.h>
#include <cmath>

drgCamera *drgCamera::m_DefaultCamera = NULL;

drgCamera::drgCamera(float fov_, float aspect_, float near_plane_, float far_plane_, CAMERA_MODE mode_)
    : fov(fov_), aspect(aspect_), near_plane(near_plane_), far_plane(far_plane_), mode(mode_),
      position(0.0f, 0.0f, 5.0f), target(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f),
      yaw(-1.5708f), pitch(0.0f), distance(5.0f), move_speed(5.0f), mouse_sensitivity(0.005f),
      first_mouse(true), last_x(0.0), last_y(0.0)
{
    UpdateView();
    UpdateProjection();
}

void drgCamera::Init(float fov_, float aspect_, float near_plane_, float far_plane_, CAMERA_MODE mode_)
{
    fov = fov_;
    aspect = aspect_;
    near_plane = near_plane_;
    far_plane = far_plane_;
    mode = mode_;
    position = drgVec3{0.0f, 0.0f, 5.0f};
    target = drgVec3{0.0f, 0.0f, 0.0f};
    up = drgVec3{0.0f, 1.0f, 0.0f};
    yaw = -1.5708f; // -90 degrees
    pitch = 0.0f;
    distance = 5.0f;
    move_speed = 5.0f;
    mouse_sensitivity = 0.005f;
    first_mouse = true;
    UpdateView();
    UpdateProjection();
}

void drgCamera::UpdateView()
{
    if (mode == CAMERA_MODE::CAMERA_FREE)
    {
        // Calculate forward direction from yaw and pitch
        drgVec3 forward;
        forward.x = cos(pitch) * cos(yaw);
        forward.y = sin(pitch);
        forward.z = cos(pitch) * sin(yaw);
        view.LookAt(position, position + forward, up);
    }
    else if (mode == CAMERA_MODE::CAMERA_FOLLOW)
    {
        // Position is offset from target
        position = target + drgVec3{0.0f, 2.0f, distance};
        view.LookAt(position, target, up);
    }
    else if (mode == CAMERA_MODE::CAMERA_ORBITAL)
    {
        // Calculate position based on yaw, pitch, and distance
        position.x = target.x + distance * cos(pitch) * cos(yaw);
        position.y = target.y + distance * sin(pitch);
        position.z = target.z + distance * cos(pitch) * sin(yaw);
        view.LookAt(position, target, up);
    }
    projection_view = view * projection;
}

void drgCamera::UpdateProjection()
{
    projection.Perspective(fov, aspect, near_plane, far_plane);
    projection_view = view * projection;
}

// Process keyboard input for movement (call per frame)
void drgCamera::ProcessMovement(float delta_time)
{
    if (mode == CAMERA_MODE::CAMERA_FREE)
    {
        drgVec3 forward = {cos(pitch) * cos(yaw), sin(pitch), cos(pitch) * sin(yaw)};
        drgVec3 right = {cos(yaw + 1.5708f), 0.0f, sin(yaw + 1.5708f)};
        float velocity = move_speed * delta_time;

        // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //     position += forward * velocity;
        // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //     position -= forward * velocity;
        // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //     position -= right * velocity;
        // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //     position += right * velocity;
    }
    else if (mode == CAMERA_MODE::CAMERA_ORBITAL)
    {
        // float velocity = move_speed * delta_time;
        // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //     distance = fmax(1.0f, distance - velocity);
        // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //     distance += velocity;
    }
    UpdateView();
}

// GLFW mouse callback
void drgCamera::UpdateMouseMove(double xoffset, double yoffset)
{
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    if (mode == CAMERA_MODE::CAMERA_FREE || mode == CAMERA_MODE::CAMERA_ORBITAL)
    {
        yaw += xoffset;
        pitch = fmin(fmax(pitch + yoffset, -1.57f), 1.57f); // Clamp pitch
        UpdateView();
    }
}

// scroll callback for zooming
void drgCamera::UpdateMouseScroll(double offset)
{
    if (mode == CAMERA_MODE::CAMERA_ORBITAL)
    {
        distance = fmax(1.0f, distance - offset * 0.5f);
        UpdateView();
    }
    else if (mode == CAMERA_MODE::CAMERA_FOLLOW)
    {
        distance = fmax(1.0f, distance - offset * 0.5f);
        UpdateView();
    }
}

// Set target position (for follow/orbital)
void drgCamera::SetTarget(const drgVec3 &new_target)
{
    target = new_target;
    UpdateView();
}
