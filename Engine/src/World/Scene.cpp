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
void Scene::Update(double dt) const
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
	constexpr std::array<glm::vec4, 12> randomColors =
	{
		glm::vec4(1, 0, 0, 1),
		glm::vec4(0, 1, 0, 1),
		glm::vec4(0, 0, 1, 1),
		glm::vec4(1, 1, 0, 1),
		glm::vec4(0, 1, 1, 1),
		glm::vec4(0.5,  0.85, 0.65, 1),
		glm::vec4(0.15, 0.15, 0.25, 1),
		glm::vec4(0.85, 0.45, 0.25, 1),
		glm::vec4(0.15, 0.25, 0.15, 1),
		glm::vec4(0.25, 0.50, 0.35, 1),
		glm::vec4(0.55, 0.15, 0.65, 1),
		glm::vec4(0.05, 0.25, 0.95, 1),
	};

	std::random_device rd;
	std::mt19937 rng{ rd() };
	std::uniform_int_distribution<int> gen(0, 11); // uniform, unbiased

	int index = gen(rng);

	// First Cube
	VulkanCube* pCube = new VulkanCube("Cube", randomColors[index]);
	CHECK(pCube->Initialize(reinterpret_cast<const void*>(pDevice)))
	pCube->SetPosition(glm::vec3(1.5f,-3,3));
	pCube->SetRotationAxis(glm::vec3(0, 1, 0));
	pCube->SetRotationAngle(10.0f);
	pCube->SetScale(glm::vec3(1.5f));

	m_ListModels.push_back(pCube);

	// Second Cube
	index = gen(rng);
	VulkanCube* pVerticalCube = new VulkanCube("Vertical Cube", randomColors[index]);
	CHECK(pVerticalCube->Initialize(reinterpret_cast<const void*>(pDevice)))
	pVerticalCube->SetPosition(glm::vec3(-2,-2,1));
	pVerticalCube->SetRotationAxis(glm::vec3(0, 1, 0));
	pVerticalCube->SetRotationAngle(-10.0f);
	pVerticalCube->SetScale(glm::vec3(1.5,3,1.5));

	m_ListModels.push_back(pVerticalCube);

	constexpr float fRoomDimension = 5.0f;

	// Left Wall
	index = gen(rng);
	VulkanCube* pLeftWall = new VulkanCube("Left Wall", randomColors[index]);
	CHECK(pLeftWall->Initialize(reinterpret_cast<const void*>(pDevice)))
	pLeftWall->SetPosition(glm::vec3(-fRoomDimension, 0, 0));
	pLeftWall->SetScale(glm::vec3(0.01, fRoomDimension, fRoomDimension));

	m_ListModels.push_back(pLeftWall);

	// Right wall
	index = gen(rng);
	VulkanCube* pRightWall = new VulkanCube("Right Wall", randomColors[index]);
	CHECK(pRightWall->Initialize(reinterpret_cast<const void*>(pDevice)))
	pRightWall->SetPosition(glm::vec3(fRoomDimension, 0, 0));
	pRightWall->SetScale(glm::vec3(0.01, fRoomDimension, fRoomDimension));

	m_ListModels.push_back(pRightWall);

	// Back wall
	index = gen(rng);
	VulkanCube* pBackWall = new VulkanCube("Back Wall", randomColors[index]);
	CHECK(pBackWall->Initialize(reinterpret_cast<const void*>(pDevice)))
	pBackWall->SetPosition(glm::vec3(0, 0, 0));
	pBackWall->SetScale(glm::vec3(fRoomDimension, fRoomDimension, 0.01));

	m_ListModels.push_back(pBackWall);

	// Top wall
	index = gen(rng);
	VulkanCube* pTopWall = new VulkanCube("Top Wall", randomColors[index]);
	CHECK(pTopWall->Initialize(reinterpret_cast<const void*>(pDevice)))
	pTopWall->SetPosition(glm::vec3(0, fRoomDimension, 0));
	pTopWall->SetScale(glm::vec3(fRoomDimension, 0.01, fRoomDimension));

	m_ListModels.push_back(pTopWall);

	// Bottom plane
	index = gen(rng);
	VulkanCube* pBottomWall = new VulkanCube("Bottom Wall", randomColors[index]);
	CHECK(pBottomWall->Initialize(reinterpret_cast<const void*>(pDevice)))
	pBottomWall->SetPosition(glm::vec3(0, -fRoomDimension, 0));
	pBottomWall->SetScale(glm::vec3(fRoomDimension, 0.01, fRoomDimension));

	m_ListModels.push_back(pBottomWall);

	return true;
}
