#pragma once
#include "../Core/Core.h"
#include "VulkanGlobals.h"

#include "Volk/volk.h"

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
	void								FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface);
	void								AcquirePhysicalDevice(VulkanContext* pRC);
	void								CreateLogicalDevice(VulkanContext* pRC);
	void								FetchQueueFamilies(VkPhysicalDevice physicalDevice, const VulkanContext* pRC);
	bool								CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

private:
	VkPhysicalDeviceProperties			m_vkDeviceProps;
	VkPhysicalDeviceFeatures			m_vkDeviceFeaturesAvailable;
	VkPhysicalDeviceFeatures			m_vkDeviceFeaturesEnabled;
	VkPhysicalDeviceMemoryProperties	m_vkDeviceMemoryProps;
	std::vector<VkExtensionProperties>	m_vecSupportedExtensions;

public:
	UT::VK::QueueFamilyIndices			m_QueueFamilyIndices;
	UT::VK::SwapchainInfo				m_SwapchainInfo;
};

