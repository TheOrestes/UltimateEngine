#include "UltimateEnginePCH.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanGlobals.h"
#include "../EngineHeader.h"
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanSwapchain::VulkanSwapchain()
{
	m_vkSwapchain = nullptr;

	m_vecSwapchainImages.clear();
	m_vecSwapchainImageViews.clear();
}

//---------------------------------------------------------------------------------------------------------------------
VulkanSwapchain::~VulkanSwapchain()
{
	m_vecSwapchainImages.clear();
	m_vecSwapchainImageViews.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::CreateSwapChain(const GLFWwindow* pWindow, vk::SurfaceKHR surface, const VulkanDevice* pDevice)
{
	// Get swap chain details so we can pick the best setting!
	SwapchainSupportDetails* pSwapchainSupportDetails = QuerySwapChainSupport(pDevice->GetPhysicalDevice(), surface);

	// 1. Choose best surface format
	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(pSwapchainSupportDetails->surfaceFormats);

	// 2. Choose best presentation format
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(pSwapchainSupportDetails->surfacePresentModes);

	// 3. Choose Swapchain image resolution
	vk::Extent2D extent = ChooseSwapExtent(pWindow, pDevice->GetPhysicalDevice(), surface);

	// decide how many images to have in the swap chain, it's good practice to have an extra count.
	// Also make sure it does not exceed maximum number of images
	// Get the surface capabilities for a given device
	uint32_t minImageCount = pSwapchainSupportDetails->surfaceCapabilities.minImageCount;
	minImageCount = minImageCount + 1;
	if (pSwapchainSupportDetails->surfaceCapabilities.maxImageCount > 0 && minImageCount > pSwapchainSupportDetails->surfaceCapabilities.maxImageCount)
	{
		minImageCount = pSwapchainSupportDetails->surfaceCapabilities.maxImageCount;
	}

	// Swapchain creation info
	vk::SwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.minImageCount = minImageCount;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	swapchainCreateInfo.preTransform = pSwapchainSupportDetails->surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainCreateInfo.clipped = true;

	// Specify how to handle swap chain images that will be used across multiple queue families. That will be the case 
	// in our application if the graphics queue family is different from the presentation queue. We'll be drawing on 
	// the images in the swap chain from the graphics queue and then submitting them on the presentation queue. 
	// There are two ways to handle images that are accessed from multiple queues. 
	std::array<uint32_t, 2> queueFamilyIndices = { pDevice->GetGraphicsQueueFamilyIndex(), pDevice->GetPresentQueueFamilyIndex() };

	if (!pDevice->IsQueueSharing())
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	m_vkSwapchain = pDevice->GetDevice().createSwapchainKHR(swapchainCreateInfo);

	// Retrieve handle to swapchain images...
	m_vecSwapchainImages = pDevice->GetDevice().getSwapchainImagesKHR(m_vkSwapchain);

	// Save this for later purposes. 
	m_vkSwapchainImageFormat = surfaceFormat.format;
	m_vkSwapchainExtent = extent;

	// Create swapchain image views!
	CreateSwapChainImageViews(pDevice->GetDevice());

	LOG_INFO("Vulkan Swapchain Created!");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::Cleanup(vk::Device vkDevice)
{
	for (uint32_t i = 0; i < m_vecSwapchainImageViews.size(); ++i)
	{
		vkDevice.destroyImageView(m_vecSwapchainImageViews[i]);
	}

	m_vecSwapchainImageViews.clear();
	m_vecSwapchainImages.clear();

	vkDevice.destroySwapchainKHR(m_vkSwapchain);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::CleanupOnWindowResize(vk::Device vkDevice)
{
	for (uint32_t i = 0; i < m_vecSwapchainImageViews.size(); ++i)
	{
		vkDevice.destroyImageView(m_vecSwapchainImageViews[i]);
	}

	m_vecSwapchainImageViews.clear();
	m_vecSwapchainImages.clear();

	vkDevice.destroySwapchainKHR(m_vkSwapchain);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::RecreateOnWindowResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::CreateSwapChainImageViews(vk::Device vkDevice)
{
	m_vecSwapchainImageViews.resize(m_vecSwapchainImages.size());

	for (uint32_t i = 0; i < m_vecSwapchainImageViews.size(); ++i)
	{
		// Image View Creation
		vk::ImageViewCreateInfo createInfo;

		createInfo.format = m_vkSwapchainImageFormat;
		createInfo.image = m_vecSwapchainImages[i];
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;

		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		m_vecSwapchainImageViews[i] = vkDevice.createImageView(createInfo);
	}
}

//---------------------------------------------------------------------------------------------------------------------
SwapchainSupportDetails* VulkanSwapchain::QuerySwapChainSupport(vk::PhysicalDevice vkPhysicalDevice, vk::SurfaceKHR surface)
{
	SwapchainSupportDetails* swapChainDetails = new SwapchainSupportDetails();

	// Start with basic surface capabilities...
	swapChainDetails->surfaceCapabilities = vkPhysicalDevice.getSurfaceCapabilitiesKHR(surface);
	
	// Now query supported surface formats...
	swapChainDetails->surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(surface);
	
	// Finally, query supported presentation modes...
	swapChainDetails->surfacePresentModes = vkPhysicalDevice.getSurfacePresentModesKHR(surface);
	
	return swapChainDetails;
}

//---------------------------------------------------------------------------------------------------------------------
vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	// If only 1 format available and is undefined, then this means ALL formats are available (no restrictions)
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
	{
		return { vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear };
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
vk::PresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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
vk::Extent2D VulkanSwapchain::ChooseSwapExtent(const GLFWwindow* pWindow, vk::PhysicalDevice vkPhysicalDevice, vk::SurfaceKHR vkSurface)
{
	// Get the surface capabilities for a given device
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = vkPhysicalDevice.getSurfaceCapabilitiesKHR(vkSurface);

	// The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the 
	// resolution of the window that we're drawing to.The range of the possible resolutions is defined in the 
	// VkSurfaceCapabilitiesKHR structure.Vulkan tells us to match the resolution of the window by setting the 
	// width and height in the currentExtent member.However, some window managers do allow us to differ here 
	// and this is indicated by setting the width and height in currentExtent to a special value : the maximum 
	// value of uint32_t. In that case we'll pick the resolution that best matches the window within the 
	// minImageExtent and maxImageExtent bounds.
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		// To handle window resize properly, query current width-height of framebuffer, instead of global value!
		int width, height;
		glfwGetFramebufferSize(const_cast<GLFWwindow*>(pWindow), &width, &height);

		//VkExtent2D actualExtent = { App::WIDTH, App::HEIGHT };
		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}
