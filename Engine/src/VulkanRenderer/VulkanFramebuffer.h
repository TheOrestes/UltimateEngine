#pragma once

#include "VulkanGlobals.h"

struct VulkanContext;

class VulkanFramebuffer
{
public:
	VulkanFramebuffer();
	~VulkanFramebuffer();

	void									Cleanup(VulkanContext* pRC);
	void									CleanupOnWindowsResize(VulkanContext* pRC);
	void									HandleWindowResize(VulkanContext* pRC);
	void									CreateFramebuffersAttachments(VulkanContext* pRC);
	void									CreateFramebuffers(VulkanContext* pRC);

private:
	void									CreateDepthBuffer(VulkanContext* pRC);

public:
	std::vector<UT::VULKAN::VulkanImage>	m_ListAttachments;
	UT::VULKAN::VulkanImage					m_depthAttachment;
};

