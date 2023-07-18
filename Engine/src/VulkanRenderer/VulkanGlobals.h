#pragma once

#include "../Core/Core.h"
#include "../UltimateEnginePCH.h"

#include "Volk/volk.h"

namespace UT
{
	namespace VK
	{
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
			VkSurfaceCapabilitiesKHR		surfaceCapabilities;
			std::vector<VkSurfaceFormatKHR> surfaceFormats;
			std::vector<VkPresentModeKHR>	surfacePresentModes;

			bool isValid() const { return !surfaceFormats.empty() && !surfacePresentModes.empty(); }
		};
	}
}