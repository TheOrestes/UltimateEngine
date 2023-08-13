#include "UltimateEnginePCH.h"
#include "../EngineHeader.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

#include "GLFW/glfw3.h"

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
	vk::CommandPoolCreateInfo poolInfo = {};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value();

	pRC->vkGraphicsCommandPool = pRC->vkDevice.createCommandPool(poolInfo);

	LOG_INFO("Graphics command pool created");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateCommandBuffers(VulkanContext* pRC)
{
	if (pRC->vkListFramebuffers.empty())
		LOG_ERROR("Framebuffers needs to be created before creating command buffers!");

	// Make sure we have command buffer for each framebuffer!
	pRC->vkListGraphicsCommandBuffers.resize(pRC->vkListFramebuffers.size());

	// Allocate buffer from the Graphics command pool
	vk::CommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.commandPool = pRC->vkGraphicsCommandPool;
	cbAllocInfo.level = vk::CommandBufferLevel::ePrimary;
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(pRC->vkListGraphicsCommandBuffers.size());

	pRC->vkListGraphicsCommandBuffers = pRC->vkDevice.allocateCommandBuffers(cbAllocInfo);

	LOG_INFO("Graphics command buffer created");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::HandleWindowsResize(VulkanContext* pRC)
{
	CleanupOnWindowsResize(pRC);

	// re-create swapchain
	CreateSwapchain(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::Cleanup(VulkanContext* pRC)
{
	pRC->vkDevice.destroySwapchainKHR(pRC->vkSwapchain);
	pRC->vkDevice.destroy();	
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CleanupOnWindowsResize(VulkanContext* pRC)
{
	pRC->vkDevice.destroySwapchainKHR(pRC->vkSwapchain);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateSwapchain(VulkanContext* pRC)
{
	// 1. Choose best surface format
	vk::SurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat(m_SwapchainInfo.surfaceFormats);

	// 2. Choose best presentation format
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(m_SwapchainInfo.surfacePresentModes);

	// 3. Choose Swapchain image resolution
	vk::Extent2D extent = ChooseSwapExtent(pRC);

	// decide how many images to have in the swap chain, it's good practice to have an extra count.
	// Also make sure it does not exceed maximum number of images
	uint32_t minImageCount = m_SwapchainInfo.surfaceCapabilities.minImageCount;
	minImageCount = minImageCount + 1;
	if (m_SwapchainInfo.surfaceCapabilities.maxImageCount > 0 && minImageCount > m_SwapchainInfo.surfaceCapabilities.maxImageCount)
	{
		minImageCount = m_SwapchainInfo.surfaceCapabilities.maxImageCount;
	}

	// Swapchain creation info
	vk::SwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.surface = pRC->vkSurface;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.minImageCount = minImageCount;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapchainCreateInfo.preTransform = m_SwapchainInfo.surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainCreateInfo.clipped = true;

	if (m_QueueFamilyIndices.graphicsFamily != m_QueueFamilyIndices.presentFamily)
	{
		uint32_t indices[] =
		{
			m_QueueFamilyIndices.graphicsFamily.value(),
			m_QueueFamilyIndices.presentFamily.value()
		};

		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = indices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	pRC->vkSwapchain = pRC->vkDevice.createSwapchainKHR(swapchainCreateInfo);

	// Save this for later purposes. 
	pRC->vkSwapchainImageFormat = surfaceFormat.format;
	pRC->vkSwapchainExtent = extent;

	LOG_INFO("Vulkan Swapchain Created!");
}

//---------------------------------------------------------------------------------------------------------------------
vk::SurfaceFormatKHR VulkanDevice::ChooseBestSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	// If only 1 format available and is undefined, then this means ALL formats are available (no restrictions)
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
	{
		return { vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};
	}

	// If restricted, search for optimal format
	for (const auto& format : availableFormats)
	{
		if ((format.format == vk::Format::eR8G8B8A8Srgb || format.format == vk::Format::eB8G8R8A8Srgb)
			&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return availableFormats[0];
}

//---------------------------------------------------------------------------------------------------------------------
vk::PresentModeKHR VulkanDevice::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	// out of all Mailbox allows triple buffering, so if available use it, else use FIFO mode.
	for (uint32_t i = 0; i < availablePresentModes.size(); ++i)
	{
		if (availablePresentModes[i] == vk::PresentModeKHR::eMailbox)
		{
			return availablePresentModes[i];
		}
	}

	return vk::PresentModeKHR::eFifo;
}

//---------------------------------------------------------------------------------------------------------------------
vk::Extent2D VulkanDevice::ChooseSwapExtent(const VulkanContext* pContext)
{
	// Get the surface capabilities for a given device
	m_SwapchainInfo.surfaceCapabilities = pContext->vkPhysicalDevice.getSurfaceCapabilitiesKHR(pContext->vkSurface);

	// The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the 
	// resolution of the window that we're drawing to.The range of the possible resolutions is defined in the 
	// VkSurfaceCapabilitiesKHR structure.Vulkan tells us to match the resolution of the window by setting the 
	// width and height in the currentExtent member.However, some window managers do allow us to differ here 
	// and this is indicated by setting the width and height in currentExtent to a special value : the maximum 
	// value of uint32_t. In that case we'll pick the resolution that best matches the window within the 
	// minImageExtent and maxImageExtent bounds.
	if (m_SwapchainInfo.surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return m_SwapchainInfo.surfaceCapabilities.currentExtent;
	}
	else
	{
		// To handle window resize properly, query current width-height of framebuffer, instead of global value!
		int width, height;
		glfwGetFramebufferSize(pContext->pWindow, &width, &height);

		//VkExtent2D actualExtent = { App::WIDTH, App::HEIGHT };
		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::clamp(actualExtent.width, m_SwapchainInfo.surfaceCapabilities.minImageExtent.width,
			m_SwapchainInfo.surfaceCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, m_SwapchainInfo.surfaceCapabilities.minImageExtent.height,
			m_SwapchainInfo.surfaceCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::FetchSwapchainInfo(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	// Get the formats
	m_SwapchainInfo.surfaceFormats = device.getSurfaceFormatsKHR(surface);
	
	// Presentation modes
	m_SwapchainInfo.surfacePresentModes = device.getSurfacePresentModesKHR(surface);
}

//---------------------------------------------------------------------------------------------------------------------
// List out all the available physical devices. Choose the one which supports required Queue families & extensions. 
// Give preference to Discrete GPU over Integrated GPU!
//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::AcquirePhysicalDevice(VulkanContext* pRC)
{
	uint32_t deviceCount = 0;
	
	std::vector<vk::PhysicalDevice> physicalDevices = pRC->vkInst.enumeratePhysicalDevices();

	UT_ASSERT_BOOL((!physicalDevices.empty()), "Can't find GPUs supporting Vulkan!!!");

	// List out all the physical devices & get their properties
	vk::PhysicalDevice chosenDevice = nullptr;
	std::vector<vk::PhysicalDevice>::iterator iter = physicalDevices.begin();
	for (; iter != physicalDevices.end(); ++iter)
	{
		LOG_INFO("{0} Detected", (*iter).getProperties().deviceName);

		// Prefer Discrete GPU over integrated one!
		if ((*iter).getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			chosenDevice = (*iter);
			m_vkDeviceProps = (*iter).getProperties();
		}
		else
		{
			LOG_WARNING("{0} device rejected since it's not a Discrete GPU", (*iter).getProperties().deviceName);
		}
	}

	UT_ASSERT_NULL(chosenDevice, "Failed to find suitable GPU!!!");

	// Check if Discrete GPU we found has all the needed extension support!
	bool bExtensionsSupported = CheckDeviceExtensionSupport(chosenDevice);

	if (bExtensionsSupported)
	{
		// Fetch Queue families supported!
		FetchQueueFamilies(chosenDevice, pRC);

		// Fetch if surface has required parameters to create swapchain!
		FetchSwapchainInfo(chosenDevice, pRC->vkSurface);

		if (m_QueueFamilyIndices.isComplete() && m_SwapchainInfo.isValid())
		{
			pRC->vkPhysicalDevice = chosenDevice;

			// Get properties of physical device memory
			pRC->vkDeviceMemoryProps = pRC->vkPhysicalDevice.getMemoryProperties();

			LOG_INFO("{0} Selected!!", m_vkDeviceProps.deviceName);
			LOG_DEBUG("---------- Device Limits ----------");
			LOG_DEBUG("Max Color Attachments: {0}", m_vkDeviceProps.limits.maxColorAttachments);
			LOG_DEBUG("Max Descriptor Set Samplers: {0}", m_vkDeviceProps.limits.maxDescriptorSetSamplers);
			LOG_DEBUG("Max Descriptor Set Uniform Buffers: {0}", m_vkDeviceProps.limits.maxDescriptorSetUniformBuffers);
			LOG_DEBUG("Max Framebuffer Height: {0}", m_vkDeviceProps.limits.maxFramebufferHeight);
			LOG_DEBUG("Max Framebuffer Width: {0}", m_vkDeviceProps.limits.maxFramebufferWidth);
			LOG_DEBUG("Max Push Constant Size: {0}", m_vkDeviceProps.limits.maxPushConstantsSize);
			LOG_DEBUG("Max Uniform Buffer Range: {0}", m_vkDeviceProps.limits.maxUniformBufferRange);
			LOG_DEBUG("Max Vertex Input Attributes: {0}", m_vkDeviceProps.limits.maxVertexInputAttributes);
			LOG_DEBUG("----------------------------------");
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateLogicalDevice(VulkanContext* pRC)
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

	// std::set allows only One unique value for input values, no duplicate is allowed, so if both Graphics Queue family
	// and Presentation Queue family index is same then it will avoid the duplicates and assign only one queue index!
	std::set<uint32_t> uniqueQueueFamilies =
	{
		m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value()
	};

	float queuePriority = 1.0f;

	for (uint32_t queueFamily = 0; queueFamily < uniqueQueueFamilies.size(); ++queueFamily)
	{
		// Queue the logical device needs to create & the info to do so!
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information needed to create logical device!
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(UT::VULKAN::DeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = UT::VULKAN::DeviceExtensions.data();

	// Physical device features that logical device will use...
	vk::PhysicalDeviceFeatures deviceFeatures = pRC->vkPhysicalDevice.getFeatures();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create logical device from the given physical device...
	pRC->vkDevice = pRC->vkPhysicalDevice.createDevice(deviceCreateInfo);

	LOG_DEBUG("Vulkan Logical device created!");

	// Queues are created at the same time as device creation, store their handle!
	pRC->vkQueueGraphics = pRC->vkDevice.getQueue(m_QueueFamilyIndices.graphicsFamily.value(), 0);
	pRC->vkQueuePresent = pRC->vkDevice.getQueue(m_QueueFamilyIndices.presentFamily.value(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
// For a given physical device, checks if it has Queue families which support Graphics & Present family queues!
void VulkanDevice::FetchQueueFamilies(vk::PhysicalDevice physicalDevice, const VulkanContext* pRC)
{
	// Get all queue families & their properties supported by physical device!
	std::vector<vk::QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();
	
	// Go through queue families and check if it supports graphics & present family queue!
	uint32_t i = 0;
	std::vector<vk::QueueFamilyProperties>::iterator iter = queueFamilyProps.begin();
	for (; iter != queueFamilyProps.end(); ++iter)
	{
		if ((*iter).queueFlags & vk::QueueFlagBits::eGraphics)
		{
			m_QueueFamilyIndices.graphicsFamily = i;
		}

		// check if this queue family has capability of presenting to our window surface!
		VkBool32 bPresentSupport = physicalDevice.getSurfaceSupportKHR(i, pRC->vkSurface);

		// if yes, store presentation family queue index!
		if (bPresentSupport)
		{
			m_QueueFamilyIndices.presentFamily = i;
		}

		if (m_QueueFamilyIndices.isComplete())
			break;

		++i;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice)
{
	// Get count of total number of extensions
	std::vector<vk::ExtensionProperties> vecSupportedExtensions;
	vecSupportedExtensions = physicalDevice.enumerateDeviceExtensionProperties();

	// Compare Required extensions with supported extensions...
	for (int i = 0; i < UT::VULKAN::DeviceExtensions.size(); ++i)
	{
		bool bExtensionFound = false;

		for (int j = 0; j < vecSupportedExtensions.size(); ++j)
		{
			// If device supported extensions matches the one we want, good news ... Enumarate them!
			if (strcmp(UT::VULKAN::DeviceExtensions[i], vecSupportedExtensions[j].extensionName) == 0)
			{
				bExtensionFound = true;

				std::string msg = std::string(UT::VULKAN::DeviceExtensions[i]) + " device extension found!";
				LOG_DEBUG(msg.c_str());

				break;
			}
		}

		// No matching extension found ... bail out!
		if (!bExtensionFound)
		{
			return false;
		}
	}

	return true;
}
