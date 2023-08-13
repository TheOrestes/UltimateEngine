#pragma once

#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"

struct GLFWwindow;

//---------------------------------------------------------------------------------------------------------------------
class UT_API VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

public:
	//-- Shader module
	vk::ShaderModule					CreateShaderModule(const std::string& fileName) const;

	//-- Images
	vk::Format							ChooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags) const;
	void								CreateImage2D(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image* pImage, vk::DeviceMemory* pDeviceMemory);
	void								CreateImageView2D(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView* imageView);
	void								CopyBufferToImage(vk::Buffer srcBuffer, uint32_t width, uint32_t height, vk::Image* image);
	void								TransitionImageLayout(vk::Image srcImage, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer cmdBuffer = nullptr);

	//-- Buffers
	uint32_t							FindMemoryTypeIndex(uint32_t allowedTypeIndex, vk::MemoryPropertyFlags props) const;
	void								CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memFlags, vk::Buffer* outBuffer, vk::DeviceMemory* outMemory);
	void								CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize bufferSize);
	VkCommandBuffer						BeginCommandBuffer() const;
	void								EndAndSubmitCommandBuffer(vk::CommandBuffer commandBuffer) const;

public:
	GLFWwindow*							pWindow;					// Window Handle
	vk::Instance						vkInst;						// Vulkan Instance
	vk::SurfaceKHR						vkSurface;					// Vulkan Surface
	vk::PhysicalDevice					vkPhysicalDevice;			// Vulkan Physical Device
	vk::Device							vkDevice;					// Vulkan Logical Device
	vk::PhysicalDeviceMemoryProperties	vkDeviceMemoryProps;		// Physical device memory properties

	uint16_t							m_uiWindowWidth;			// current window width
	uint16_t							m_uiWindowHeight;			// current window height
	vk::SwapchainKHR					vkSwapchain;				// Vulkan Swapchain handle
	vk::Extent2D						vkSwapchainExtent;			// Swapchain extents
	vk::Format							vkSwapchainImageFormat;		// swapchain image format
	vk::Format							vkDepthImageFormat;			// swapchain depth image format

	vk::Queue							vkQueueGraphics;			// Graphics Queue handle
	vk::Queue							vkQueuePresent;				// Presentation Queue handle

	vk::CommandPool						vkGraphicsCommandPool;
	std::vector<vk::CommandBuffer>		vkListGraphicsCommandBuffers;

	std::vector<vk::Framebuffer>		vkListFramebuffers;
	std::vector<vk::Image>				vkListSwapchainImages;

	vk::Pipeline						vkForwardRenderingPipeline;
	vk::PipelineLayout					vkForwardRenderingPipelineLayout;
	vk::RenderPass						vkForwardRenderingRenderPass;
};
