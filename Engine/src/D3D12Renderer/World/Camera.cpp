#include "UltimateEnginePCH.h"
#include "Camera.h"

//-------------------------------------------------------------------------------------------------------------------
Camera::Camera() :	m_Position(0, 0, 0),
					m_fPitch(0),
					m_fYaw(0),
					m_fRoll(0),
					m_bForward(false),
					m_bBack(false),
					m_bLeft(false),
					m_bRight(false),
					m_bMoveUp(false),           
					m_bMoveDown(false),         
					m_bMouseFirstClick(false),  
					m_fPrevMouseX(0),
					m_fPrevMouseY(0),
					m_fMouseSensitivity(0.005f),   
					m_fMovementSpeed(5.0f),        
					m_fDamping(0.85f),             
					m_Velocity(0, 0, 0)            
{
	const XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&m_viewMatrix, identity);
}

//-------------------------------------------------------------------------------------------------------------------
Camera::~Camera()
{
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::SetPosition(float x, float y, float z)
{
	m_Position = XMFLOAT3(x, y, z);
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::SetRotation(float pitch, float yaw, float roll)
{
	m_fPitch = pitch;
	m_fYaw = yaw;
	m_fRoll = roll;
}

//-------------------------------------------------------------------------------------------------------------------
XMMATRIX Camera::GetViewMatrix() const
{
	return XMLoadFloat4x4(&m_viewMatrix);
}

//-------------------------------------------------------------------------------------------------------------------
XMMATRIX Camera::GetProjectionMatrix() const
{
	constexpr float fovY = 45.0f;
	const float aspectRatio = static_cast<float>(UT::GLOBALS::GWindowWidth) / UT::GLOBALS::GWindowHeight;
	constexpr float nearZ = 0.1f;
	constexpr float farZ = 100.0f;

	return XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), aspectRatio, nearZ, farZ);
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::OnKeyPressed(UT::GLOBALS::InputAction action)
{
	switch (action)
	{
		case UT::GLOBALS::InputAction::FORWARD: m_bForward = true; break;
		case UT::GLOBALS::InputAction::BACK: m_bBack = true;  break;
		case UT::GLOBALS::InputAction::LEFT: m_bLeft = true;  break;
		case UT::GLOBALS::InputAction::RIGHT: m_bRight = true;  break;
		case UT::GLOBALS::InputAction::UP: m_bMoveUp = true; break;      
		case UT::GLOBALS::InputAction::DOWN: m_bMoveDown = true; break;  
	}
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::OnKeyReleased(UT::GLOBALS::InputAction action)
{
	switch (action)
	{
		case UT::GLOBALS::InputAction::FORWARD: m_bForward = false; break;
		case UT::GLOBALS::InputAction::BACK: m_bBack = false;  break;
		case UT::GLOBALS::InputAction::LEFT: m_bLeft = false;  break;
		case UT::GLOBALS::InputAction::RIGHT: m_bRight = false;  break;
		case UT::GLOBALS::InputAction::UP: m_bMoveUp = false; break;      
		case UT::GLOBALS::InputAction::DOWN: m_bMoveDown = false; break;  
	}
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::OnMouseMove(float x, float y, bool bMouseClicked)
{
	// If mouse button not pressed, just reset the "first click" flag
	if (!bMouseClicked)
	{
		m_bMouseFirstClick = false;  
		return;
	}

	// Initialize previous position on FIRST click only (avoid jitter)
	if (!m_bMouseFirstClick)
	{
		m_fPrevMouseX = x;
		m_fPrevMouseY = y;
		m_bMouseFirstClick = true;

		return;							// Skip first frame rotation
	}

	// Now calculate smooth delta
	const float dx = x - m_fPrevMouseX;
	const float dy = y - m_fPrevMouseY;

	// Use configurable sensitivity instead of hardcoded 0.005f
	m_fYaw += dx * m_fMouseSensitivity;
	m_fPitch -= dy * m_fMouseSensitivity;

	// Clamp pitch to avoid gimbal lock (use 85° instead of 90°)
	constexpr float maxPitch = XM_PIDIV2 * 0.95f;
	m_fPitch = std::clamp(m_fPitch, -maxPitch, maxPitch);

	m_fPrevMouseX = x;
	m_fPrevMouseY = y;
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::Update(double dt)
{
	// Calculate direction vectors
	const XMVECTOR forward = XMVectorSet(
		sinf(m_fYaw) * cosf(m_fPitch),
		sinf(m_fPitch),
		cosf(m_fYaw) * cosf(m_fPitch),
		0.0f
	);

	const XMVECTOR right = XMVector3Cross(forward, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Build desired velocity direction
	XMVECTOR desiredVelocity = XMVectorZero();
	if (m_bForward) desiredVelocity += forward;
	if (m_bBack) desiredVelocity -= forward;
	if (m_bLeft) desiredVelocity += right;
	if (m_bRight) desiredVelocity -= right;
	if (m_bMoveUp) desiredVelocity += up;
	if (m_bMoveDown) desiredVelocity -= up;

	// Normalize and scale by speed
	XMVECTOR velocity = XMLoadFloat3(&m_Velocity);
	desiredVelocity = XMVector3Normalize(desiredVelocity);
	desiredVelocity = desiredVelocity * m_fMovementSpeed;

	// Smooth acceleration towards desired velocity
	velocity = XMVectorLerp(velocity, desiredVelocity, 0.15f);

	// Apply damping if no input (smooth deceleration)
	if (!m_bForward && !m_bBack && !m_bLeft && !m_bRight && !m_bMoveUp && !m_bMoveDown)
		velocity = velocity * m_fDamping;

	// Update position
	XMVECTOR pos = XMLoadFloat3(&m_Position);
	pos += velocity * static_cast<float>(dt);

	XMStoreFloat3(&m_Position, pos);
	XMStoreFloat3(&m_Velocity, velocity);

	// Recalculate view matrix
	const XMVECTOR look = forward;
	const XMMATRIX view = XMMatrixLookToLH(pos, look, up);
	XMStoreFloat4x4(&m_viewMatrix, view);
}

