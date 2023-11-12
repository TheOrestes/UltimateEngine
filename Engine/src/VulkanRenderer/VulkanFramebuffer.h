#pragma once

#include "VulkanGlobals.h"

struct GLFWWindow;
class VulkanDevice;
class VulkanSwapchain;

class UT_API VulkanFramebuffer
{
public:
	VulkanFramebuffer();
	~VulkanFramebuffer();

	void									Cleanup(vk::Device vkDevice);
	void									CleanupOnWindowsResize(vk::Device vkDevice);
	void									RecreateOnWindowResize(const VulkanDevice* pDevice, const VulkanSwapchain* pSwapchain);
	void									CreateFramebuffersAttachments(const VulkanDevice* pDevice, const VulkanSwapchain* pSwapchain);
	void									CreateFramebuffers(const VulkanDevice* pDevice, vk::RenderPass renderPass);

	inline vk::Format						GetColorBufferFormat()			const { return m_ListColorAttachments[0].format; }
	inline vk::Format						GetDepthBufferFormat()			const { return m_DepthAttachment.format; }
	inline uint32_t							GetFramebufferCount()			const { return static_cast<uint32_t>(m_vkListFramebuffers.size()); }
	inline vk::Framebuffer					GetFramebuffer(uint32_t index)	const { return m_vkListFramebuffers.at(index); }

private:
	void									CreateDepthBufferAttachment(const VulkanDevice* pDevice, uint32_t width, uint32_t height, UT::VkStructs::VulkanImage& depthImage);

private:
	std::vector<vk::Framebuffer>			m_vkListFramebuffers;
	std::vector<UT::VkStructs::VulkanImage> m_ListColorAttachments;
	UT::VkStructs::VulkanImage				m_DepthAttachment;
};

