#include "UltimateEnginePCH.h"
#include "Scene.h"
#include "Camera.h"
#include "../RenderObjects/GameObject.h"
#include "../VulkanRenderer/VulkanDevice.h"
#include "../RenderObjects/VulkanCube.h"

//---------------------------------------------------------------------------------------------------------------------
bool Scene::LoadScene(const VulkanDevice* pDevice)
{
	m_pCamera = new Camera();
	CHECK(LoadModels(pDevice));
}

//---------------------------------------------------------------------------------------------------------------------
void Scene::Cleanup(VulkanDevice* pDevice)
{
	for (GameObject* object : m_ListModels)
	{
		object->Cleanup(reinterpret_cast<void*>(pDevice));
	}
}

//---------------------------------------------------------------------------------------------------------------------
void Scene::Update(float dt) const
{
	m_pCamera->Update(dt);

	for (GameObject* object : m_ListModels)
	{
		if (const VulkanCube* pCube = dynamic_cast<VulkanCube*>(object))
		{
			pCube->Update(m_pCamera, dt);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void Scene::UpdateUniforms(const VulkanDevice* pDevice, uint32_t imageIndex) const
{
	const vk::Device vkDevice = pDevice->GetDevice();

	for (GameObject* object : m_ListModels)
	{
		if (const VulkanCube* pCube = dynamic_cast<VulkanCube*>(object))
		{
			pCube->UpdateUniforms(vkDevice, imageIndex);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void Scene::Render(const VulkanDevice* pDevice, uint32_t imageIndex) const
{
	for (GameObject* object : m_ListModels)
	{
		if (const VulkanCube* pCube = dynamic_cast<VulkanCube*>(object))
		{
			pCube->Render(pDevice, imageIndex);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool Scene::LoadModels(const VulkanDevice* pDevice)
{
	VulkanCube* pCube = new VulkanCube("Cube");
	CHECK(pCube->Initialize(reinterpret_cast<const void*>(pDevice)));
	pCube->SetPosition(glm::vec3(0));
	pCube->SetScale(glm::vec3(1));

	m_ListModels.push_back(pCube);

	return true;
}
