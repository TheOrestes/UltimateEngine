#pragma once
#include "../UltimateEnginePCH.h"
#include "../Core/Core.h"
#include "../EngineHeader.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.hpp"
#include "VulkanGlobals.h"

namespace UT
{
	namespace VkUtility
	{
		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline bool CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions)
		{
			bool hasExtension = false;
			std::vector<vk::ExtensionProperties> vecExtensions = vk::enumerateInstanceExtensionProperties();
			
#if defined _DEBUG
			// Enumerate all the extensions supported by the vulkan instance.
			// Ideally, this list should contain extensions requested by GLFW and
			// few additional ones!

			for (uint32_t i = 0; i < vecExtensions.size(); ++i)
			{
				std::string str(std::begin(vecExtensions[i].extensionName), std::end(vecExtensions[i].extensionName));
				LOG_DEBUG(vecExtensions[i].extensionName.data(), " extension available!");
			}

#endif

			// Check if given extensions are in the list of available extensions
			for (uint32_t i = 0; i < vecExtensions.size(); i++)
			{
				for (uint32_t j = 0; j < instanceExtensions.size(); j++)
				{
					if (!strcmp(vecExtensions[i].extensionName, instanceExtensions[j]))
					{
						hasExtension = true;
						break;
					}
				}
			}

			return hasExtension;
		}

		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline void FetchQueueFamilies(vk::PhysicalDevice vkPhysicalDevice, vk::SurfaceKHR vkSurface, QueueFamilyIndices& queueFamilyIndices)
		{
			// Get all queue families & their properties supported by physical device!
			std::vector<vk::QueueFamilyProperties> queueFamilyProps = vkPhysicalDevice.getQueueFamilyProperties();

			// Go through queue families and check if it supports graphics & present family queue!
			uint32_t i = 0;
			std::vector<vk::QueueFamilyProperties>::iterator iter = queueFamilyProps.begin();
			for (; iter != queueFamilyProps.end(); ++iter)
			{
				if ((*iter).queueFlags & vk::QueueFlagBits::eGraphics)
				{
					queueFamilyIndices.graphicsFamily = i;
				}

				// check if this queue family has capability of presenting to our window surface!
				VkBool32 bPresentSupport = vkPhysicalDevice.getSurfaceSupportKHR(i, vkSurface);

				// if yes, store presentation family queue index!
				if (bPresentSupport)
				{
					queueFamilyIndices.presentFamily = i;
				}

				if (queueFamilyIndices.isComplete())
					break;

				++i;
			}
		}

		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline bool CheckDeviceExtensionSupport(vk::PhysicalDevice vkPhysicalDevice)
		{
			// Get count of total number of extensions
			std::vector<vk::ExtensionProperties> vecSupportedExtensions;
			vecSupportedExtensions = vkPhysicalDevice.enumerateDeviceExtensionProperties();

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
		UT_API inline vk::ShaderModule CreateShaderModule(vk::Device vkDevice, const std::string& fileName)
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
		UT_API inline uint32_t FindMemoryTypeIndex(vk::PhysicalDevice vkPhysicalDevice, uint32_t allowedTypeIndex, vk::MemoryPropertyFlags props)
		{
			for (uint32_t i = 0; i < vkPhysicalDevice.getMemoryProperties().memoryTypeCount; i++)
			{
				if ((allowedTypeIndex & (1 << i))																	// Index of memory type must match corresponding bit in allowed types!
					&& (vkPhysicalDevice.getMemoryProperties().memoryTypes[i].propertyFlags & props) == props)		// Desired property bit flags are part of the memory type's property flags!
				{
					// This memory type is valid, so return index!
					return i;
				}
			}

			return 0;
		}

		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline vk::Format ChooseSupportedFormat(vk::PhysicalDevice vkPhysicalDevice, const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags)
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
				else
					return vk::Format::eUndefined;
			}

			LOG_ERROR("Failed to find matching format!");
		}

		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline vk::ImageView CreateImageView2D(vk::Device vkDevice, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
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

			vk::ImageView imgView = vkDevice.createImageView(createInfo);

			return imgView;
		}

		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline void CopyBufferToImage(vk::Buffer srcBuffer, uint32_t width, uint32_t height, vk::Image* image)
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
		UT_API inline std::tuple<vk::Image, vk::DeviceMemory> CreateImage2D(vk::Device vkDevice, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags)
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
			vk::Image image = vkDevice.createImage(imageInfo);

			// Get memory requirements for the image...
			vk::MemoryRequirements imgMemReqs = vkDevice.getImageMemoryRequirements(image);

			// Allocate memory using requirements & user defined properties...	
			vk::MemoryAllocateInfo memAllocInfo;
			memAllocInfo.allocationSize = imgMemReqs.size;
			memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(physicalDevice, imgMemReqs.memoryTypeBits, memoryPropertyFlags);

			vk::DeviceMemory deviceMemory = vkDevice.allocateMemory(memAllocInfo);
			vkDevice.bindImageMemory(image, deviceMemory, 0);

			return std::make_tuple(image, deviceMemory);
		}

		//---------------------------------------------------------------------------------------------------------------------
		UT_API inline std::tuple<vk::Buffer, vk::DeviceMemory> CreateBuffer(vk::Device vkDevice, vk::PhysicalDevice physicalDevice, vk::DeviceSize bufferSize, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memFlags)
		{
			// Buffer creation info!
			vk::BufferCreateInfo vbInfo;
			vbInfo.size = bufferSize;
			vbInfo.usage = usageFlags;
			vbInfo.sharingMode = vk::SharingMode::eExclusive;

			vk::Buffer outBuffer = vkDevice.createBuffer(vbInfo);

			// Buffer's memory requirements!
			vk::MemoryRequirements memReq = vkDevice.getBufferMemoryRequirements(outBuffer);

			// Allocate memory to buffer!
			vk::MemoryAllocateInfo memAllocInfo;
			memAllocInfo.allocationSize = memReq.size;
			memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(physicalDevice, memReq.memoryTypeBits, memFlags);

			vk::DeviceMemory outMemory = vkDevice.allocateMemory(memAllocInfo);

			// Bind memory to given Vertex buffer
			vkDevice.bindBufferMemory(outBuffer, outMemory, 0);

			return std::make_tuple(outBuffer, outMemory);
		}
	}
}
