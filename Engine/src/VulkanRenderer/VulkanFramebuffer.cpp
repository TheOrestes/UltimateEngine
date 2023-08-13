#include "UltimateEnginePCH.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "../EngineHeader.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanFramebuffer::VulkanFramebuffer()
{
	m_ListAttachments.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanFramebuffer::~VulkanFramebuffer()
{
	m_ListAttachments.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::Cleanup(VulkanContext* pRC)
{
	m_depthAttachment.DestroyAll(pRC->vkDevice);
	
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		pRC->vkDevice.destroyFramebuffer(pRC->vkListFramebuffers[i]);
		pRC->vkDevice.destroyImageView(m_ListAttachments[i].imageView);
	}

	m_ListAttachments.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CleanupOnWindowsResize(VulkanContext* pRC)
{
	m_depthAttachment.DestroyImageView(pRC->vkDevice);
	
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		pRC->vkDevice.destroyFramebuffer(pRC->vkListFramebuffers[i]);
		pRC->vkDevice.destroyImageView(m_ListAttachments[i].imageView);
	}

	m_ListAttachments.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::HandleWindowResize(VulkanContext* pRC)
{
	//uint32_t swapchainImageCount;
	//vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, nullptr);
	//
	//std::vector<VkImage> images(swapchainImageCount);
	//vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, images.data());
	//
	//for (VkImage image : images)
	//{
	//	// store image handle
	//	Helper::SwapchainAttachment swapchainImage = {};
	//	swapchainImage.image = image;
	//
	//	// Create Image View
	//	pRC->CreateImageView2D(image, pRC->vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, &(swapchainImage.imageView));
	//
	//	// Add to the list...
	//	m_ListAttachments.push_back(swapchainImage);
	//}
	//
	//pRC->vkListFramebuffers.resize(m_ListAttachments.size());
	//
	//// create framebuffer for each swapchain image
	//for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	//{
	//	std::array<VkImageView, 1> attachments =
	//	{
	//		m_ListAttachments[i].imageView
	//	};
	//
	//	VkFramebufferCreateInfo fbCreateInfo = {};
	//	fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//	fbCreateInfo.renderPass = pRC->vkForwardRenderingRenderPass;
	//	fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	//	fbCreateInfo.pAttachments = attachments.data();
	//	fbCreateInfo.width = pRC->vkSwapchainExtent.width;
	//	fbCreateInfo.height = pRC->vkSwapchainExtent.height;
	//	fbCreateInfo.layers = 1;
	//
	//	VkResult result = vkCreateFramebuffer(pRC->vkDevice, &fbCreateInfo, nullptr, &(pRC->vkListFramebuffers[i]));
	//	if (result != VK_SUCCESS)
	//	{
	//		LOG_ERROR("Failed to Re-Create Framebuffer on Windows Resize!");
	//		return;
	//	}
	//}
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateFramebuffersAttachments(VulkanContext* pRC)
{
	pRC->vkListSwapchainImages = pRC->vkDevice.getSwapchainImagesKHR(pRC->vkSwapchain);
	
	std::vector<vk::Image>::iterator iter = pRC->vkListSwapchainImages.begin();

	for (; iter != pRC->vkListSwapchainImages.end() ; ++iter)
	{
		UT::VULKAN::VulkanImage swapchainImage;
		// store image handle
		swapchainImage.image = (*iter);

		// Create Image View
		pRC->CreateImageView2D(swapchainImage.image, pRC->vkSwapchainImageFormat, vk::ImageAspectFlagBits::eColor, &swapchainImage.imageView);

		// Add to the list...
		m_ListAttachments.push_back(swapchainImage);
	}

	// Create Depth buffer attachment!
	CreateDepthBuffer(pRC);

	LOG_INFO("Framebuffer attachments created");
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateFramebuffers(VulkanContext* pRC)
{
	// Check if RenderPass exists!
	if (!pRC->vkForwardRenderingRenderPass)
	{
		LOG_ERROR("Renderpass not set!!!");
	}

	pRC->vkListFramebuffers.resize(m_ListAttachments.size());

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments =
		{
			m_ListAttachments[i].imageView,
			m_depthAttachment.imageView
		};

		vk::FramebufferCreateInfo fbCreateInfo;
		fbCreateInfo.renderPass = pRC->vkForwardRenderingRenderPass;
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCreateInfo.pAttachments = attachments.data();
		fbCreateInfo.width = pRC->vkSwapchainExtent.width;
		fbCreateInfo.height = pRC->vkSwapchainExtent.height;
		fbCreateInfo.layers = 1;

		pRC->vkListFramebuffers[i] = pRC->vkDevice.createFramebuffer(fbCreateInfo);
	}

	LOG_DEBUG("Framebuffers created!");
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateDepthBuffer(VulkanContext* pRC)
{
	// List of depth formats we need
	std::vector<vk::Format> depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint };

	// Choose the supported format
	vk::Format chosenFormat = pRC->ChooseSupportedFormat(depthFormats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	pRC->vkDepthImageFormat = chosenFormat;

	// Create depth image
	pRC->CreateImage2D(	pRC->vkSwapchainExtent.width,
						pRC->vkSwapchainExtent.height,
						chosenFormat, vk::ImageTiling::eOptimal,
						vk::ImageUsageFlagBits::eDepthStencilAttachment,
						vk::MemoryPropertyFlagBits::eDeviceLocal,
						&m_depthAttachment.image, &m_depthAttachment.deviceMemory);

	// Create depth imageview
	pRC->CreateImageView2D(m_depthAttachment.image, chosenFormat, vk::ImageAspectFlagBits::eDepth, &m_depthAttachment.imageView);
}
