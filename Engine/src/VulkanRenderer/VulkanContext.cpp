
#include "UltimateEnginePCH.h"
#include "VulkanContext.h"
#include "GLFW/glfw3.h"
#include "../EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanContext::VulkanContext()
{
	pWindow = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanContext::~VulkanContext()
{
	pWindow = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
vk::ShaderModule VulkanContext::CreateShaderModule(const std::string& fileName) const
{
	// start reading at the end & in binary mode.
	// Advantage of reading file from the end is we can use read position to determine
	// size of the file & allocate buffer accordingly!
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		LOG_ERROR("Failed to open Shader file!");

	// get the file size & allocate buffer memory!
	size_t fileSize = (size_t)file.tellg();
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

	vk::ShaderModule shaderModule;
	std::string shaderModuleName = fileName;

	shaderModule = vkDevice.createShaderModule(shaderModuleInfo);

	return shaderModule;
}

//-----------------------------------------------------------------------------------------------------------------------
//--- Find suitable memory type based on allowed type & property flags
uint32_t VulkanContext::FindMemoryTypeIndex(uint32_t allowedTypeIndex, vk::MemoryPropertyFlags props) const
{
	for (uint32_t i = 0; i < vkDeviceMemoryProps.memoryTypeCount; i++)
	{
		if ((allowedTypeIndex & (1 << i))												// Index of memory type must match corresponding bit in allowed types!
			&& (vkDeviceMemoryProps.memoryTypes[i].propertyFlags & props) == props)		// Desired property bit flags are part of the memory type's property flags!
		{
			// This memory type is valid, so return index!
			return i;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
vk::Format VulkanContext::ChooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags) const
{
	for (vk::Format format : formats)
	{
		// Get properties for given formats on this device
		vk::FormatProperties props;
		vkPhysicalDevice.getFormatProperties(format, &props);

		// depending on tiling choice, need to check for different bit flag
		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}

	LOG_ERROR("Failed to find matching format!");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanContext::CreateImageView2D(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView* imageView) 
{
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

	//vkDevice.createImageView(createInfo, nullptr, imageView);

	vk::ImageViewCreateInfo* pCreateInfo = &createInfo;
	vkDevice.createImageView(pCreateInfo, nullptr, imageView);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanContext::CopyBufferToImage(vk::Buffer srcBuffer, uint32_t width, uint32_t height, vk::Image* image)
{
	// Create buffer
	//VkCommandBuffer transferCommandBuffer = BeginCommandBuffer();
	//
	//VkBufferImageCopy imgRegion = {};
	//imgRegion.bufferOffset = 0;
	//imgRegion.bufferRowLength = 0;
	//imgRegion.bufferImageHeight = 0;
	//imgRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//imgRegion.imageSubresource.mipLevel = 0;
	//imgRegion.imageSubresource.baseArrayLayer = 0;
	//imgRegion.imageSubresource.layerCount = 1;
	//imgRegion.imageOffset = { 0,0,0 };
	//imgRegion.imageExtent = { width, height, 1 };
	//
	//// copy buffer to given image!
	//vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgRegion);
	//
	//EndAndSubmitCommandBuffer(transferCommandBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanContext::CreateImage2D(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image* pImage, vk::DeviceMemory* pDeviceMemory)
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
	vk::ImageCreateInfo* pImageInfo = &imageInfo;
	vkDevice.createImage(pImageInfo, nullptr, pImage);

	// Get memory requirements for the image...
	vk::MemoryRequirements imgMemReqs = vkDevice.getImageMemoryRequirements(*pImage);

	// Allocate memory using requirements & user defined properties...	
	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.allocationSize = imgMemReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(imgMemReqs.memoryTypeBits, memoryPropertyFlags);

	vk::MemoryAllocateInfo* pMemAllocInfo = &memAllocInfo;
	vkDevice.allocateMemory(pMemAllocInfo, nullptr, pDeviceMemory);
	vkDevice.bindImageMemory(*pImage, *pDeviceMemory, 0);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanContext::CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memFlags, vk::Buffer* outBuffer, vk::DeviceMemory* outMemory)
{
	// Buffer creation info!
	vk::BufferCreateInfo vbInfo;
	vbInfo.size = bufferSize;
	vbInfo.usage = usageFlags;
	vbInfo.sharingMode = vk::SharingMode::eExclusive;

	vk::BufferCreateInfo* pVbInfo = &vbInfo;
	vkDevice.createBuffer(pVbInfo, nullptr, outBuffer);

	// Buffer's memory requirements!
	vk::MemoryRequirements memReq = vkDevice.getBufferMemoryRequirements(*outBuffer);

	// Allocate memory to buffer!
	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(memReq.memoryTypeBits, memFlags);

	vk::MemoryAllocateInfo* pMemAllocInfo = &memAllocInfo;
	vkDevice.allocateMemory(pMemAllocInfo, nullptr, outMemory);
	
	// Bind memory to given Vertex buffer
	vkDevice.bindBufferMemory(*outBuffer, *outMemory, 0);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanContext::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize bufferSize)
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
VkCommandBuffer VulkanContext::BeginCommandBuffer() const
{
	// Command buffer to hold transfer command
	vk::CommandBuffer commandBuffer;

	// Command buffer details
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = vkGraphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	commandBuffer = vkDevice.allocateCommandBuffers(allocInfo).front();

	// Information to begin the command buffer record!
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;	// We are only using the command buffer once, so set for one time submit!

	// Begin recording transfer commands
	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanContext::EndAndSubmitCommandBuffer(vk::CommandBuffer commandBuffer) const
{
	// End Commands!
	commandBuffer.end();

	// Queue submission information
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Submit transfer command to transfer queue (which is same as Graphics Queue) & wait until it finishes!
	vk::Fence commandFence = vkDevice.createFence({});
	vkQueueGraphics.submit(submitInfo, commandFence);
	vkQueueGraphics.waitIdle();

	vkDevice.freeCommandBuffers(vkGraphicsCommandPool, commandBuffer);
}



