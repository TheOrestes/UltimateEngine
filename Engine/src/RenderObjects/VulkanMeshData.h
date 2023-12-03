#pragma once

#include "..\VulkanRenderer\VulkanGlobals.h"
#include "..\VulkanRenderer\VulkanDevice.h"

//---------------------------------------------------------------------------------------------------------------------
struct MeshUniformData
{
	MeshUniformData()
	{
		matWorld = glm::mat4(1);
		matView = glm::mat4(1);
		matProjection = glm::mat4(1);
	}

	// Transformation data...
	alignas(64) glm::mat4 matWorld;
	alignas(64) glm::mat4 matView;
	alignas(64) glm::mat4 matProjection;

	// Material data...
	alignas(16)	glm::vec4	albedoColor;
	alignas(16) glm::vec4	emissionColor;
	alignas(16) glm::vec3	hasTextureAEN;
	alignas(16) glm::vec3	hasTextureRMO;
	alignas(4)	float		occlusion;
	alignas(4)	float		roughness;
	alignas(4)	float		metalness;
};

//---------------------------------------------------------------------------------------------------------------------
struct MeshUniformDataBuffer
{
	MeshUniformDataBuffer()
	{
		listBuffers.clear();
	}

	inline UT_API void	CreateUniformDataBuffers(const VulkanDevice* pDevice)
	{
		vk::DeviceSize bufferSize = sizeof(MeshUniformData);

		// one uniform buffer for each swapchain/command buffer
		uint16_t count = pDevice->GetSwapchainImageCount();

		for (uint16_t i = 0; i < count; i++)
		{
			UT::VkStructs::VulkanBuffer buffer;
			pDevice->CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				&buffer);

			listBuffers.emplace_back(buffer);
		}
	}

	inline UT_API void	Cleanup(const VulkanDevice* pDevice)
	{
		for (uint16_t i = 0; i < pDevice->GetSwapchainImageCount(); i++)
		{
			listBuffers[i].DestroyAll(pDevice->GetDevice());
		}
	}

	inline UT_API void	CleanupOnWindowsResize(const VulkanDevice* pDevice)
	{

	}

	MeshUniformData								shaderData;
	std::vector<UT::VkStructs::VulkanBuffer>	listBuffers;
};

//-----------------------------------------------------------------------------------------------------------------------
// VERTEX STRUCTURES
struct VertexPC
{
	VertexPC() : Position(glm::vec3(0)), Color(glm::vec3(1)) {}
	VertexPC(const glm::vec3& pos, const glm::vec3& col) : Position(pos), Color(col) {}

	glm::vec3 Position;
	glm::vec3 Color;
};

struct VertexPNTBT
{
	VertexPNTBT() { Position = glm::vec3(0); Normal = glm::vec3(0); Tangent = glm::vec3(0); BiNormal = glm::vec3(0); UV = glm::vec2(0); }
	VertexPNTBT(const glm::vec3 _pos, const glm::vec3 _normal, const glm::vec3 _tangent, const glm::vec3& _binormal, const glm::vec2& _uv) :
		Position(_pos),
		Normal(_normal),
		Tangent(_tangent),
		BiNormal(_binormal),
		UV(_uv) {}

	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec3 BiNormal;
	glm::vec2 UV;
};
