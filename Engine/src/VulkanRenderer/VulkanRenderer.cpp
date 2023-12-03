
#include "UltimateEnginePCH.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanFramebuffer.h"
#include "VulkanGlobals.h"
#include "../World/Scene.h"
#include "../RenderObjects/GameObject.h"
#include "../RenderObjects/VulkanCube.h"
#include "../EngineHeader.h"
#include "../RenderObjects/VulkanMeshData.h"
#include "../UI/UIManager.h"

#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_uiCurrentFrame = 0;
	m_uiSwapchainImageIndex = 0;
	m_pVulkanDevice = nullptr;
	m_pSwapchain = nullptr;
	m_pFramebuffer = nullptr;

	m_pScene = nullptr;
	m_pGUI = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pGUI);
	SAFE_DELETE(m_pScene);
	SAFE_DELETE(m_pFramebuffer);
	SAFE_DELETE(m_pSwapchain);
	SAFE_DELETE(m_pVulkanDevice);
}   

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::Initialize(const GLFWwindow* pWindow, vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	CHECK_LOG(CreateVulkanDevice(vkInst, vkSurface),	"Vulkan Device creation FAILED!");
	CHECK_LOG(CreateSwapchain(pWindow, vkSurface),		"Swapchain creation FAILED!");
	CHECK_LOG(CreateFramebufferAttachments(),			"Framebuffer attachment creation FAILED!");
	CHECK_LOG(CreateRenderPass(),						"Renderpass creation FAILED!");
	CHECK_LOG(CreateFramebuffers(),						"Framebuffer creation FAILED!");
	CHECK_LOG(CreateCommandbuffers(),					"Command buffer creation FAILED!");
	CHECK_LOG(CreateFencesAndSemaphores(),				"Fences & Semaphore creation FAILED!");

	m_pWindow = const_cast<GLFWwindow*>(pWindow);
	m_vkSurface = vkSurface;

	m_pGUI = new UIManager();
	CHECK_LOG(m_pGUI->Initialize(pWindow, vkInst, m_vkForwardRenderingRenderPass, m_pVulkanDevice), "LogManager initialization FAILED!");

	m_pScene = new Scene();
	CHECK_LOG(m_pScene->LoadScene(m_pVulkanDevice), "Load Scene FAILED!");

	CHECK_LOG(CreateGraphicsPipeline(), "Graphics Pipeline creation FAILED!");

	LOG_DEBUG("Vulkan Renderer Initialized!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(float dt) const
{
	m_pScene->Update(dt);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::BeginFrame()
{
	const vk::Device vkDevice = m_pVulkanDevice->GetDevice();

	// -- GET NEXT IMAGE
	vkDevice.waitForFences(m_vkListFences[m_uiCurrentFrame], true, UT::VkGlobals::GFenceTimeout);
	vkDevice.resetFences(m_vkListFences[m_uiCurrentFrame]);

	// Get index of next image to be drawn to & signal semaphore when ready to be drawn to!
	const vk::ResultValue<uint32_t> currentBuffer = vkDevice.acquireNextImageKHR(m_pSwapchain->GetSwapchainHandle(), UT::VkGlobals::GFenceTimeout, m_vkListSemaphoreImageAvailable[m_uiCurrentFrame], nullptr);
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

	const vk::Semaphore waitSemaphores[] = { m_vkListSemaphoreImageAvailable[m_uiCurrentFrame] };
	const vk::Semaphore signalSemaphores[] = { m_vkListSemaphoreRenderFinished[m_uiCurrentFrame] };

	const std::array<vk::CommandBuffer, 1> commandBuffers = { m_pVulkanDevice->GetGraphicsCommandBuffer(m_uiSwapchainImageIndex) };

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

	const vk::Result result = m_pVulkanDevice->GetPresentQueue().presentKHR(presentInfo);

	if (result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to Present Image!");
		return;
	}

	m_uiCurrentFrame = (m_uiCurrentFrame + 1) % UT::VkGlobals::GMaxFramesDraws;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::RecreateOnWindowsResize(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface) const
{
	m_pSwapchain->CreateSwapChain(pWindow, vkSurface, m_pVulkanDevice);
	m_pFramebuffer->CreateFramebuffersAttachments(m_pVulkanDevice, m_pSwapchain);
	m_pFramebuffer->CreateFramebuffers(m_pVulkanDevice, m_vkForwardRenderingRenderPass);
	m_pVulkanDevice->CreateCommandBuffers(m_pFramebuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Cleanup() const
{
	vk::Device vkDevice = m_pVulkanDevice->GetDevice();
	vkDevice.waitIdle();

	m_pSwapchain->Cleanup(vkDevice);
	m_pFramebuffer->Cleanup(vkDevice);
	m_pVulkanDevice->Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CleanupOnWindowsResize() const
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_pWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_pWindow, &width, &height);
		glfwWaitEvents();
	}

	const vk::Device vkDevice = m_pVulkanDevice->GetDevice();
	vkDevice.waitIdle();

	m_pSwapchain->CleanupOnWindowResize(vkDevice);
	m_pFramebuffer->CleanupOnWindowsResize(vkDevice);
	m_pVulkanDevice->CleanupOnWindowsResize();
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateFencesAndSemaphores()
{
	const vk::Device vkDevice = m_pVulkanDevice->GetDevice();

	m_vkListSemaphoreImageAvailable.resize(UT::VkGlobals::GMaxFramesDraws);
	m_vkListSemaphoreRenderFinished.resize(UT::VkGlobals::GMaxFramesDraws);
	m_vkListFences.resize(UT::VkGlobals::GMaxFramesDraws);

	constexpr vk::SemaphoreCreateInfo sempaphoreCreateInfo = {};
	vk::FenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (uint16_t i = 0; i < UT::VkGlobals::GMaxFramesDraws; i++)
	{
		m_vkListSemaphoreImageAvailable[i] = vkDevice.createSemaphore(sempaphoreCreateInfo);
		m_vkListSemaphoreRenderFinished[i] = vkDevice.createSemaphore(sempaphoreCreateInfo);
		m_vkListFences[i] = vkDevice.createFence(fenceCreateInfo);
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateGraphicsPipeline()
{
	// Read shader code & create modules
	vk::ShaderModule vsModule = m_pVulkanDevice->CreateShaderModule("Assets/Shaders/triangle.vert.spv");
	vk::ShaderModule fsModule = m_pVulkanDevice->CreateShaderModule("Assets/Shaders/triangle.frag.spv");

	// Vertex Shader stage creation info
	vk::PipelineShaderStageCreateInfo vsCreateInfo = {};
	vsCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vsCreateInfo.module = vsModule;
	vsCreateInfo.pName = "main";

	// Fragment Shader stage creation info
	vk::PipelineShaderStageCreateInfo fsCreateInfo = {};
	fsCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fsCreateInfo.module = fsModule;
	fsCreateInfo.pName = "main";

	std::array<vk::PipelineShaderStageCreateInfo, 2> arrShaderStages = { vsCreateInfo, fsCreateInfo };

	// How the data for a single vertex is as a whole!
	vk::VertexInputBindingDescription inputBindingDesc = {};
	inputBindingDesc.binding = 0;
	inputBindingDesc.stride = sizeof(VertexPNTBT);
	inputBindingDesc.inputRate = vk::VertexInputRate::eVertex;

	std::array<vk::VertexInputAttributeDescription, 5> attrDesc = {};

	// Position
	attrDesc[0].binding = 0;
	attrDesc[0].location = 0;
	attrDesc[0].format = vk::Format::eR32G32B32A32Sfloat;
	attrDesc[0].offset = offsetof(VertexPNTBT, Position);

	// Normal
	attrDesc[1].binding = 0;
	attrDesc[1].location = 1;
	attrDesc[1].format = vk::Format::eR32G32B32Sfloat;
	attrDesc[1].offset = offsetof(VertexPNTBT, Normal);

	// Tangent
	attrDesc[2].binding = 0;
	attrDesc[2].location = 2;
	attrDesc[2].format = vk::Format::eR32G32B32Sfloat;
	attrDesc[2].offset = offsetof(VertexPNTBT, Tangent);

	// BiNormal
	attrDesc[3].binding = 0;
	attrDesc[3].location = 3;
	attrDesc[3].format = vk::Format::eR32G32B32Sfloat;
	attrDesc[3].offset = offsetof(VertexPNTBT, BiNormal);

	// UV
	attrDesc[4].binding = 0;
	attrDesc[4].location = 4;
	attrDesc[4].format = vk::Format::eR32G32Sfloat;
	attrDesc[4].offset = offsetof(VertexPNTBT, UV);

	// Vertex Input (TODO)
	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attrDesc.data();
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &inputBindingDesc;

	// Input Assembly
	vk::PipelineInputAssemblyStateCreateInfo inputASCreateInfo = {};
	inputASCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
	inputASCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Viewport & Scissor
	vk::Viewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = static_cast<float>(UT::VkGlobals::GCurrentResolution.x);
	vp.height = static_cast<float>(UT::VkGlobals::GCurrentResolution.y);
	vp.maxDepth = 1.0f;
	vp.minDepth = 0.0f;

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D(0,0);
	scissor.extent = m_pSwapchain->GetSwapchainExtent();

	vk::PipelineViewportStateCreateInfo vpCreateInfo = {};
	vpCreateInfo.viewportCount = 1;
	vpCreateInfo.pViewports = &vp;
	vpCreateInfo.scissorCount = 1;
	vpCreateInfo.pScissors = &scissor;

	// Dynamic States
	//std::vector<VkDynamicState> dynamicStates;
	//dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	//dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	//
	//VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	//dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	//dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	// Rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = vk::PolygonMode::eFill;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
	rasterizerCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	// Multisampling
	vk::PipelineMultisampleStateCreateInfo msCreateInfo = {};
	msCreateInfo.sampleShadingEnable = VK_FALSE;
	msCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

	// Blending
	vk::PipelineColorBlendAttachmentState colorState = {};
	colorState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorState.blendEnable = VK_TRUE;
	colorState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	colorState.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	colorState.colorBlendOp = vk::BlendOp::eAdd;
	colorState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	colorState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	colorState.alphaBlendOp = vk::BlendOp::eAdd;

	vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorState;

	// Depth Stencil 
	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = vk::CompareOp::eLess;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	vk::GraphicsPipelineCreateInfo forwardRenderingPipelineInfo = {};
	forwardRenderingPipelineInfo.stageCount = static_cast<uint32_t>(arrShaderStages.size());
	forwardRenderingPipelineInfo.pStages = arrShaderStages.data();
	forwardRenderingPipelineInfo.pVertexInputState = &vertexInputCreateInfo;
	forwardRenderingPipelineInfo.pInputAssemblyState = &inputASCreateInfo;
	forwardRenderingPipelineInfo.pViewportState = &vpCreateInfo;
	forwardRenderingPipelineInfo.pDynamicState = nullptr;
	forwardRenderingPipelineInfo.pRasterizationState = &rasterizerCreateInfo;
	forwardRenderingPipelineInfo.pMultisampleState = &msCreateInfo;
	forwardRenderingPipelineInfo.pColorBlendState = &colorBlendCreateInfo;
	forwardRenderingPipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
	forwardRenderingPipelineInfo.layout = dynamic_cast<VulkanCube*>(m_pScene->GetFirstObject())->GetPipelineLayout();
	forwardRenderingPipelineInfo.renderPass = m_vkForwardRenderingRenderPass;
	forwardRenderingPipelineInfo.subpass = 0;
	forwardRenderingPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	forwardRenderingPipelineInfo.basePipelineIndex = -1;

	//--  Create Graphics Pipeline!!
	const vk::Device vkDevice = m_pVulkanDevice->GetDevice();
	vk::Result result;
	std::tie(result, m_vkForwardRenderingPipeline) = vkDevice.createGraphicsPipeline(nullptr, forwardRenderingPipelineInfo);

	switch (result)
	{
		case vk::Result::eSuccess:
			{
				LOG_DEBUG("Forward Graphics Pipeline created!");
				break;
			}

		// should ideally never happen!
		default: assert(false);
	}

	// Destroy shader module
	vkDestroyShaderModule(vkDevice, fsModule, nullptr);
	vkDestroyShaderModule(vkDevice, vsModule, nullptr);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateVulkanDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	CHECK_LOG(vkInst, "CreateVulkanDevice()-->Vulkan Instance not valid");
	CHECK_LOG(vkSurface, "CreateVulkanDevice()-->Vulkan Surface not valid");

	m_pVulkanDevice = new VulkanDevice();
	CHECK(m_pVulkanDevice->SetupDevice(vkInst, vkSurface));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateSwapchain(const GLFWwindow* pWindow, vk::SurfaceKHR vkSurface)
{
	CHECK_LOG(m_pVulkanDevice, "CreateSwapchain()-->VulkanDevice class object not valid?!");
	
	m_pSwapchain = new VulkanSwapchain();
	m_pSwapchain->CreateSwapChain(pWindow, vkSurface, m_pVulkanDevice);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateFramebufferAttachments()
{
	UT_ASSERT_NULL(m_pSwapchain, "CreateFramebufferAttachments()-->VulkanSwapchain class object not valid?!");

	m_pFramebuffer = new VulkanFramebuffer();
	m_pFramebuffer->CreateFramebuffersAttachments(m_pVulkanDevice, m_pSwapchain);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateRenderPass()
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

	constexpr vk::AttachmentReference colorAttachRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	constexpr vk::AttachmentReference depthAttachRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	
	//-- SUBPASSES
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	subpass.pDepthStencilAttachment = &depthAttachRef;

	std::array<vk::AttachmentDescription, 2> renderPassAttachmentsDesc = { colorAttachment, depthAttachment };

	const vk::RenderPassCreateInfo renderPassInfo(vk::RenderPassCreateFlags(), renderPassAttachmentsDesc, subpass);

	m_vkForwardRenderingRenderPass = m_pVulkanDevice->GetDevice().createRenderPass(renderPassInfo);

	LOG_INFO("Forward Renderpass created");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateFramebuffers() const
{
	UT_ASSERT_NULL(m_pFramebuffer, "CreateFrameBuffers()-->VulkanFramebuffer class object not valid?!");
	UT_ASSERT_NULL(m_vkForwardRenderingRenderPass, "CreateFrameBuffers()-->vk::RenderPass not valid?!");

	m_pFramebuffer->CreateFramebuffers(m_pVulkanDevice, m_vkForwardRenderingRenderPass);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateCommandbuffers() const
{
	UT_ASSERT_NULL(m_pFramebuffer, "CreateCommandbuffers()-->VulkanFramebuffer class object not valid?!");

	m_pVulkanDevice->CreateCommandBuffers(m_pFramebuffer);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::RecordCommands(uint32_t currentImage) const
{
	// Information about how to begin each command buffer
	constexpr vk::CommandBufferBeginInfo cmdBufferBeginInfo = {};

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
	m_pVulkanDevice->BeginGraphicsCommandBuffer(currentImage, cmdBufferBeginInfo);

	// Begin RenderPass
	m_pVulkanDevice->BeginRenderPass(currentImage, renderPassBeginInfo);

	// Bind Rendering pipeline
	m_pVulkanDevice->BindPipeline(currentImage, vk::PipelineBindPoint::eGraphics, m_vkForwardRenderingPipeline);
	
	m_pScene->Render(m_pVulkanDevice, currentImage);

	m_pGUI->BeginRender();
	m_pGUI->Render();
	m_pGUI->EndRender(m_pVulkanDevice, currentImage);

	// End RenderPass
	m_pVulkanDevice->EndRenderPass(currentImage);
	
	// end recording...
	m_pVulkanDevice->EndGraphicsCommandBuffer(currentImage);

	// Update uniforms!
	m_pScene->UpdateUniforms(m_pVulkanDevice, currentImage);
}
