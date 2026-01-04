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

    void        SetMouseSensitivity(float sensitivity)  { m_fMouseSensitivity = sensitivity; }
    void        SetMovementSpeed(float speed)           { m_fMovementSpeed = speed; }
    void        SetDamping(float damping)               { m_fDamping = damping; }

private:
    Camera();
    ~Camera();

    // ===== POSITION & ROTATION =====
    XMFLOAT3    m_Position;
    float       m_fPitch;
    float       m_fYaw;
    float       m_fRoll;
    XMFLOAT4X4  m_viewMatrix;

    // ===== KEYBOARD STATE: WASD =====
    bool        m_bForward;
    bool        m_bBack;
    bool        m_bLeft;
	bool        m_bRight;

    // ===== KEYBOARD STATE: Q-E =====
    bool        m_bMoveUp;      
    bool        m_bMoveDown;    

    // ===== MOUSE STATE =====
    bool        m_bMouseFirstClick;  // NEW: Prevents jitter on initial click
    float       m_fPrevMouseX;
	float       m_fPrevMouseY;

    // ===== CONFIGURABLE PARAMETERS =====
    float       m_fMouseSensitivity;  
    float       m_fMovementSpeed;       
    float       m_fDamping;

    // ==== VELOCITY FOR INERTIA =====
	XMFLOAT3    m_Velocity;        // For smooth acceleration/deceleration
};

