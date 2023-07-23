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
	GLFWwindow*							pWindow;					// Window Handle
	vk::Instance						vkInst;						// Vulkan Instance
	vk::SurfaceKHR						vkSurface;					// Vulkan Surface
	vk::PhysicalDevice					vkPhysicalDevice;			// Vulkan Physical Device
	vk::Device							vkDevice;					// Vulkan Logical Device
	vk::PhysicalDeviceMemoryProperties	vkDeviceMemoryProps;		// Physical device memory properties

	uint32_t							uiNumSwapchainImages;		// Number of swapchain images
	vk::SwapchainKHR					vkSwapchain;				// Vulkan Swapchain handle
	vk::Extent2D						vkSwapchainExtent;			// Swapchain extents
	vk::Format							vkSwapchainImageFormat;		// swapchain image format
	vk::Format							vkDepthImageFormat;			// swapchain depth image format

	vk::Queue							vkQueueGraphics;			// Graphics Queue handle
	vk::Queue							vkQueuePresent;				// Presentation Queue handle
};
