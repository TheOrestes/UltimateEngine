#include "UltimateEnginePCH.h"
#include "Camera.h"
#include "../Core/Core.h"
#include "../VulkanRenderer/VulkanGlobals.h"

//---------------------------------------------------------------------------------------------------------------------
Camera::Camera()
{
    m_iViewportX = 0;
    m_iViewportY = 0;

    m_iWindowWidth = 0;
    m_iWindowHeight = 0;

    m_fAspect   = UT::VkGlobals::GCurrentResolution.x / UT::VkGlobals::GCurrentResolution.y;
    m_fFOV      = 45.0f;
    m_fNearClip = 0.01f;
    m_fFarClip = 10000.0f;

    m_fCameraScale = 0.01f;
    m_fCameraYaw = 0.0f;
    m_fCameraPitch = 0.0f;

    m_fMaxPitchRate = 0.5f;
    m_fMaxYawRate = 0.5f;

    m_vecCameraPosition = glm::vec3(0, 1, 15);
    m_vecCameraPositionDelta = glm::vec3(0);
    m_vecCameraLookAt = glm::vec3(0, 1, 0);

    m_vecCameraDirection = glm::vec3(1);
    m_vecCameraUp = glm::vec3(0,1,0);

    m_matProjection = glm::mat4(1);
    m_matView = glm::mat4(1);
    m_matModel = glm::mat4(1);
    m_matMVP = glm::mat4(1);

    m_bPrevInitialized = false;
    m_vecPrevMousePos = glm::vec2(0);
    m_vecMousePosition = glm::vec2(UT::VkGlobals::GCurrentResolution.x / 2.0f, UT::VkGlobals::GCurrentResolution.y / 2.0f);
}

//---------------------------------------------------------------------------------------------------------------------
Camera::~Camera()
{
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::Reset()
{
    m_vecCameraUp = glm::vec3(0,1,0);
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::Update(float dt)
{
    m_vecCameraDirection = glm::normalize(m_vecCameraLookAt - m_vecCameraPosition);

    m_matProjection = glm::perspectiveFov(m_fFOV, UT::VkGlobals::GCurrentResolution.x, UT::VkGlobals::GCurrentResolution.y, m_fNearClip, m_fFarClip);

    // determine axis for pitch rotation
    glm::vec3 pitchAxis = glm::cross(m_vecCameraDirection, m_vecCameraUp);

    // compute quaternion for pitch based on camera pitch angle
    glm::quat pitchQuat = glm::angleAxis(m_fCameraPitch, pitchAxis);

    // compute quaternion for yaw based on yaw angle & camera up vector
    glm::quat yawQuat = glm::angleAxis(m_fCameraYaw, m_vecCameraUp);

    // Add the two quaternions
    glm::quat temp = glm::normalize(glm::cross(pitchQuat, yawQuat));

    // update the direction from quaternion
    m_vecCameraDirection = glm::rotate(temp, m_vecCameraDirection);

    // Add the camera delta
    m_vecCameraPosition += m_vecCameraPositionDelta;

    // Set the lookAt to be at the front of camera
    m_vecCameraLookAt = m_vecCameraPosition + m_vecCameraDirection * 1.0f;

    // Damping for smooth camera
    m_fCameraYaw *= dt;
    m_fCameraPitch *= dt;


    // Compute MVP
    m_matView = glm::lookAt(m_vecCameraPosition, m_vecCameraLookAt, m_vecCameraUp);
     m_matModel = glm::mat4(1);
    m_matMVP = m_matProjection * m_matView * m_matModel;
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::Move(CameraAction dir)
{
    switch (dir)
    {
        case CameraAction::CAMERA_UP:
        {
            m_vecCameraPositionDelta = m_vecCameraUp * m_fCameraScale;
            break;
        }
            
        case CameraAction::CAMERA_DOWN:
        {
            m_vecCameraPositionDelta = -m_vecCameraUp * m_fCameraScale;
            break;
        }
            
        case CameraAction::CAMERA_LEFT:
        {
            m_vecCameraPositionDelta = -glm::normalize(glm::cross(m_vecCameraDirection, m_vecCameraUp)) * m_fCameraScale;
            break;
        }
            
        case CameraAction::CAMERA_RIGHT:
        {
            m_vecCameraPositionDelta = glm::normalize(glm::cross(m_vecCameraDirection, m_vecCameraUp)) * m_fCameraScale;
            break;
        }

        case CameraAction::CAMERA_FORWARD:
        {
            m_vecCameraPositionDelta = m_vecCameraDirection * m_fCameraScale;        
            break;
        }
            
        case CameraAction::CAMERA_BACK:
        {
            m_vecCameraPositionDelta = -m_vecCameraDirection * m_fCameraScale;
            break;
        }
    }

    LOG_INFO("Camera Pos: [{0},{1},{2}] | Camera Dir: [{3},{4},{5}] ", m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z, 
                                                                        m_vecCameraDirection.x, m_vecCameraDirection.y, m_vecCameraDirection.z);
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::Stop()
{
    m_vecCameraPositionDelta = glm::vec3(0);
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::ChangePitch(float degrees)
{
    //Check bounds with the max pitch rate so that we aren't moving too fast
    if (degrees < -m_fMaxPitchRate) 
    {
        degrees = -m_fMaxPitchRate;
    }
    else if (degrees > m_fMaxPitchRate) 
    {
        degrees = m_fMaxPitchRate;
    }

    m_fCameraPitch += degrees;

    //Check bounds for the camera pitch
    if (m_fCameraPitch > 360.0f) 
    {
        m_fCameraPitch -= 360.0f;
    }
    else if (m_fCameraPitch < -360.0f)
    {
        m_fCameraPitch += 360.0f;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::ChangeYaw(float degrees)
{
    //Check bounds with the max heading rate so that we aren't moving too fast
    if (degrees < -m_fMaxYawRate) 
    {
        degrees = -m_fMaxYawRate;
    }
    else if (degrees > m_fMaxYawRate)
    {
        degrees = m_fMaxYawRate;
    }
    //This controls how the heading is changed if the camera is pointed straight up or down
    //The heading delta direction changes
    if (m_fCameraPitch > 90 && m_fCameraPitch < 270 || (m_fCameraPitch < -90 && m_fCameraPitch > -270))
    {
        m_fCameraYaw -= degrees;
    }
    else 
    {
        m_fCameraYaw += degrees;
    }
    //Check bounds for the camera heading
    if (m_fCameraYaw > 360.0f)
    {
        m_fCameraYaw -= 360.0f;
    }
    else if (m_fCameraYaw < -360.0f)
    {
        m_fCameraYaw += 360.0f;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void Camera::Move2D(const glm::vec2& pos, bool isButtonClicked)
{
    if (!m_bPrevInitialized)
    {
        m_vecPrevMousePos = pos;
        m_bPrevInitialized = true;
        return;
    }

    //compute the mouse delta from the previous mouse position
    glm::vec2 mouse_delta = m_vecPrevMousePos - pos;

    //if the camera is moving, meaning that the mouse was clicked and dragged, change the pitch and heading
    if (isButtonClicked) 
    {
        ChangeYaw(.001f * mouse_delta.x);
        ChangePitch(.001f * mouse_delta.y);
    }

    m_vecPrevMousePos = pos;
}

