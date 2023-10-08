#include "UltimateEnginePCH.h"
#include "../EngineHeader.h"
#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanGlobals.h"
#include "VulkanFramebuffer.h"
#include "VulkanHeaders.h"
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::VulkanDevice()
{
}

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::~VulkanDevice()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::SetupDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	AcquirePhysicalDevice(vkInst, vkSurface);
	CreateLogicalDevice();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::RecreateOnWindowResize()
{
	// re-create swapchain
	//pRC->CreateSwapchain();
	//pRC->CreateCommandBuffers();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::Cleanup()
{
	m_vkDevice.freeCommandBuffers(m_vkGraphicsCommandPool, m_vkListGraphicsCommandBuffers);
	m_vkDevice.destroyCommandPool(m_vkGraphicsCommandPool);

	m_vkListGraphicsCommandBuffers.clear();

	//pRC->CleanupCommandBuffers();
	//
	//pRC->vkDevice.destroySwapchainKHR(pRC->vkSwapchain);
	//pRC->vkDevice.destroy();	
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CleanupOnWindowsResize()
{
	m_vkDevice.freeCommandBuffers(m_vkGraphicsCommandPool, m_vkListGraphicsCommandBuffers);
	m_vkListGraphicsCommandBuffers.clear();

	//pRC->vkDevice.destroySwapchainKHR(pRC->vkSwapchain);
	//pRC->vkDevice.freeCommandBuffers(pRC->vkGraphicsCommandPool, pRC->vkListGraphicsCommandBuffers);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateCommandBuffers(const VulkanFramebuffer* pFrameBuffer)
{
	// Create command pool!
	vk::CommandPoolCreateInfo poolInfo = {};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value();

	m_vkGraphicsCommandPool = m_vkDevice.createCommandPool(poolInfo);

	LOG_INFO("Graphics command pool created");

	// Create command buffers!
	if (pFrameBuffer->GetFramebufferCount() <= 0)
		LOG_ERROR("Framebuffers needs to be created before creating command buffers!");

	// Make sure we have command buffer for each framebuffer!
	m_vkListGraphicsCommandBuffers.resize(pFrameBuffer->GetFramebufferCount());

	// Allocate buffer from the Graphics command pool
	vk::CommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.commandPool = m_vkGraphicsCommandPool;
	cbAllocInfo.level = vk::CommandBufferLevel::ePrimary;
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(m_vkListGraphicsCommandBuffers.size());

	m_vkListGraphicsCommandBuffers = m_vkDevice.allocateCommandBuffers(cbAllocInfo);

	LOG_INFO("Graphics command buffer created");
}

//---------------------------------------------------------------------------------------------------------------------
VkCommandBuffer VulkanDevice::BeginCommandBuffer() const
{
	// Command buffer to hold transfer command
	vk::CommandBuffer commandBuffer;

	// Command buffer details
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = m_vkGraphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	commandBuffer = m_vkDevice.allocateCommandBuffers(allocInfo).front();

	// Information to begin the command buffer record!
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;	// We are only using the command buffer once, so set for one time submit!

	// Begin recording transfer commands
	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::EndAndSubmitCommandBuffer(vk::CommandBuffer commandBuffer) const
{
	// End Commands!
	commandBuffer.end();

	// Queue submission information
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Submit transfer command to transfer queue (which is same as Graphics Queue) & wait until it finishes!
	vk::Fence commandFence = m_vkDevice.createFence({});
	m_vkQueueGraphics.submit(submitInfo, commandFence);
	m_vkQueueGraphics.waitIdle();

	m_vkDevice.freeCommandBuffers(m_vkGraphicsCommandPool, commandBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize bufferSize) const
{
	vk::CommandBuffer cmdBuffer = BeginCommandBuffer();

	vk::BufferCopy bufferCopyRegion;
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	cmdBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	EndAndSubmitCommandBuffer(cmdBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
// List out all the available physical devices. Choose the one which supports required Queue families & extensions. 
// Give preference to Discrete GPU over Integrated GPU!
//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::AcquirePhysicalDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	uint32_t deviceCount = 0;

	std::vector<vk::PhysicalDevice> physicalDevices = vkInst.enumeratePhysicalDevices();

	UT_ASSERT_BOOL((!physicalDevices.empty()), "Can't find GPUs supporting Vulkan!!!");

	// List out all the physical devices & get their properties
	vk::PhysicalDeviceProperties	vkDeviceProps;
	std::vector<vk::PhysicalDevice>::iterator iter = physicalDevices.begin();
	for (; iter != physicalDevices.end(); ++iter)
	{
		LOG_INFO("{0} Detected", (*iter).getProperties().deviceName);

		// Prefer Discrete GPU over integrated one!
		if ((*iter).getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			m_vkPhysicalDevice = (*iter);
			vkDeviceProps = (*iter).getProperties();
		}
		else
		{
			LOG_WARNING("{0} device rejected since it's not a Discrete GPU", (*iter).getProperties().deviceName);
		}
	}

	UT_ASSERT_NULL(m_vkPhysicalDevice, "Failed to find suitable GPU!!!");

	// Check if Discrete GPU we found has all the needed extension support!
	bool bExtensionsSupported = UT::VkUtility::CheckDeviceExtensionSupport(m_vkPhysicalDevice);

	if (bExtensionsSupported)
	{
		// Fetch Queue families supported!
		UT::VkUtility::FetchQueueFamilies(m_vkPhysicalDevice, vkSurface, m_QueueFamilyIndices);

		if (m_QueueFamilyIndices.isComplete())
		{
			LOG_INFO("{0} Selected!!", vkDeviceProps.deviceName);
			LOG_DEBUG("---------- Device Limits ----------");
			LOG_DEBUG("Max Color Attachments: {0}", vkDeviceProps.limits.maxColorAttachments);
			LOG_DEBUG("Max Descriptor Set Samplers: {0}", vkDeviceProps.limits.maxDescriptorSetSamplers);
			LOG_DEBUG("Max Descriptor Set Uniform Buffers: {0}", vkDeviceProps.limits.maxDescriptorSetUniformBuffers);
			LOG_DEBUG("Max Framebuffer Height: {0}", vkDeviceProps.limits.maxFramebufferHeight);
			LOG_DEBUG("Max Framebuffer Width: {0}", vkDeviceProps.limits.maxFramebufferWidth);
			LOG_DEBUG("Max Push Constant Size: {0}", vkDeviceProps.limits.maxPushConstantsSize);
			LOG_DEBUG("Max Uniform Buffer Range: {0}", vkDeviceProps.limits.maxUniformBufferRange);
			LOG_DEBUG("Max Vertex Input Attributes: {0}", vkDeviceProps.limits.maxVertexInputAttributes);
			LOG_DEBUG("----------------------------------");
		}
	}

}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateLogicalDevice()
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

	// std::set allows only One unique value for input values, no duplicate is allowed, so if both Graphics Queue family
	// and Presentation Queue family index is same then it will avoid the duplicates and assign only one queue index!
	std::set<uint32_t> uniqueQueueFamilies =
	{
		m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value()
	};

	float queuePriority = 1.0f;

	for (uint32_t queueFamily = 0; queueFamily < uniqueQueueFamilies.size(); ++queueFamily)
	{
		// Queue the logical device needs to create & the info to do so!
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information needed to create logical device!
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(UT::VkGlobals::GListDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = UT::VkGlobals::GListDeviceExtensions.data();

	// Physical device features that logical device will use...
	vk::PhysicalDeviceFeatures deviceFeatures = m_vkPhysicalDevice.getFeatures();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create logical device from the given physical device...
	m_vkDevice = m_vkPhysicalDevice.createDevice(deviceCreateInfo);

	LOG_DEBUG("Vulkan Logical device created!");

	// Queues are created at the same time as device creation, store their handle!
	m_vkQueueGraphics = m_vkDevice.getQueue(m_QueueFamilyIndices.graphicsFamily.value(), 0);
	m_vkQueuePresent = m_vkDevice.getQueue(m_QueueFamilyIndices.presentFamily.value(), 0);
}




