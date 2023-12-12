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

	bool									SetupDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	void									RecreateOnWindowResize();
	void									Cleanup();
	void									CleanupOnWindowsResize();
	void									CreateCommandBuffers(const VulkanFramebuffer* pFrameBuffer);

private:
	bool									AcquirePhysicalDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	bool									CreateLogicalDevice();

	bool									CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions);
	bool									CheckDeviceExtensionSupport() const;
	void									FetchQueueFamilies(vk::SurfaceKHR vkSurface);
	uint32_t								FindMemoryTypeIndex(uint32_t allowedTypeIndex, vk::MemoryPropertyFlags props) const;

public:
	inline bool								IsQueueSharing() const							{ return (m_QueueFamilyIndices.graphicsFamily == m_QueueFamilyIndices.presentFamily); }
	inline uint32_t							GetGraphicsQueueFamilyIndex() const				{ return m_QueueFamilyIndices.graphicsFamily.value(); }
	inline uint32_t							GetPresentQueueFamilyIndex()  const				{ return m_QueueFamilyIndices.presentFamily.value();  }
	inline vk::CommandBuffer			    GetGraphicsCommandBuffer(uint32_t index) const	{ return m_vkListGraphicsCommandBuffers[index]; }
	inline vk::Queue						GetGraphicsQueue() const						{ return m_vkQueueGraphics; }
	inline vk::Queue						GetPresentQueue() const							{ return m_vkQueuePresent; }
	inline vk::Device						GetDevice()	const								{ return m_vkDevice; }
	inline vk::PhysicalDevice				GetPhysicalDevice() const						{ return m_vkPhysicalDevice;  }
	inline uint16_t							GetSwapchainImageCount() const					{ return static_cast<uint32_t>(m_vkListGraphicsCommandBuffers.size()); }

public:
	vk::ShaderModule						CreateShaderModule(const std::string& fileName) const;
	vk::Format								ChooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags) const;
	void									CopyBufferToImage(vk::Buffer srcBuffer, uint32_t width, uint32_t height, vk::Image* image) const;
	void									CreateImage2D(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::ImageAspectFlags aspectFlags, UT::VkStructs::VulkanImage* pOutImage2D) const;
	void									CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memFlags, UT::VkStructs::VulkanBuffer* pOutBuffer) const;
	void									CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize bufferSize) const;
	void									BeginGraphicsCommandBuffer(uint32_t imageIndex, vk::CommandBufferBeginInfo cmdBufferBeginInfo) const;
	void									EndGraphicsCommandBuffer(uint32_t imageIndex) const;
	void									BeginRenderPass(uint32_t imageIndex, vk::RenderPassBeginInfo renderPassInfo) const;
	void									EndRenderPass(uint32_t imageIndex) const;
	vk::CommandBuffer						BeginTransferCommandBuffer() const;
	void									EndAndSubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer) const;
	void									BindPipeline(uint32_t imageIndex, vk::PipelineBindPoint bindPoint, vk::Pipeline pipeline) const;
	void									TransitionImageLayout(vk::Image srcImage, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer cmdBuffer) const;

private:
	vk::Device								m_vkDevice;
	vk::PhysicalDevice						m_vkPhysicalDevice;
	vk::Queue								m_vkQueueGraphics;
	vk::Queue								m_vkQueuePresent;
	vk::CommandPool							m_vkGraphicsCommandPool;
	std::vector<vk::CommandBuffer>			m_vkListGraphicsCommandBuffers;
	QueueFamilyIndices						m_QueueFamilyIndices;	
};

