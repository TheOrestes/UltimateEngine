#include "UltimateEnginePCH.h"
#include "GameObject.h"

//-------------------------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
	m_Position = XMFLOAT3(0, 0, 0);
	m_RotationAxis = XMFLOAT3(0, 1, 0);
	m_fAngle = 0.0f;
	m_Scale = XMFLOAT3(1, 1, 1);

	const XMMATRIX Identity = XMMatrixIdentity();
	XMStoreFloat4x4(&m_World, Identity);
}

//-------------------------------------------------------------------------------------------------------------------
GameObject::~GameObject()
{
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::Cleanup()
{
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::SetName(const std::string& name)
{
	m_strName = name;
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::SetPosition(float x, float y, float z)
{
	m_Position = XMFLOAT3(x, y, z);
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::SetRotation(float x, float y, float z, float angle)
{
	m_RotationAxis = XMFLOAT3(x, y, z);
	m_fAngle = angle;
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::SetScale(float x, float y, float z)
{
	m_Scale = XMFLOAT3(x, y, z);
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::SetMesh(const std::string& filePath)
{
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::SetTexture(const std::string& filePath)
{
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::Update(double dt)
{
	// Load from storage format
	const XMVECTOR position = XMLoadFloat3(&m_Position);
	const XMVECTOR rotAxis = XMLoadFloat3(&m_RotationAxis);
	const XMVECTOR scale = XMLoadFloat3(&m_Scale);

	// Perform SIMD calculations
	XMMATRIX world = XMMatrixScaling(scale.m128_f32[0], scale.m128_f32[1], scale.m128_f32[2]);
	world *= XMMatrixRotationAxis(rotAxis, m_fAngle);
	world *= XMMatrixTranslationFromVector(position);

	// Store back to member variable
	XMStoreFloat4x4(&m_World, world);

	UpdateConstantBuffer();
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::Render()
{
}

//-------------------------------------------------------------------------------------------------------------------
void GameObject::UpdateConstantBuffer()
{
}
