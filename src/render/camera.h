#ifndef DRAGO_CAMERA_H
#define DRAGO_CAMERA_H

#include "util/math_def.h"

// Camera modes
enum class CAMERA_MODE
{
    CAMERA_FREE,   // Free movement with mouse look
    CAMERA_FOLLOW, // Follows a target at a fixed offset
    CAMERA_ORBITAL // Orbits around a target
};

// drgCamera class: Supports free, follow, and orbital camera configurations
class drgCamera
{
public:
    drgVec3 position;         // Camera position in world space
    drgVec3 target;           // Target point for follow/orbital cameras
    drgVec3 up;               // Up vector for orientation
    float fov;                // Field of view in radians
    float aspect;             // Aspect ratio (width/height)
    float near_plane;         // Near clipping plane
    float far_plane;          // Far clipping plane
    drgMat44 view;            // View matrix
    drgMat44 projection;      // Projection matrix
    drgMat44 projection_view; // Projection view matrix
    CAMERA_MODE mode;         // Camera mode
    float yaw;                // Yaw angle (radians)
    float pitch;              // Pitch angle (radians)
    float distance;           // Distance from target (for follow/orbital)
    float move_speed;         // Movement speed for free/orbital
    float mouse_sensitivity;  // Mouse sensitivity for rotation
    bool first_mouse;         // Flag for first mouse input
    double last_x, last_y;    // Last mouse position

    drgCamera(float fov_ = 1.0472f, float aspect_ = 1.7778f, float near_plane_ = 0.1f, float far_plane_ = 1000.0f, CAMERA_MODE mode_ = CAMERA_MODE::CAMERA_FREE);

    void Init(float fov_, float aspect_, float near_plane_, float far_plane_, CAMERA_MODE mode_);
    void UpdateView();
    void UpdateProjection();
    void ProcessMovement(float dtime);
    void UpdateMouseMove(double xpos, double ypos);
    void UpdateMouseScroll(double offset);
    void SetTarget(const drgVec3 &new_target);

    inline drgMat44 *GetViewProjectionMatrix() 
    {
        return &projection_view;
    }

    static void SetDefaultCamera(drgCamera *camera)
    {
        m_DefaultCamera = camera;
    }

    static drgCamera &GetDefaultCamera()
    {
        return *m_DefaultCamera;
    }

protected:
    static drgCamera *m_DefaultCamera;
};

#endif // DRAGO_CAMERA_H