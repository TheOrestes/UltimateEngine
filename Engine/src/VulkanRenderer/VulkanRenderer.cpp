
#include "UltimateEnginePCH.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanFramebuffer.h"
#include "VulkanGlobals.h"
#include "../EngineHeader.h"

#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_uiCurrentFrame = 0;
	m_uiSwapchainImageIndex = 0;
	m_pVulkanDevice = nullptr;
	m_pSwapchain = nullptr;
	m_pFramebuffer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pFramebuffer);
	SAFE_DELETE(m_pSwapchain);
	SAFE_DELETE(m_pVulkanDevice);
}   

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Initialize(const GLFWwindow* pWindow, vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	CreateVulkanDevice(vkInst, vkSurface);
	CreateSwapchain(pWindow, vkSurface);
	CreateFramebufferAttachments();
	CreateRenderPass();
	CreateFramebuffers();
	CreateCommandbuffers();
	CreateFencesAndSemaphores();

	m_pWindow = const_cast<GLFWwindow*>(pWindow);
	m_vkSurface = vkSurface;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(float dt)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::BeginFrame()
{
	vk::Device vkDevice = m_pVulkanDevice->GetDevice();

	// -- GET NEXT IMAGE
	vkDevice.waitForFences(m_vkListFences[m_uiCurrentFrame], true, UT::VkGlobals::GFenceTimeout);
	vkDevice.resetFences(m_vkListFences[m_uiCurrentFrame]);

	// Get index of next image to be drawn to & signal semaphore when ready to be drawn to!
	vk::ResultValue<uint32_t> currentBuffer = vkDevice.acquireNextImageKHR(m_pSwapchain->GetSwapchainHandle(), UT::VkGlobals::GFenceTimeout, m_vkListSemaphoreImageAvailable[m_uiCurrentFrame], nullptr);
	m_uiSwapchainImageIndex = currentBuffer.value;

	// During any event such as window size change etc. we need to check if swap chain recreation is necessary
	// Vulkan tells us that swap chain in no longer adequate during presentation
	// VK_ERROR_OUT_OF_DATE_KHR = swap chain has become incompatible with the surface & can no longer be used for rendering. (window resize)
	// VK_SUBOPTIMAL_KHR = swap chain can be still used to present to the surface but the surface properties are no longer matching!

	// if swap chain is out of date while acquiring the image, then its not possible to present it!
	// We should recreate the swap chain & try again in the next draw call...
	if (currentBuffer.result == vk::Result::eErrorOutOfDateKHR)
	{
		CleanupOnWindowsResize();
		RecreateOnWindowsResize(m_pWindow, m_vkSurface);
		return;
	}
	if (currentBuffer.result != vk::Result::eSuccess && currentBuffer.result != vk::Result::eSuboptimalKHR)
	{
		LOG_ERROR("Failed to acquire swapchain image!");
		return;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Render()
{
	BeginFrame();
	RecordCommands(m_uiSwapchainImageIndex);
	SubmitAndPresentFrame();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::SubmitAndPresentFrame()
{
	// -- SUBMIT COMMAND BUFFER TO RENDER
	// We ask for image from the swapchain for drawing, but we need to wait till that image is available 
	// also, we need to wait till our pipeline reaches COLOR_ATTACHMENT_OUTPUT stage.
	// Once we are done with the drawing to the image using graphics command buffer, we need to signal
	// saying that we are done with the drawing to the image & that image is ready to PRESENT!

	vk::Semaphore waitSemaphores[] = { m_vkListSemaphoreImageAvailable[m_uiCurrentFrame] };
	vk::Semaphore signalSemaphores[] = { m_vkListSemaphoreRenderFinished[m_uiCurrentFrame] };

	std::array<vk::CommandBuffer, 1> commandBuffers = { m_pVulkanDevice->GetGraphicsCommandBuffer(m_uiSwapchainImageIndex) };

	// Queue submission info
	vk::SubmitInfo submitInfo = {};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;						// sempahores to WAIT on

	std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pCommandBuffers = commandBuffers.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;					// semaphores to SIGNAL

	// Submit the command buffer to Graphics Queue!
	m_pVulkanDevice->GetGraphicsQueue().submit(submitInfo, m_vkListFences[m_uiCurrentFrame]);

	std::array<vk::SwapchainKHR, 1> swapchains = { m_pSwapchain->GetSwapchainHandle() };

	// 3. PRESENT RENDER IMAGE TO THE SCREEN!
	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(m_pSwapchain->m_vkSwapchain);
	presentInfo.pImageIndices = &m_uiSwapchainImageIndex;

	vk::Result result = m_pVulkanDevice->GetPresentQueue().presentKHR(presentInfo);

	if (result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to Present Image!");
		return;
	}

	m_uiCurrentFrame = (m_uiCurrentFrame + 1) % UT::VkGlobals::GMaxFramesDraws;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::RecreateOnWindowsResize(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface)
{
	m_pSwapchain->CreateSwapChain(pWindow, vkSurface, m_pVulkanDevice);
	m_pFramebuffer->CreateFramebuffersAttachments(m_pVulkanDevice, m_pSwapchain);
	m_pFramebuffer->CreateFramebuffers(m_pVulkanDevice->GetDevice(), m_vkForwardRenderingRenderPass);
	m_pVulkanDevice->CreateCommandBuffers(m_pFramebuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Cleanup()
{
	vk::Device vkDevice = m_pVulkanDevice->GetDevice();
	vkDevice.waitIdle();

	m_pSwapchain->Cleanup(vkDevice);
	m_pFramebuffer->Cleanup(vkDevice);
	m_pVulkanDevice->Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CleanupOnWindowsResize()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_pWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_pWindow, &width, &height);
		glfwWaitEvents();
	}

	vk::Device vkDevice = m_pVulkanDevice->GetDevice();
	vkDevice.waitIdle();

	m_pSwapchain->CleanupOnWindowResize(vkDevice);
	m_pFramebuffer->CleanupOnWindowsResize(vkDevice);
	m_pVulkanDevice->CleanupOnWindowsResize();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateFencesAndSemaphores()
{
	vk::Device vkDevice = m_pVulkanDevice->GetDevice();

	m_vkListSemaphoreImageAvailable.resize(UT::VkGlobals::GMaxFramesDraws);
	m_vkListSemaphoreRenderFinished.resize(UT::VkGlobals::GMaxFramesDraws);
	m_vkListFences.resize(UT::VkGlobals::GMaxFramesDraws);

	vk::SemaphoreCreateInfo sempaphoreCreateInfo = {};
	vk::FenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (uint16_t i = 0; i < UT::VkGlobals::GMaxFramesDraws; i++)
	{
		m_vkListSemaphoreImageAvailable[i] = vkDevice.createSemaphore(sempaphoreCreateInfo);
		m_vkListSemaphoreRenderFinished[i] = vkDevice.createSemaphore(sempaphoreCreateInfo);
		m_vkListFences[i] = vkDevice.createFence(fenceCreateInfo);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateVulkanDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	UT_ASSERT_NULL(vkInst, "CreateVulkanDevice()-->Vulkan Instance not valid");
	UT_ASSERT_NULL(vkSurface, "CreateVulkanDevice()-->Vulkan Surface not valid");

	m_pVulkanDevice = new VulkanDevice();
	m_pVulkanDevice->SetupDevice(vkInst, vkSurface);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateSwapchain(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface)
{
	UT_ASSERT_NULL(m_pVulkanDevice, "CreateSwapchain()-->VulkanDevice class object not valid?!");
	
	m_pSwapchain = new VulkanSwapchain();
	m_pSwapchain->CreateSwapChain(pWindow, vkSurface, m_pVulkanDevice);	
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateFramebufferAttachments()
{
	UT_ASSERT_NULL(m_pSwapchain, "CreateFramebufferAttachments()-->VulkanSwapchain class object not valid?!");

	m_pFramebuffer = new VulkanFramebuffer();
	m_pFramebuffer->CreateFramebuffersAttachments(m_pVulkanDevice, m_pSwapchain);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateRenderPass()
{
	UT_ASSERT_NULL(m_pFramebuffer, "CreateRenderPass()-->VulkanFramebuffer class object not valid?!");

	//-- ATTACHMENTS
	// 1. Color attachment
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = m_pFramebuffer->GetColorBufferFormat();
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// 2. Depth attachment
	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = m_pFramebuffer->GetDepthBufferFormat();
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference colorAttachRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthAttachRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	
	//-- SUBPASSES
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	subpass.pDepthStencilAttachment = &depthAttachRef;

	std::array<vk::AttachmentDescription, 2> renderPassAttachmentsDesc = { colorAttachment, depthAttachment };

	vk::RenderPassCreateInfo renderPassInfo(vk::RenderPassCreateFlags(), renderPassAttachmentsDesc, subpass);

	m_vkForwardRenderingRenderPass = m_pVulkanDevice->GetDevice().createRenderPass(renderPassInfo);

	LOG_INFO("Forward Renderpass created");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateFramebuffers()
{
	UT_ASSERT_NULL(m_pFramebuffer, "CreateFrameBuffers()-->VulkanFramebuffer class object not valid?!");
	UT_ASSERT_NULL(m_vkForwardRenderingRenderPass, "CreateFrameBuffers()-->vk::RenderPass not valid?!");

	m_pFramebuffer->CreateFramebuffers(m_pVulkanDevice->GetDevice(), m_vkForwardRenderingRenderPass);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateCommandbuffers()
{
	UT_ASSERT_NULL(m_pFramebuffer, "CreateCommandbuffers()-->VulkanFramebuffer class object not valid?!");

	m_pVulkanDevice->CreateCommandBuffers(m_pFramebuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::RecordCommands(uint32_t currentImage)
{
	// Information about how to begin each command buffer
	vk::CommandBufferBeginInfo cmdBufferBeginInfo = {};

	// Information about how to begin the render pass
	vk::RenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.renderPass = m_vkForwardRenderingRenderPass;
	renderPassBeginInfo.renderArea.offset = vk::Offset2D(0,0);
	renderPassBeginInfo.renderArea.extent = m_pSwapchain->GetSwapchainExtent();

	std::array<vk::ClearValue, 2> clearValues = {};

	clearValues[0].color = { 0.11f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	renderPassBeginInfo.framebuffer = m_pFramebuffer->GetFramebuffer(m_uiCurrentFrame);

	// start recording...
	m_pVulkanDevice->GetGraphicsCommandBuffer(currentImage).begin(cmdBufferBeginInfo);

	// Begin RenderPass
	m_pVulkanDevice->GetGraphicsCommandBuffer(currentImage).beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	// End RenderPass
	m_pVulkanDevice->GetGraphicsCommandBuffer(currentImage).endRenderPass();

	// end recording...
	m_pVulkanDevice->GetGraphicsCommandBuffer(currentImage).end();
}
