#include "UltimateEnginePCH.h"
#include "VulkanFramebuffer.h"
#include "VulkanGlobals.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "../EngineHeader.h"
#include "GLFW/glfw3.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanFramebuffer::VulkanFramebuffer()
{
	m_ListColorAttachments.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanFramebuffer::~VulkanFramebuffer()
{
	m_ListColorAttachments.clear();
}


//-----------------------------------------------------------------------------------------------------------------------
// Note: think about sending VulkanSwapchain* instead of Vulkan handles!
void VulkanFramebuffer::CreateFramebuffersAttachments(const VulkanDevice* pDevice, const VulkanSwapchain* pSwapchain)
{
	const vk::Format imgFormat = pSwapchain->GetSwapchainImageFormat();
	const vk::Extent2D imgExtent = pSwapchain->GetSwapchainExtent();

	for (uint32_t i = 0 ; i < pSwapchain->GetSwapchainImageCount() ; ++i)
	{
		UT::VkStructs::VulkanImage colorBufferImage;
		colorBufferImage.extent = imgExtent;
		colorBufferImage.format = imgFormat;
		colorBufferImage.image = pSwapchain->GetSwapchainImageAt(i);
		colorBufferImage.imageView = pSwapchain->GetSwapchainImageViewAt(i);
		
		m_ListColorAttachments.push_back(colorBufferImage);
	}

	// Create Depth buffer attachment!
	CreateDepthBufferAttachment(pDevice, imgExtent.width, imgExtent.height, m_DepthAttachment);

	LOG_INFO("Framebuffer attachments created");
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateFramebuffers(const VulkanDevice* pDevice, vk::RenderPass renderPass)
{
	// Check if RenderPass exists!
	if (!renderPass)
	{
		LOG_ERROR("Renderpass not set!!!");
	}

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListColorAttachments.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments =
		{
			m_ListColorAttachments[i].imageView,
			m_DepthAttachment.imageView
		};

		vk::FramebufferCreateInfo fbCreateInfo;
		fbCreateInfo.renderPass = renderPass;
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCreateInfo.pAttachments = attachments.data();
		fbCreateInfo.width = m_ListColorAttachments[i].extent.width;
		fbCreateInfo.height = m_ListColorAttachments[i].extent.height;
		fbCreateInfo.layers = 1;

		vk::Framebuffer framebuffer = pDevice->GetDevice().createFramebuffer(fbCreateInfo);
		m_vkListFramebuffers.push_back(framebuffer);
	}

	LOG_DEBUG("Framebuffers created!");
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateDepthBufferAttachment(const VulkanDevice* pDevice, uint32_t width, uint32_t height, UT::VkStructs::VulkanImage& depthImage)
{
	// List of depth formats we need
	const std::vector<vk::Format> depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint };

	// Choose the supported format
	const vk::Format chosenFormat = pDevice->ChooseSupportedFormat(depthFormats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	// Create depth image
	pDevice->CreateImage2D(	width, height,
							chosenFormat, vk::ImageTiling::eOptimal,
							vk::ImageUsageFlagBits::eDepthStencilAttachment,
							vk::MemoryPropertyFlagBits::eDeviceLocal,
							vk::ImageAspectFlagBits::eDepth,
							&depthImage);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::Cleanup(vk::Device vkDevice)
{
	m_DepthAttachment.DestroyAll(vkDevice);

	for (uint32_t i = 0; i < m_ListColorAttachments.size(); i++)
	{
		vkDevice.destroyFramebuffer(m_vkListFramebuffers[i]);

		// framebuffer uses swapchain image/imageviews, by this time, that should be cleaned up!
		//vkDevice.destroyImageView(m_ListColorAttachments[i].imageView);
	}

	m_ListColorAttachments.clear();
	m_vkListFramebuffers.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CleanupOnWindowsResize(vk::Device vkDevice)
{
	m_DepthAttachment.DestroyImageView(vkDevice);

	for (uint32_t i = 0; i < m_ListColorAttachments.size(); i++)
	{
		vkDevice.destroyFramebuffer(m_vkListFramebuffers[i]);
		//vkDevice.destroyImageView(m_ListColorAttachments[i].imageView);
	}


	m_ListColorAttachments.clear();
	m_vkListFramebuffers.clear();
}

