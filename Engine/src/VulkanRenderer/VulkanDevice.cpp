#include "UltimateEnginePCH.h"
#include "../EngineHeader.h"
#include "VulkanDevice.h"
#include "VulkanGlobals.h"
#include "VulkanFramebuffer.h"
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
bool VulkanDevice::SetupDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	CHECK(AcquirePhysicalDevice(vkInst, vkSurface));
	CHECK(CreateLogicalDevice());

	return true;
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

	LOG_DEBUG("Window Resize ======> Command buffer destroyed");
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
vk::CommandBuffer VulkanDevice::BeginTransferCommandBuffer() const
{
	// Command buffer details
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = m_vkGraphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	const vk::CommandBuffer cmdBuffer = m_vkDevice.allocateCommandBuffers(allocInfo).front();

	// Information to begin the command buffer record!
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;	// We are only using the command buffer once, so set for one time submit!

	// Begin recording transfer commands
	cmdBuffer.begin(beginInfo);

	return cmdBuffer;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::EndAndSubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer) const
{
	// End Commands!
	commandBuffer.end();

	// Queue submission information
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Submit transfer command to transfer queue (which is same as Graphics Queue) & wait until it finishes!
	const vk::Fence commandFence = m_vkDevice.createFence({});
	m_vkQueueGraphics.submit(submitInfo, commandFence);
	m_vkQueueGraphics.waitIdle();

	m_vkDevice.freeCommandBuffers(m_vkGraphicsCommandPool, commandBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::BindPipeline(uint32_t imageIndex, vk::PipelineBindPoint bindPoint, vk::Pipeline pipeline) const
{
	m_vkListGraphicsCommandBuffers[imageIndex].bindPipeline(bindPoint, pipeline);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize bufferSize) const
{
	vk::CommandBuffer cmdBuffer = BeginTransferCommandBuffer();

	vk::BufferCopy bufferCopyRegion;
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	cmdBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	EndAndSubmitTransferCommandBuffer(cmdBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
// List out all the available physical devices. Choose the one which supports required Queue families & extensions. 
// Give preference to Discrete GPU over Integrated GPU!
//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::AcquirePhysicalDevice(vk::Instance vkInst, vk::SurfaceKHR vkSurface)
{
	uint32_t deviceCount = 0;

	std::vector<vk::PhysicalDevice> physicalDevices = vkInst.enumeratePhysicalDevices();

	CHECK_LOG(!physicalDevices.empty(), "Can't find GPUs supporting Vulkan!!!");

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
			vkDeviceProps = iter->getProperties();
		}
		else
		{
			LOG_WARNING("{0} device rejected since it's not a Discrete GPU", iter->getProperties().deviceName);
		}
	}

	CHECK_LOG(m_vkPhysicalDevice, "Failed to find suitable GPU!!!");

	// Check if Discrete GPU we found has all the needed extension support!

	if (bool bExtensionsSupported = CheckDeviceExtensionSupport())
	{
		// Fetch Queue families supported!
		FetchQueueFamilies(vkSurface);

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

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CreateLogicalDevice()
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

	// std::set allows only One unique value for input values, no duplicate is allowed, so if both Graphics Queue family
	// and Presentation Queue family index is same then it will avoid the duplicates and assign only one queue index!
	const std::set<uint32_t> uniqueQueueFamilies =
	{
		m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value()
	};

	constexpr float queuePriority = 1.0f;

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
	const vk::PhysicalDeviceFeatures deviceFeatures = m_vkPhysicalDevice.getFeatures();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create logical device from the given physical device...
	m_vkDevice = m_vkPhysicalDevice.createDevice(deviceCreateInfo);
	LOG_DEBUG("Vulkan Logical device created!");

	// Queues are created at the same time as device creation, store their handle!
	m_vkQueueGraphics = m_vkDevice.getQueue(m_QueueFamilyIndices.graphicsFamily.value(), 0);
	m_vkQueuePresent = m_vkDevice.getQueue(m_QueueFamilyIndices.presentFamily.value(), 0);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::FetchQueueFamilies(vk::SurfaceKHR vkSurface)
{
	// Get all queue families & their properties supported by physical device!
	std::vector<vk::QueueFamilyProperties> queueFamilyProps = m_vkPhysicalDevice.getQueueFamilyProperties();

	// Go through queue families and check if it supports graphics & present family queue!
	uint32_t i = 0;
	std::vector<vk::QueueFamilyProperties>::iterator iter = queueFamilyProps.begin();
	for (; iter != queueFamilyProps.end(); ++iter)
	{
		if ((*iter).queueFlags & vk::QueueFlagBits::eGraphics)
		{
			m_QueueFamilyIndices.graphicsFamily = i;
		}

		// check if this queue family has capability of presenting to our window surface!
		VkBool32 bPresentSupport = m_vkPhysicalDevice.getSurfaceSupportKHR(i, vkSurface);

		// if yes, store presentation family queue index!
		if (bPresentSupport)
		{
			m_QueueFamilyIndices.presentFamily = i;
		}

		if (m_QueueFamilyIndices.isComplete())
			break;

		++i;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CheckDeviceExtensionSupport() const
{
	// Get count of total number of extensions
	std::vector<vk::ExtensionProperties> vecSupportedExtensions = m_vkPhysicalDevice.enumerateDeviceExtensionProperties();

	// Compare Required extensions with supported extensions...
	for (int i = 0; i < UT::VkGlobals::GListDeviceExtensions.size(); ++i)
	{
		bool bExtensionFound = false;

		for (int j = 0; j < vecSupportedExtensions.size(); ++j)
		{
			// If device supported extensions matches the one we want, good news ... Enumarate them!
			if (strcmp(UT::VkGlobals::GListDeviceExtensions[i], vecSupportedExtensions[j].extensionName) == 0)
			{
				bExtensionFound = true;

				std::string msg = std::string(UT::VkGlobals::GListDeviceExtensions[i]) + " device extension found!";
				LOG_DEBUG(msg.c_str());

				break;
			}
		}

		// No matching extension found ... bail out!
		if (!bExtensionFound)
		{
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
vk::ShaderModule VulkanDevice::CreateShaderModule(const std::string& fileName) const
{
	// start reading at the end & in binary mode.
	// Advantage of reading file from the end is we can use read position to determine
	// size of the file & allocate buffer accordingly!
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		LOG_ERROR("Failed to open Shader file!");

	// get the file size & allocate buffer memory!
	const size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// now seek back to the beginning of the file & read all bytes at once!
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// close the file!
	file.close();

	// Create Shader Module
	vk::ShaderModuleCreateInfo shaderModuleInfo;
	shaderModuleInfo.codeSize = buffer.size();
	shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	std::string shaderModuleName = fileName;

	const vk::ShaderModule shaderModule = m_vkDevice.createShaderModule(shaderModuleInfo);

	return shaderModule;
}

//-----------------------------------------------------------------------------------------------------------------------
uint32_t VulkanDevice::FindMemoryTypeIndex(uint32_t allowedTypeIndex, vk::MemoryPropertyFlags props) const
{
	for (uint32_t i = 0; i < m_vkPhysicalDevice.getMemoryProperties().memoryTypeCount; i++)
	{
		if ((allowedTypeIndex & (1 << i))																		// Index of memory type must match corresponding bit in allowed types!
			&& (m_vkPhysicalDevice.getMemoryProperties().memoryTypes[i].propertyFlags & props) == props)		// Desired property bit flags are part of the memory type's property flags!
		{
			// This memory type is valid, so return index!
			return i;
		}
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
vk::Format VulkanDevice::ChooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags) const
{
	for (const vk::Format format : formats)
	{
		// Get properties for given formats on this device
		vk::FormatProperties props;
		m_vkPhysicalDevice.getFormatProperties(format, &props);

		// depending on tiling choice, need to check for different bit flag
		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else
		{
			LOG_ERROR("Failed to find matching format!");
			return vk::Format::eUndefined;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CopyBufferToImage(vk::Buffer srcBuffer, uint32_t width, uint32_t height, vk::Image* image) const
{
	// Create buffer
	const vk::CommandBuffer transferCommandBuffer = BeginTransferCommandBuffer();

	vk::BufferImageCopy imgRegion = {};
	imgRegion.bufferOffset = 0;
	imgRegion.bufferRowLength = 0;
	imgRegion.bufferImageHeight = 0;
	imgRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgRegion.imageSubresource.mipLevel = 0;
	imgRegion.imageSubresource.baseArrayLayer = 0;
	imgRegion.imageSubresource.layerCount = 1;
	imgRegion.imageOffset = VkOffset3D{ 0,0,0 };
	imgRegion.imageExtent = VkExtent3D{ width, height, 1 };
	
	// copy buffer to given image!
	transferCommandBuffer.copyBufferToImage(srcBuffer, *image, vk::ImageLayout::eTransferDstOptimal, imgRegion);
		
	EndAndSubmitTransferCommandBuffer(transferCommandBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateImage2D(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
								vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags,
								vk::ImageAspectFlags aspectFlags, UT::VkStructs::VulkanImage* pOutImage2D) const 

{
	// Image creation info!
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.usage = usageFlags;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;

	// Create Image!
	vk::Image image = m_vkDevice.createImage(imageInfo);

	// Get memory requirements for the image...
	const vk::MemoryRequirements imgMemReqs = m_vkDevice.getImageMemoryRequirements(image);

	// Allocate memory using requirements & user defined properties...	
	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.allocationSize = imgMemReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(imgMemReqs.memoryTypeBits, memoryPropertyFlags);

	const vk::DeviceMemory deviceMemory = m_vkDevice.allocateMemory(memAllocInfo);
	m_vkDevice.bindImageMemory(image, deviceMemory, 0);

	// Image View Creation
	vk::ImageViewCreateInfo createInfo;

	createInfo.format = format;
	createInfo.image = image;
	createInfo.viewType = vk::ImageViewType::e2D;
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	const vk::ImageView imgView = m_vkDevice.createImageView(createInfo);

	// Fill out output image params!
	pOutImage2D->image = image;
	pOutImage2D->deviceMemory = deviceMemory;
	pOutImage2D->extent.width = width;
	pOutImage2D->extent.height = height;
	pOutImage2D->format = format;
	pOutImage2D->imageView = imgView;
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanDevice::TransitionImageLayout(vk::Image srcImage, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer cmdBuffer) const
{
	vk::CommandBuffer commandBuffer;

	if (cmdBuffer != VK_NULL_HANDLE)
		commandBuffer = cmdBuffer;
	else
		commandBuffer = BeginTransferCommandBuffer();

	vk::ImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.oldLayout = oldLayout;											// Layout to transition from
	imageMemoryBarrier.newLayout = newLayout;											// Layout to transition to
	imageMemoryBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;					// Queue family to transition from
	imageMemoryBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;					// Queue family to transition to
	imageMemoryBarrier.image = srcImage;												// Image being accessed & modified as a part of barrier
	imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;	// Aspect of image being altered
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;								// First mip level to start alteration on
	imageMemoryBarrier.subresourceRange.levelCount = 1;									// Number of mip levels to alter starting from base mip level
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;								// First layer of start alterations on
	imageMemoryBarrier.subresourceRange.layerCount = 1;									// Number of layers to alter starting from base array layer

	vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands;
	vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands;

	// Credit = Sascha Willems : VulkanTools.cpp!
	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	// ? Added srcStage logic too, only where it's required! 
	switch (oldLayout)
	{
		case vk::ImageLayout::eUndefined:
		{
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		}

		case vk::ImageLayout::ePreinitialized:	
		{
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
			break;
		}

		case vk::ImageLayout::eColorAttachmentOptimal:
		{
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		}

		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		{
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		}

		case vk::ImageLayout::eTransferSrcOptimal:
		{
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		}

		case vk::ImageLayout::eTransferDstOptimal:
		{
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		}

		case vk::ImageLayout::eShaderReadOnlyOptimal:
		{
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		}

		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout)
	{
		case vk::ImageLayout::eTransferDstOptimal:
		{
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		}

		case vk::ImageLayout::eTransferSrcOptimal:
		{
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
			break;
		}

		case vk::ImageLayout::eColorAttachmentOptimal:
		{
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		}

		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		{
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		}

		case vk::ImageLayout::eShaderReadOnlyOptimal:
		{
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == vk::AccessFlagBits::eNone)
			{
				imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
			}

			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;

			break;
		}

		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	vk::DependencyFlags flags = {};
	//cmdBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, &imageMemoryBarrier);
	commandBuffer.pipelineBarrier(srcStage, dstStage, flags, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	EndAndSubmitTransferCommandBuffer(commandBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memFlags,
								UT::VkStructs::VulkanBuffer* pOutBuffer) const
{
	// Buffer creation info!
	vk::BufferCreateInfo vbInfo;
	vbInfo.size = bufferSize;
	vbInfo.usage = usageFlags;
	vbInfo.sharingMode = vk::SharingMode::eExclusive;

	vk::Buffer outBuffer = m_vkDevice.createBuffer(vbInfo);

	// Buffer's memory requirements!
	vk::MemoryRequirements memReq = m_vkDevice.getBufferMemoryRequirements(outBuffer);

	// Allocate memory to buffer!
	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(memReq.memoryTypeBits, memFlags);

	vk::DeviceMemory outMemory = m_vkDevice.allocateMemory(memAllocInfo);

	// Bind memory to given Vertex buffer
	m_vkDevice.bindBufferMemory(outBuffer, outMemory, 0);

	// Output buffer!
	pOutBuffer->buffer = outBuffer;
	pOutBuffer->deviceMemory = outMemory;
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanDevice::BeginGraphicsCommandBuffer(uint32_t imageIndex, vk::CommandBufferBeginInfo cmdBufferBeginInfo) const
{
	if (imageIndex > m_vkListGraphicsCommandBuffers.size())
	{
		LOG_CRITICAL("Command buffer image index out of bound!");
		return;
	}
		
	m_vkListGraphicsCommandBuffers.at(imageIndex).begin(cmdBufferBeginInfo);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanDevice::EndGraphicsCommandBuffer(uint32_t imageIndex) const
{
	if (imageIndex > m_vkListGraphicsCommandBuffers.size())
	{
		LOG_CRITICAL("Command buffer image index out of bound!");
		return;
	}

	m_vkListGraphicsCommandBuffers.at(imageIndex).end();
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanDevice::BeginRenderPass(uint32_t imageIndex, vk::RenderPassBeginInfo renderPassInfo) const
{
	if (imageIndex > m_vkListGraphicsCommandBuffers.size())
	{
		LOG_CRITICAL("Command buffer image index out of bound!");
		return;
	}

	m_vkListGraphicsCommandBuffers.at(imageIndex).beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanDevice::EndRenderPass(uint32_t imageIndex) const
{
	if (imageIndex > m_vkListGraphicsCommandBuffers.size())
	{
		LOG_CRITICAL("Command buffer image index out of bound!");
		return;
	}

	m_vkListGraphicsCommandBuffers.at(imageIndex).endRenderPass();
}




