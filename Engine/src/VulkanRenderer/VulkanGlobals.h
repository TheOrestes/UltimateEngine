#pragma once

#include "../UltimateEnginePCH.h"
#include "../Core/Core.h"
#include "vulkan/vulkan.hpp"
#include "../EngineHeader.h"
#include "glm/glm.hpp"

namespace UT
{
	namespace VkGlobals
	{
		const uint16_t		GMaxFramesDraws = 3;
		const uint64_t		GFenceTimeout = 100000000;

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

		//-----------------------------------------------------------------------------------------------------------------------
		// VERTEX STRUCTURES
		struct VertexPC
		{
			VertexPC() : Position(glm::vec3(0)), Color(glm::vec3(1)) {}
			VertexPC(const glm::vec3& pos, const glm::vec3& col) : Position(pos), Color(col) {}

			glm::vec3 Position;
			glm::vec3 Color;
		};

		struct VertexPT
		{
			VertexPT() : Position(glm::vec3(0)), UV(glm::vec2(0)) {}
			VertexPT(const glm::vec3& _pos, const glm::vec2& _uv) : Position(_pos), UV(_uv) {}

			glm::vec3 Position;
			glm::vec2 UV;
		};

		struct VertexPNTBT
		{
			VertexPNTBT() { Position = glm::vec3(0); Normal = glm::vec3(0); Tangent = glm::vec3(0); BiNormal = glm::vec3(0); UV = glm::vec2(0); }
			VertexPNTBT(const glm::vec3 _pos, const glm::vec3 _normal, const glm::vec3 _tangent, const glm::vec3& _binormal, const glm::vec2& _uv) :
				Position(_pos),
				Normal(_normal),
				Tangent(_tangent),
				BiNormal(_binormal),
				UV(_uv) {}

			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 BiNormal;
			glm::vec2 UV;
		};
	}

	namespace VkUtility
	{
		//---------------------------------------------------------------------------------------------------------------------
		inline bool CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions)
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

