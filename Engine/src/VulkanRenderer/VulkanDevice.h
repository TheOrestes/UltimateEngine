#pragma once

#include "VulkanGlobals.h"

class VulkanRenderer;
class VulkanFramebuffer;
class VulkanSwapchain;

//---------------------------------------------------------------------------------------------------------------------
struct QueueFamilyIndices
{
	QueueFamilyIndices()
	{
		graphicsFamily.reset();
		presentFamily.reset();
	}

	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool UT_API isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

//---------------------------------------------------------------------------------------------------------------------
class UT_API VulkanDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

	void									SetupDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	void									RecreateOnWindowResize();
	void									Cleanup();
	void									CleanupOnWindowsResize();
	void									CreateCommandBuffers(const VulkanFramebuffer* pFrameBuffer);

	VkCommandBuffer							BeginCommandBuffer();
	void									EndAndSubmitCommandBuffer(vk::CommandBuffer commandBuffer);
	void									CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize bufferSize);

private:
	void									AcquirePhysicalDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	void									CreateLogicalDevice();

public:
	inline bool								IsQueueSharing() const							{ return (m_QueueFamilyIndices.graphicsFamily == m_QueueFamilyIndices.presentFamily); }
	inline uint32_t							GetGraphicsQueueFamilyIndex() const				{ return m_QueueFamilyIndices.graphicsFamily.value(); }
	inline uint32_t							GetPresentQueueFamilyIndex()  const				{ return m_QueueFamilyIndices.presentFamily.value();  }
	inline vk::CommandBuffer			    GetGraphicsCommandBuffer(uint32_t index) const	{ return m_vkListGraphicsCommandBuffers[index]; }
	inline vk::Queue						GetGraphicsQueue() const						{ return m_vkQueueGraphics; }
	inline vk::Queue						GetPresentQueue() const							{ return m_vkQueuePresent; }
	inline vk::Device						GetDevice()	const								{ return m_vkDevice; }
	inline vk::PhysicalDevice				GetPhysicalDevice() const						{ return m_vkPhysicalDevice;  }

private:
	vk::Device								m_vkDevice;
	vk::PhysicalDevice						m_vkPhysicalDevice;
	vk::Queue								m_vkQueueGraphics;
	vk::Queue								m_vkQueuePresent;
	vk::CommandPool							m_vkGraphicsCommandPool;
	std::vector<vk::CommandBuffer>			m_vkListGraphicsCommandBuffers;
	QueueFamilyIndices						m_QueueFamilyIndices;
};

