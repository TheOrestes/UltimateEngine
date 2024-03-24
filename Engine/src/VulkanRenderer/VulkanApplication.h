#pragma once

#include "../Core/Core.h"
#include "../EngineHeader.h"

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_to_string.hpp"

class EngineApplication;
class VulkanRenderer;
enum class CameraAction;

class UT_API VulkanApplication : public EngineApplication
{
public:
	VulkanApplication();
	virtual ~VulkanApplication() override;

	virtual void				Cleanup() override;

	virtual bool				Initialize(const GLFWwindow* pWindow);
	virtual void				Update(double dt);
	virtual void				Render();

	void						HandleSceneInput(const GLFWwindow* pWindow, CameraAction direction, float mousePosX = 0.0f, float mousePosY = 0.0f, bool isMouseClicked = false) const;
	void						HandleWindowResizedCallback(const GLFWwindow* pWindow);

private:
	VulkanApplication(const VulkanApplication&);
	VulkanApplication& operator=(const VulkanApplication&);

	bool						CreateInstance();
	bool						CreateSurface(const GLFWwindow* pWindow);
	void						CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions);
	bool						SetupDebugMessenger();
	bool						RunShaderCompiler(const std::string& directoryPath);
	void						PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void						CleanupOnWindowResize();
	void						RecreateOnWindowResize(const GLFWwindow* pWindow);

private:
	vk::Instance				m_vkInstance;
	vk::SurfaceKHR				m_vkSurface;

	VulkanRenderer*				m_pVulkanRenderer;
	VkDebugUtilsMessengerEXT	m_vkDebugMessenger;

	bool						m_bEnableValidation;
	uint16_t					m_uiAppWidth;
	uint16_t					m_uiAppHeight;

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
};

