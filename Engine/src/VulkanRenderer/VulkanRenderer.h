#pragma once

#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"

struct GLFWwindow;
class VulkanDevice;
class VulkanSwapchain;
class VulkanFramebuffer;

class UT_API VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	void								Initialize(const GLFWwindow* pWindow, vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	void								Update(float dt);
	void								BeginFrame();
	void								Render();
	void								SubmitAndPresentFrame();
	void								Cleanup();
	void								CleanupOnWindowsResize();
	void								RecreateOnWindowsResize(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface);
	void								CreateFencesAndSemaphores();

private:
	void								CreateVulkanDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	void								CreateSwapchain(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface);
	void								CreateFramebufferAttachments();
	void								CreateRenderPass();
	void								CreateFramebuffers();
	void								CreateCommandbuffers();
	void								RecordCommands(uint32_t imageIndex);

private:
	VulkanDevice*						m_pVulkanDevice;
	VulkanSwapchain*					m_pSwapchain;
	VulkanFramebuffer*					m_pFramebuffer;

	//vk::Pipeline						m_vkForwardRenderingPipeline;
	//vk::PipelineLayout				m_vkForwardRenderingPipelineLayout;
	vk::RenderPass						m_vkForwardRenderingRenderPass;

	// -- Synchronization!
	uint32_t							m_uiCurrentFrame;
	uint32_t							m_uiSwapchainImageIndex;
	std::vector<vk::Semaphore>			m_vkListSemaphoreImageAvailable;
	std::vector<vk::Semaphore>			m_vkListSemaphoreRenderFinished;
	std::vector<vk::Fence>				m_vkListFences;

	GLFWwindow*							m_pWindow;
	vk::SurfaceKHR						m_vkSurface;
};

