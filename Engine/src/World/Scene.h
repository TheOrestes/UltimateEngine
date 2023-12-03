#pragma once

#include "../Core/Core.h"

class VulkanDevice;
class GameObject;

class UT_API Scene
{
public:
	Scene() = default;
	~Scene() = default;

	bool								LoadScene(const VulkanDevice* pDevice);
	void								Cleanup(VulkanDevice* pDevice);

	void								Update(float dt) const;
	void								UpdateUniforms(const VulkanDevice* pDevice, uint32_t imageIndex) const;
	void								Render(const VulkanDevice* pDevice, uint32_t imageIndex) const;

public:
	inline GameObject* GetFirstObject() const { return m_ListModels[0]; }
	//inline Camera* GetCamera()	const { return m_pCamera; }

private:
	bool								LoadModels(const VulkanDevice* pDevice);

public:
	std::vector <GameObject*>			m_ListModels;
	//Camera*							m_pCamera;

};

