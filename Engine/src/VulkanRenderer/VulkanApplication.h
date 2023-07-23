#pragma once

#include "../Core/Core.h"
#include "../EngineHeader.h"

//#define VOLK_IMPLEMENTATION
//#include "Volk/volk.h"

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

class EngineApplication;
class VulkanContext;
class VulkanRenderer;

class UT_API VulkanApplication : public EngineApplication
{
public:
	VulkanApplication();
	virtual ~VulkanApplication();

	virtual void				Cleanup() override;

	virtual void				Initialize(void* pWindow);
	virtual void				Update(float dt);
	virtual void				Render();

private:
	VulkanApplication(const VulkanApplication&);
	VulkanApplication& operator=(const VulkanApplication&);

	void						CreateInstance();
	void						CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions);
	void						SetupDebugMessenger();
	void						RunShaderCompiler(const std::string& directoryPath);
	void						PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
	VulkanContext*				m_pVKContext;
	VulkanRenderer*				m_pVulkanRenderer;
	VkDebugUtilsMessengerEXT	m_vkDebugMessenger;

	bool						m_bEnableValidation;

	//-----------------------------------------------------------------------------------------------------------------
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
		VkDebugUtilsMessageTypeFlagsEXT msgType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		LOG_ERROR("----------------------------------------------------------------------------------------------------");
	
		if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			LOG_DEBUG("Validation Layer: {0}", pCallbackData->pMessage);
		}
		else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_WARNING("Validation Layer: {0}", pCallbackData->pMessage);
		}
		else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			LOG_INFO("Validation Layer: {0}", pCallbackData->pMessage);
		}
		else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_CRITICAL("Validation Layer: {0}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	//---------------------------------------------------------------------------------------------------------------------
	static VkResult VKAPI_ATTR CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		//return PFN_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pDebugMessenger);
		
		vk::DynamicLoader dl;
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkCreateDebugUtilsMessengerEXT");
		
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//---------------------------------------------------------------------------------------------------------------------
	static void VKAPI_ATTR DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		vk::DynamicLoader dl;

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
};

