#include "UltimateEnginePCH.h"
#include "VulkanMesh.h"
#include "../VulkanRenderer/VulkanDevice.h"
#include "../VulkanRenderer/VulkanGlobals.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanMesh::~VulkanMesh()
{
	
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanMesh::VulkanMesh(const VulkanDevice* pVulkanDevice, const std::vector<VertexPNTBT>& vertices, const std::vector<uint32_t>& indices)
{
	m_uiVertexCount = vertices.size();
	m_uiIndexCount = indices.size();

	CreateVertexBuffer(pVulkanDevice, vertices);
	CreateIndexBuffer(pVulkanDevice, indices);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::Cleanup(vk::Device vkDevice)
{
	m_vkVertexBuffer.DestroyAll(vkDevice);
	m_vkIndexBuffer.DestroyAll(vkDevice);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::CreateVertexBuffer(const VulkanDevice* pVulkanDevice, const std::vector<VertexPNTBT>& vertices)
{
	// Get the size of buffer needed for vertices
	const VkDeviceSize bufferSize = m_uiVertexCount * sizeof(VertexPNTBT);
	
	// Create buffer & allocate memory to it!
	UT::VkStructs::VulkanBuffer srcBuffer;
	pVulkanDevice->CreateBuffer(bufferSize,
								vk::BufferUsageFlagBits::eTransferSrc,
								vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
								&srcBuffer);

	// Map memory to Vertex buffer
	const vk::Device vkDevice = pVulkanDevice->GetDevice();

	void* data = vkDevice.mapMemory(srcBuffer.deviceMemory, 0, bufferSize);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkDevice.unmapMemory(srcBuffer.deviceMemory);

	// Now, Create buffer with TRANSFER_DST_BIT to make as recipient of data (also VERTEX_BUFFER_BIT)
	// this time, buffer memory is set to DEVICE_LOCAL which means, it's on the GPU. 
	pVulkanDevice->CreateBuffer( bufferSize,
								 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
								 vk::MemoryPropertyFlagBits::eDeviceLocal,
								 &m_vkVertexBuffer);

	// Copy staging buffer data to vertex buffer on GPUP using Command buffer!
	pVulkanDevice->CopyBuffer(srcBuffer.buffer, m_vkVertexBuffer.buffer, bufferSize);
	
	// Destroy source/staging buffer!
	srcBuffer.DestroyAll(vkDevice);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::CreateIndexBuffer(const VulkanDevice* pVulkanDevice, const std::vector<uint32_t>& indices)
{
	// Get size of buffer needed for indices
	const VkDeviceSize bufferSize = m_uiIndexCount * sizeof(uint32_t);

	// Temporary buffer to "stage" index data before transferring to GPU
	UT::VkStructs::VulkanBuffer srcBuffer;
	pVulkanDevice->CreateBuffer(bufferSize,
								vk::BufferUsageFlagBits::eTransferSrc,
								vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
								&srcBuffer);

	// Map memory to Index buffer
	const vk::Device vkDevice = pVulkanDevice->GetDevice();

	void* data = vkDevice.mapMemory(srcBuffer.deviceMemory, 0, bufferSize);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vkDevice.unmapMemory(srcBuffer.deviceMemory);

	// Create buffer for index data on GPU access only area
	pVulkanDevice->CreateBuffer(bufferSize,
								vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
								vk::MemoryPropertyFlagBits::eDeviceLocal,
								&m_vkIndexBuffer);

	// Copy staging buffer data to vertex buffer on GPUP using Command buffer!
	pVulkanDevice->CopyBuffer(srcBuffer.buffer, m_vkIndexBuffer.buffer, bufferSize);

	// Destroy source/staging buffer!
	srcBuffer.DestroyAll(vkDevice);
}





