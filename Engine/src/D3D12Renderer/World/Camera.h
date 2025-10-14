#pragma once

#include "D3D12Renderer/D3DGlobals.h"

class Camera
{
public:
    //Singleton access
    static Camera& GetInstance()
    {
        static Camera instance;
        return instance;
    }

    // Delete copy-constructor & assignment operator
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    
    void        Update(double dt);

    void        SetPosition(float x, float y, float z);
    void        SetRotation(float pitch, float yaw, float roll);

    XMMATRIX    GetViewMatrix() const;
    XMMATRIX    GetProjectionMatrix() const;

    void	    OnKeyPressed(UT::GLOBALS::InputAction action);
    void	    OnKeyReleased(UT::GLOBALS::InputAction action);
    void        OnMouseMove(float x, float y, bool bMouseClicked);

private:
    Camera();
    ~Camera();

    XMFLOAT3    m_Position;
    
    float       m_fPitch;
    float       m_fYaw;
    float       m_fRoll;

    XMFLOAT4X4  m_viewMatrix;

    bool        m_bForward, m_bBack, m_bLeft, m_bRight;
    float       m_fPrevMouseX, m_fPrevMouseY;
};

