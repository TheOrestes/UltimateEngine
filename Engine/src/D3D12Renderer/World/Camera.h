#pragma once

#include "D3D12Renderer/D3DGlobals.h"

class Camera
{
public:
    Camera();
    ~Camera();

    void Update(double dt);

    void SetPosition(float x, float y, float z);
    void SetRotation(float pitch, float yaw, float roll);

    XMMATRIX GetViewMatrix();
    XMMATRIX GetProjectionMatrix(float aspect, float nearZ, float farZ) const;

    void	OnKeyPressed(UT::GLOBALS::InputAction action);
    void	OnKeyReleased(UT::GLOBALS::InputAction action);
    void    OnMouseMove(float x, float y, bool bMouseClicked);

private:
    XMFLOAT3 m_Position;
    
    float m_fPitch;
    float m_fYaw;
    float m_fRoll;

    XMMATRIX viewMatrix;

    bool m_bForward, m_bBack, m_bLeft, m_bRight;
    float m_iPrevMouseX, m_iPrevMouseY;
};

