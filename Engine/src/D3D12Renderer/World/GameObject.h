#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class GameObject
{
public:
	GameObject();
	virtual			~GameObject();

	void			SetName(const std::string& name);
	void			SetPosition(float x, float y, float z);
	void			SetRotation(float x, float y, float z, float angle);
	void			SetScale(float x, float y, float z);

	virtual void	SetMesh(const std::string& filePath);
	virtual void	SetTexture(const std::string& filePath);
	virtual void	Update(double dt);
	virtual void	Render();

private:
	virtual void	UpdateConstantBuffer();

private:
	std::string		m_strName;

	XMFLOAT3		m_Position;
	XMFLOAT3		m_RotationAxis;
	float			m_fAngle;
	XMFLOAT3		m_Scale;

protected:
	XMFLOAT4X4		m_World;
};
