#pragma once

#include "../Core/Core.h"
#include "Volk/volk.h"

struct GLFWwindow;

//---------------------------------------------------------------------------------------------------------------------
class UT_API VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

public:
	GLFWwindow*							pWindow;
	VkInstance							vkInst;
	VkSurfaceKHR						vkSurface;
	VkPhysicalDevice					vkPhysicalDevice;
	VkDevice							vkDevice;
	VkPhysicalDeviceMemoryProperties	vkDeviceMemoryProps;
};
