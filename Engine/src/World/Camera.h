// Based on https://github.com/hmazhar/moderngl_camera

#pragma once

#include "../Core/Core.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//---------------------------------------------------------------------------------------------------------------------
enum class CameraAction
{
    CAMERA_NONE,
    CAMERA_FORWARD,
    CAMERA_BACK,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN,
    CAMERA_PAN_2D,
    CAMERA_CLICK
};

//---------------------------------------------------------------------------------------------------------------------
class UT_API Camera 
{
public:
    Camera();
    ~Camera();

    void            Update(float dt);

    void            Reset();
    void            Move(CameraAction dir);
    void            Stop();
    void            ChangePitch(float degrees);
    void            ChangeYaw(float degrees);

    void            Move2D(const glm::vec2& pos, bool isButtonClicked);               // Change the Yaw-Pitch of the camera based on the 2D movement of the Mouse!

    int             m_iViewportX;
    int             m_iViewportY;

    int             m_iWindowWidth;
    int             m_iWindowHeight;

    float           m_fAspect;
    float           m_fFOV;
    float           m_fNearClip;
    float           m_fFarClip;             

    float           m_fCameraScale;
    float           m_fCameraYaw;
    float           m_fCameraPitch;

    float           m_fMaxPitchRate;
    float           m_fMaxYawRate;

    glm::vec3       m_vecCameraPosition;
    glm::vec3       m_vecCameraPositionDelta;
    glm::vec3       m_vecCameraLookAt;
    glm::vec3       m_vecCameraDirection;
    glm::vec3       m_vecCameraUp;
                                                  
    glm::mat4       m_matProjection;
    glm::mat4       m_matView;
    glm::mat4       m_matModel;
    glm::mat4       m_matMVP;

private:

    bool            m_bPrevInitialized;
    glm::vec2       m_vecPrevMousePos;
    glm::vec2       m_vecMousePosition;

    Camera(const Camera&);
};

