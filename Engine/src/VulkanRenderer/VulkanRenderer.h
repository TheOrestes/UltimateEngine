#pragma once

#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"

class VulkanContext;
class VulkanDevice;
class VulkanFramebuffer;

class UT_API VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	void								Initialize(VulkanContext* pRC);
	void								Update(float dt);
	void								Render(VulkanContext* pRC);
	void								HandleWindowsResize();
	void								Cleanup(VulkanContext* pRC);
	void								CleanupOnWindowsResize(VulkanContext* pRC);
	void								CreateSynchronization(VulkanContext* pRC);

private:
	void								CreateVulkanDevice(VulkanContext* pRC);
	void								CreateFramebufferAttachments(VulkanContext* pRC);
	void								CreateRenderPass(VulkanContext* pRC);
	void								CreateFramebuffers(VulkanContext* pRC);
	void								CreateCommandbuffers(VulkanContext* pRC);
	void								RecordCommands(VulkanContext* pRC, uint32_t imageIndex);

private:
	VulkanDevice*						m_pVulkanDevice;
	VulkanFramebuffer*					m_pFramebuffer;

	// -- Synchronization!
	uint32_t							m_uiCurrentFrame;
	std::vector<vk::Semaphore>			m_vkListSemaphoreImageAvailable;
	std::vector<vk::Semaphore>			m_vkListSemaphoreRenderFinished;
	std::vector<vk::Fence>				m_vkListFences;
};

