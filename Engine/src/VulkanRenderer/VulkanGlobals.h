#pragma once

#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"
#include "../EngineHeader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace UT
{
	namespace VkGlobals
	{
		constexpr uint16_t		GMaxFramesDraws = 3;
		constexpr uint64_t		GFenceTimeout = 100000000;
		
		inline glm::vec2		GCurrentResolution = glm::vec2(0, 0);

		//--- list of device extensions
		const std::vector<const char*> GListDeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	}

	namespace VkStructs
	{
		//---------------------------------------------------------------------------------------------------------------------
		struct VulkanImage
		{
			VulkanImage()
			{
				image = nullptr;
				imageView = nullptr;
				deviceMemory = nullptr;
			}

			vk::Image			image;
			vk::ImageView		imageView;
			vk::DeviceMemory	deviceMemory;

			vk::Format			format;
			vk::Extent2D		extent;

			void	DestroyAll(vk::Device device)
			{
				device.destroyImageView(imageView);
				device.destroyImage(image);
				device.freeMemory(deviceMemory);
			}

			void	DestroyImageView(vk::Device device)
			{
				device.destroyImageView(imageView);
			}
		};

		//---------------------------------------------------------------------------------------------------------------------
		struct VulkanBuffer
		{
			VulkanBuffer()
			{
				buffer = nullptr;
				deviceMemory = nullptr;
			}

			vk::Buffer			buffer;
			vk::DeviceMemory	deviceMemory;

			void	DestroyAll(vk::Device device)
			{
				device.destroyBuffer(buffer);
				device.freeMemory(deviceMemory);
			}
		};
	}

	namespace VkUtility
	{
		//---------------------------------------------------------------------------------------------------------------------
		inline UT_API bool CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions)
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
	}
}

