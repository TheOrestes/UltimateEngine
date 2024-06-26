#pragma once

#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"

struct GLFWwindow;
class VulkanDevice;
class VulkanSwapchain;
class VulkanFramebuffer;
class UIManager;
class Scene;
enum class CameraAction;

class UT_API VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool								Initialize(const GLFWwindow* pWindow, vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	void								Update(double dt) const;
	void								BeginFrame();
	void								Render();
	void								SubmitAndPresentFrame();
	void								Cleanup();
	void								CleanupOnWindowsResize();
	void								RecreateOnWindowsResize(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface);
	bool								CreateFencesAndSemaphores();
	bool								CreateGraphicsPipeline();

	void								HandleSceneInput(const GLFWwindow* pWindow, CameraAction direction, float mousePosX = 0.0f, float mousePosY = 0.0f, bool isMouseClicked = false) const;

private:
	bool								CreateVulkanDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface);
	bool								CreateSwapchain(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface);
	bool								CreateFramebufferAttachments();
	bool								CreateRenderPass();
	bool								CreateFramebuffers();
	bool								CreateCommandbuffers() const;
	void								RecordCommands(uint32_t currentImage) const;

private:
	VulkanDevice*						m_pVulkanDevice;
	VulkanSwapchain*					m_pSwapchain;
	VulkanFramebuffer*					m_pFramebuffer;

	vk::Pipeline						m_vkForwardRenderingPipeline;
	vk::RenderPass						m_vkForwardRenderingRenderPass;

	// -- Synchronization!
	uint32_t							m_uiCurrentFrame;
	uint32_t							m_uiSwapchainImageIndex;
	std::vector<vk::Semaphore>			m_vkListSemaphoreImageAvailable;
	std::vector<vk::Semaphore>			m_vkListSemaphoreRenderFinished;
	std::vector<vk::Fence>				m_vkListFences;

	GLFWwindow*							m_pWindow;
	vk::SurfaceKHR						m_vkSurface;

	Scene*								m_pScene;
	UIManager*							m_pGUI;
};

