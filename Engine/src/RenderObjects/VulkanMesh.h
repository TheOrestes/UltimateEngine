#pragma once

#include "vulkan/vulkan.hpp"
#include "../VulkanRenderer/VulkanGlobals.h"
#include "VulkanMeshData.h"

class VulkanDevice;

//---------------------------------------------------------------------------------------------------------------------
class UT_API VulkanMesh
{
public:
	VulkanMesh();
	VulkanMesh(const VulkanDevice* pVulkanDevice, const std::vector<VertexPNTBT>& vertices, const std::vector<uint32_t>& indices);

	void							Cleanup(vk::Device vkDevice);

	~VulkanMesh();

public:
	uint32_t						m_uiVertexCount;
	uint32_t						m_uiIndexCount;

	UT::VkStructs::VulkanBuffer		m_vkVertexBuffer;
	UT::VkStructs::VulkanBuffer		m_vkIndexBuffer;

private:
	void							CreateVertexBuffer(const VulkanDevice* pVulkanDevice, const std::vector<VertexPNTBT>& vertices);
	void							CreateIndexBuffer(const VulkanDevice* pVulkanDevice, const std::vector<uint32_t>& indices);
};

