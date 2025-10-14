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
					m_fPrevMouseX(0),
					m_fPrevMouseY(0)
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
	}
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::OnMouseMove(float x, float y, bool bMouseClicked)
{
	if (!bMouseClicked)
	{
		m_fPrevMouseX = UT::GLOBALS::GWindowWidth / 2.0f;
		m_fPrevMouseY = UT::GLOBALS::GWindowHeight / 2.0f;

		return;
	}

	const int dx = x - m_fPrevMouseX;
	const int dy = y - m_fPrevMouseY;

	m_fYaw += dx * 0.005f;
	m_fPitch -= dy * 0.005f;

	// Clamp pitch to avoid flipping
	if (m_fPitch > XM_PIDIV2)  m_fPitch = XM_PIDIV2;
	if (m_fPitch < -XM_PIDIV2) m_fPitch = -XM_PIDIV2;

	m_fPrevMouseX = x;
	m_fPrevMouseY = y;
}

//-------------------------------------------------------------------------------------------------------------------
void Camera::Update(double dt)
{
	constexpr float speed = 5.0f;

	const XMVECTOR forward = XMVectorSet(
		sinf(m_fYaw) * cosf(m_fPitch),  // x
		sinf(m_fPitch),                // y
		cosf(m_fYaw) * cosf(m_fPitch),  // z
		0.0f
	);

	const XMVECTOR right = XMVector3Cross(forward, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMVECTOR pos = XMLoadFloat3(&m_Position);

	// Handle Input & Movement
	const float movement = static_cast<float>(speed * dt);

	if (m_bForward)
		pos += forward * movement;
	if (m_bBack)
		pos -= forward * movement;
	if (m_bLeft)
		pos += right * movement;
	if (m_bRight)
		pos -= right * movement;

	XMStoreFloat3(&m_Position, pos);

	// Recalculate & store view matrix!
	const XMVECTOR look = XMVectorSet(
										sinf(m_fYaw) * cosf(m_fPitch),
										sinf(m_fPitch),
										cosf(m_fYaw) * cosf(m_fPitch),
										0.0f
									);
	
	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	const XMMATRIX view = XMMatrixLookToLH(pos, look, up);
	XMStoreFloat4x4(&m_viewMatrix, view);
}

