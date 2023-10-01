#include "UltimateEnginePCH.h"
#include "VulkanFramebuffer.h"
#include "VulkanUtility.h"
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
	vk::Device vkDevice = pDevice->GetDevice();
	vk::PhysicalDevice vkPhysicalDevice = pDevice->GetPhysicalDevice();
	vk::SwapchainKHR vkSwapchain = pSwapchain->GetSwapchainHandle();	

	vk::Format imgFormat = pSwapchain->GetSwapchainImageFormat();
	vk::Extent2D imgExtent = pSwapchain->GetSwapchainExtent();

	for (uint32_t i = 0 ; i < pSwapchain->GetSwapchainImageCount() ; ++i)
	{
		UT::VkGlobals::VulkanImage colorBufferImage = {};
		colorBufferImage.extent = imgExtent;
		colorBufferImage.format = imgFormat;
		
		colorBufferImage.image = pSwapchain->GetSwapchainImageAt(i);
		colorBufferImage.imageView = pSwapchain->GetSwapchainImageViewAt(i);
		
		m_ListColorAttachments.push_back(colorBufferImage);
	}

	// Create Depth buffer attachment!
	CreateDepthBufferAttachment(vkDevice, vkPhysicalDevice, imgExtent.width, imgExtent.height);

	LOG_INFO("Framebuffer attachments created");
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateFramebuffers(vk::Device vkDevice, vk::RenderPass renderPass)
{
	// Check if RenderPass exists!
	if (!renderPass)
	{
		LOG_ERROR("Renderpass not set!!!");
	}

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListColorAttachments.size(); i++)
	{
		vk::Framebuffer framebuffer;

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

		framebuffer = vkDevice.createFramebuffer(fbCreateInfo);
		m_vkListFramebuffers.push_back(framebuffer);
	}

	LOG_DEBUG("Framebuffers created!");
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanFramebuffer::CreateDepthBufferAttachment(vk::Device vkDevice, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height)
{
	// List of depth formats we need
	std::vector<vk::Format> depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint };

	// Choose the supported format
	vk::Format chosenFormat = UT::VkUtility::ChooseSupportedFormat(physicalDevice, depthFormats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	// Create depth image
	std::tuple<vk::Image, vk::DeviceMemory> depthImageData;
	depthImageData = UT::VkUtility::CreateImage2D(	vkDevice, physicalDevice, width, height,
													chosenFormat, vk::ImageTiling::eOptimal,
													vk::ImageUsageFlagBits::eDepthStencilAttachment,
													vk::MemoryPropertyFlagBits::eDeviceLocal);

	m_DepthAttachment.image = std::get<vk::Image>(depthImageData);
	m_DepthAttachment.deviceMemory = std::get<vk::DeviceMemory>(depthImageData);

	m_DepthAttachment.format = chosenFormat;
	m_DepthAttachment.extent = vk::Extent2D(width, height);

	// Create depth imageview
	m_DepthAttachment.imageView = UT::VkUtility::CreateImageView2D(vkDevice, m_DepthAttachment.image, chosenFormat, vk::ImageAspectFlagBits::eDepth);
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

