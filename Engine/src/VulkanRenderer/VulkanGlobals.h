#pragma once

#include "../UltimateEnginePCH.h"
#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"
#include "VulkanHeaders.h"

namespace UT
{
	namespace VkGlobals
	{
		const uint16_t GMaxFramesDraws = 3;
		const uint64_t GFenceTimeout = 100000000;

		//--- list of device extensions
		const std::vector<const char*> GListDeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		//---------------------------------------------------------------------------------------------------------------------
		struct VulkanImage
		{
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
	}
}