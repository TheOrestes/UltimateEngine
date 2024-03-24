#pragma once

#include "../Core/Core.h"
#include "IObject.h"
#include "glm/glm.hpp"

class UT_API GameObject : public IObject
{
public:
	GameObject() = default;

	GameObject(const std::string& name) :
		m_vecPosition(0.0f),
		m_vecRotationAxis(0, 1, 0),
		m_vecScale(1),
		m_fRotation(0.0f),
		m_strName(name) {}

	virtual ~GameObject() override = default;

	virtual bool		Initialize(const void*) override;
	virtual void		Cleanup(void*) override;

	inline void			SetPosition(const glm::vec3& _pos)		{ m_vecPosition = _pos; }
	inline void			SetRotationAxis(const glm::vec3& _axis)	{ m_vecRotationAxis = _axis; }
	inline void			SetRotationAngle(float _angle)			{ m_fRotation = _angle; }
	inline void			SetScale(const glm::vec3& _scale)		{ m_vecScale = _scale; }

public:
	inline std::string	getName() const							{ return m_strName; }
	inline void			setName(const std::string& name)		{ m_strName = name; }

protected:
	// Transformations!
	glm::vec3			m_vecPosition;
	glm::vec3			m_vecRotationAxis;
	glm::vec3			m_vecScale;
	float				m_fRotation;

private:
	std::string			m_strName;
};
