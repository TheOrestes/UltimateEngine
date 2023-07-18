#include "UltimateEnginePCH.h"
#include "../EngineHeader.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::VulkanDevice(const VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::~VulkanDevice()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::SetupDevice(VulkanContext* pRC)
{
	AcquirePhysicalDevice(pRC);
	CreateLogicalDevice(pRC);
	CreateSwapchain(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateCommandPool(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateCommandBuffers(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::HandleWindowsResize(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::Cleanup(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CleanupOnWindowsResize(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateSwapchain(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	// Get the formats
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		m_SwapchainInfo.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_SwapchainInfo.surfaceFormats.data());
	}

	// Presentation modes
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

	if (presentationCount != 0)
	{
		m_SwapchainInfo.surfacePresentModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, m_SwapchainInfo.surfacePresentModes.data());
	}
}

//---------------------------------------------------------------------------------------------------------------------
// List out all the available physical devices. Choose the one which supports required Queue families & extensions. 
// Give preference to Discrete GPU over Integrated GPU!
//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::AcquirePhysicalDevice(VulkanContext* pRC)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(pRC->vkInst, &deviceCount, nullptr);
	UT_ASSERT_BOOL((deviceCount == 0), "Can't find GPUs supporting Vulkan!!!");

	std::vector<VkPhysicalDevice> vecDevices(deviceCount);
	vkEnumeratePhysicalDevices(pRC->vkInst, &deviceCount, vecDevices.data());

	// List out all the physical devices & get their properties
	for (uint16_t i = 0; i < deviceCount; ++i)
	{
		vkGetPhysicalDeviceProperties(vecDevices[i], &m_vkDeviceProps);
		LOG_INFO("{0} Detected", m_vkDeviceProps.deviceName);
	}

	for (uint16_t i = 0; i < deviceCount; ++i)
	{
		bool bExtensionsSupported = CheckDeviceExtensionSupport(vecDevices[i]);

		if (bExtensionsSupported)
		{
			// Fetch Queue families supported!
			FetchQueueFamilies(vecDevices[i], pRC);

			// Fetch if surface has required parameters to create swapchain!
			FetchSwapchainInfo(vecDevices[i], pRC->vkSurface);

			if (m_QueueFamilyIndices.isComplete() && m_SwapchainInfo.isValid())
			{
				vkGetPhysicalDeviceProperties(vecDevices[i], &m_vkDeviceProps);

				// Prefer Discrete GPU over integrated one!
				if (m_vkDeviceProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					pRC->vkPhysicalDevice = vecDevices[i];

					// Get properties of physical device memory
					vkGetPhysicalDeviceMemoryProperties(pRC->vkPhysicalDevice, &(pRC->vkDeviceMemoryProps));

					LOG_DEBUG("{0} Selected!!", m_vkDeviceProps.deviceName);
					LOG_INFO("---------- Device Limits ----------");
					LOG_INFO("Max Color Attachments: {0}", m_vkDeviceProps.limits.maxColorAttachments);
					LOG_INFO("Max Descriptor Set Samplers: {0}", m_vkDeviceProps.limits.maxDescriptorSetSamplers);
					LOG_INFO("Max Descriptor Set Uniform Buffers: {0}", m_vkDeviceProps.limits.maxDescriptorSetUniformBuffers);
					LOG_INFO("Max Framebuffer Height: {0}", m_vkDeviceProps.limits.maxFramebufferHeight);
					LOG_INFO("Max Framebuffer Width: {0}", m_vkDeviceProps.limits.maxFramebufferWidth);
					LOG_INFO("Max Push Constant Size: {0}", m_vkDeviceProps.limits.maxPushConstantsSize);
					LOG_INFO("Max Uniform Buffer Range: {0}", m_vkDeviceProps.limits.maxUniformBufferRange);
					LOG_INFO("Max Vertex Input Attributes: {0}", m_vkDeviceProps.limits.maxVertexInputAttributes);

					break;
				}
				else
				{
					LOG_ERROR("{0} Rejected!!", m_vkDeviceProps.deviceName);
				}
			}
		}
	}

	UT_ASSERT_BOOL((pRC->vkPhysicalDevice == VK_NULL_HANDLE), "Failed to find suitable GPU!!!");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateLogicalDevice(VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::FetchQueueFamilies(VkPhysicalDevice physicalDevice, const VulkanContext* pRC)
{
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	return true;
}
