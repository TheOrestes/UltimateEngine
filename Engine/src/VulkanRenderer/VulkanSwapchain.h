#pragma once

#include "VulkanGlobals.h"

struct GLFWwindow;
class VulkanDevice;

//---------------------------------------------------------------------------------------------------------------------
struct SwapchainSupportDetails
{
	vk::SurfaceCapabilitiesKHR	surfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> surfaceFormats;
	std::vector<vk::PresentModeKHR>	surfacePresentModes;
};

//---------------------------------------------------------------------------------------------------------------------
class UT_API VulkanSwapchain
{
public:
	VulkanSwapchain();
	~VulkanSwapchain();

	void									CreateSwapChain(const GLFWwindow* pWindow, vk::SurfaceKHR surface, const VulkanDevice* pDevice);
	void									Cleanup(vk::Device vkDevice);
	void									CleanupOnWindowResize(vk::Device vkDevice);
	void									RecreateOnWindowResize();

private:
	void									CreateSwapChainImageViews(vk::Device vkDevice);
	SwapchainSupportDetails*				QuerySwapChainSupport(vk::PhysicalDevice vkPhysicalDevice, vk::SurfaceKHR surface);
	vk::SurfaceFormatKHR					ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR						ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	vk::Extent2D							ChooseSwapExtent(const GLFWwindow* pWindow, vk::PhysicalDevice vkPhysicalDevice, vk::SurfaceKHR vkSurface);

public:
	inline vk::SwapchainKHR					GetSwapchainHandle() const						{ return m_vkSwapchain; }
	inline vk::Format						GetSwapchainImageFormat() const					{ return m_vkSwapchainImageFormat; }
	inline vk::Extent2D						GetSwapchainExtent() const						{ return m_vkSwapchainExtent; }
	inline uint32_t							GetSwapchainImageCount() const					{ return static_cast<uint32_t>(m_vecSwapchainImages.size()); }
	inline vk::Image						GetSwapchainImageAt(uint32_t index) const 		{ return m_vecSwapchainImages.at(index); }
	inline vk::ImageView					GetSwapchainImageViewAt(uint32_t index) const	{ return m_vecSwapchainImageViews.at(index); }

	vk::SwapchainKHR						m_vkSwapchain;

private:
	uint32_t								m_uiMinImageCount;
	vk::Format								m_vkSwapchainImageFormat;
	vk::Extent2D							m_vkSwapchainExtent;

	std::vector<vk::Image>					m_vecSwapchainImages;
	std::vector<vk::ImageView>				m_vecSwapchainImageViews;
};
