#include "UltimateEnginePCH.h"
#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanFramebuffer.h"
#include "../EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_pVulkanDevice = nullptr;
	m_pFramebuffer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pFramebuffer);
	SAFE_DELETE(m_pVulkanDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Initialize(VulkanContext* pRC)
{
	CreateVulkanDevice(pRC);
	CreateFramebufferAttachments(pRC);
	CreateRenderPass(pRC);
	CreateFramebuffers(pRC);
	CreateCommandbuffers(pRC);
	CreateSynchronization(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(float dt)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Render(VulkanContext* pRC)
{
	// -- GET NEXT IMAGE
	pRC->vkDevice.waitForFences(m_vkListFences[m_uiCurrentFrame], true, UT::VULKAN::gFenceTimeout);
	pRC->vkDevice.resetFences(m_vkListFences[m_uiCurrentFrame]);

	// Get index of next image to be drawn to & signal semaphore when ready to be drawn to!
	vk::ResultValue<uint32_t> currentBuffer = pRC->vkDevice.acquireNextImageKHR(pRC->vkSwapchain, UT::VULKAN::gFenceTimeout, m_vkListSemaphoreImageAvailable[m_uiCurrentFrame], nullptr);
	uint32_t imageIndex = currentBuffer.value;

	// During any event such as window size change etc. we need to check if swap chain recreation is necessary
	// Vulkan tells us that swap chain in no longer adequate during presentation
	// VK_ERROR_OUT_OF_DATE_KHR = swap chain has become incompatible with the surface & can no longer be used for rendering. (window resize)
	// VK_SUBOPTIMAL_KHR = swap chain can be still used to present to the surface but the surface properties are no longer matching!

	// if swap chain is out of date while acquiring the image, then its not possible to present it!
	// We should recreate the swap chain & try again in the next draw call...
	if (currentBuffer.result == vk::Result::eErrorOutOfDateKHR)
	{
		HandleWindowsResize();
		return;
	}
	else if (currentBuffer.result != vk::Result::eSuccess && currentBuffer.result != vk::Result::eSuboptimalKHR)
	{
		LOG_ERROR("Failed to acquire swapchain image!");
		return;
	}

	RecordCommands(pRC, imageIndex);

	// -- SUBMIT COMMAND BUFFER TO RENDER
	// We ask for image from the swapchain for drawing, but we need to wait till that image is available 
	// also, we need to wait till our pipeline reaches COLOR_ATTACHMENT_OUTPUT stage.
	// Once we are done with the drawing to the image using graphics command buffer, we need to signal
	// saying that we are done with the drawing to the image & that image is ready to PRESENT!

	// Queue submission info
	vk::SubmitInfo submitInfo = {};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &(m_vkListSemaphoreImageAvailable[m_uiCurrentFrame]);						// sempahores to WAIT on

	std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(pRC->vkListGraphicsCommandBuffers[imageIndex]);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &(m_vkListSemaphoreRenderFinished[m_uiCurrentFrame]);					// semaphores to SIGNAL

	// Submit the command buffer to Graphics Queue!
	pRC->vkQueueGraphics.submit(submitInfo, m_vkListFences[m_uiCurrentFrame]);

	// 3. PRESENT RENDER IMAGE TO THE SCREEN!
	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &(m_vkListSemaphoreRenderFinished[m_uiCurrentFrame]);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(pRC->vkSwapchain);
	presentInfo.pImageIndices = &imageIndex;

	vk::Result result = pRC->vkQueuePresent.presentKHR(presentInfo);
	if (result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to Present Image!");
		return;
	}

	m_uiCurrentFrame = (m_uiCurrentFrame + 1) % UT::VULKAN::gMaxFramesDraws;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::HandleWindowsResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Cleanup(VulkanContext* pRC)
{
	pRC->vkDevice.waitIdle();

	m_pFramebuffer->Cleanup(pRC);
	m_pVulkanDevice->Cleanup(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CleanupOnWindowsResize(VulkanContext* pRC)
{
	m_pFramebuffer->CleanupOnWindowsResize(pRC);
	m_pVulkanDevice->CleanupOnWindowsResize(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateSynchronization(VulkanContext* pRC)
{
	m_vkListSemaphoreImageAvailable.resize(UT::VULKAN::gMaxFramesDraws);
	m_vkListSemaphoreRenderFinished.resize(UT::VULKAN::gMaxFramesDraws);
	m_vkListFences.resize(UT::VULKAN::gMaxFramesDraws);

	vk::SemaphoreCreateInfo sempaphoreCreateInfo = {};
	vk::FenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (uint16_t i = 0; i < UT::VULKAN::gMaxFramesDraws; i++)
	{
		m_vkListSemaphoreImageAvailable[i] = pRC->vkDevice.createSemaphore(sempaphoreCreateInfo);
		m_vkListSemaphoreRenderFinished[i] = pRC->vkDevice.createSemaphore(sempaphoreCreateInfo);
		m_vkListFences[i] = pRC->vkDevice.createFence(fenceCreateInfo);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateVulkanDevice(VulkanContext* pRC)
{
	m_pVulkanDevice = new VulkanDevice(pRC);
	m_pVulkanDevice->SetupDevice(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateFramebufferAttachments(VulkanContext* pRC)
{
	m_pFramebuffer = new VulkanFramebuffer();
	m_pFramebuffer->CreateFramebuffersAttachments(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateRenderPass(VulkanContext* pRC)
{
	//-- ATTACHMENTS
	// 1. Color attachment
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = pRC->vkSwapchainImageFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// 2. Depth attachment
	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = pRC->vkDepthImageFormat;
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

	pRC->vkForwardRenderingRenderPass = pRC->vkDevice.createRenderPass(renderPassInfo);

	LOG_INFO("Forward Renderpass created");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateFramebuffers(VulkanContext* pRC)
{
	if (m_pFramebuffer)
	{
		m_pFramebuffer->CreateFramebuffers(pRC);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateCommandbuffers(VulkanContext* pRC)
{
	m_pVulkanDevice->CreateCommandPool(pRC);
	m_pVulkanDevice->CreateCommandBuffers(pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::RecordCommands(VulkanContext* pRC, uint32_t currentImage)
{
	// Information about how to begin each command buffer
	vk::CommandBufferBeginInfo cmdBufferBeginInfo = {};

	// Information about how to begin the render pass
	vk::RenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.renderPass = pRC->vkForwardRenderingRenderPass;
	renderPassBeginInfo.renderArea.offset = vk::Offset2D(0,0);
	renderPassBeginInfo.renderArea.extent = pRC->vkSwapchainExtent;

	std::array<vk::ClearValue, 2> clearValues = {};

	clearValues[0].color = { 0.11f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	renderPassBeginInfo.framebuffer = pRC->vkListFramebuffers[currentImage];

	// start recording...
	pRC->vkListGraphicsCommandBuffers[currentImage].begin(cmdBufferBeginInfo);

	// Begin RenderPass
	pRC->vkListGraphicsCommandBuffers[currentImage].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	// End RenderPass
	pRC->vkListGraphicsCommandBuffers[currentImage].endRenderPass();

	// end recording...
	pRC->vkListGraphicsCommandBuffers[currentImage].end();
}
