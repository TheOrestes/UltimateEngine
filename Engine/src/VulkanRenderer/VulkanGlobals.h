#pragma once

#include "../Core/Core.h"
#include "../UltimateEnginePCH.h"

#include "vulkan/vulkan.hpp"

namespace UT
{
	namespace VULKAN
	{
		//--- list of device extensions
		const std::vector<const char*> DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};


		//---------------------------------------------------------------------------------------------------------------------
		struct QueueFamilyIndices
		{
			QueueFamilyIndices()
			{
				graphicsFamily.reset();
				presentFamily.reset();
			}

			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
		};

		//---------------------------------------------------------------------------------------------------------------------
		struct SwapchainInfo
		{
			vk::SurfaceCapabilitiesKHR		surfaceCapabilities;
			std::vector<vk::SurfaceFormatKHR> surfaceFormats;
			std::vector<vk::PresentModeKHR>	surfacePresentModes;

			bool isValid() const { return !surfaceFormats.empty() && !surfacePresentModes.empty(); }
		};
	}
}