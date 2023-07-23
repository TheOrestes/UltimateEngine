#pragma once
#include "../Core/Core.h"
#include "VulkanGlobals.h"
#include "vulkan/vulkan.hpp"

struct VulkanContext;

class UT_API VulkanDevice
{
public:
	VulkanDevice(const VulkanContext* pRC);
	~VulkanDevice();

	void								SetupDevice(VulkanContext* pRC);
	void								CreateCommandPool(VulkanContext* pRC);
	void								CreateCommandBuffers(VulkanContext* pRC);
	void								HandleWindowsResize(VulkanContext* pRC);
	void								Cleanup(VulkanContext* pRC);
	void								CleanupOnWindowsResize(VulkanContext* pRC);

private:
	void								CreateSwapchain(VulkanContext* pRC);
	vk::SurfaceFormatKHR				ChooseBestSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
	vk::PresentModeKHR					ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	vk::Extent2D						ChooseSwapExtent(const VulkanContext* pContext);
	void								FetchSwapchainInfo(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	void								AcquirePhysicalDevice(VulkanContext* pRC);
	void								CreateLogicalDevice(VulkanContext* pRC);

	void								FetchQueueFamilies(vk::PhysicalDevice physicalDevice, const VulkanContext* pRC);
	bool								CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice);

private:
	vk::PhysicalDeviceProperties		m_vkDeviceProps;
	vk::PhysicalDeviceFeatures			m_vkDeviceFeaturesAvailable;
	vk::PhysicalDeviceFeatures			m_vkDeviceFeaturesEnabled;
	std::vector<vk::ExtensionProperties>m_vecSupportedExtensions;

public:
	UT::VULKAN::QueueFamilyIndices		m_QueueFamilyIndices;
	UT::VULKAN::SwapchainInfo			m_SwapchainInfo;
};

