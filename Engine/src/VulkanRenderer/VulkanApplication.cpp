#include "UltimateEnginePCH.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../Core/EngineApplication.h"
#include "VulkanApplication.h"
#include "VulkanRenderer.h"
#include "VulkanGlobals.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::VulkanApplication()
{

#ifdef _DEBUG
	m_bEnableValidation = true;
#else
	m_bEnableValidation = false;
#endif

	m_vkDebugMessenger = VK_NULL_HANDLE;
	m_pVulkanRenderer = nullptr;
}
//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::~VulkanApplication()
{
	Cleanup();

	SAFE_DELETE(m_pVulkanRenderer);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Cleanup()
{
	m_pVulkanRenderer->Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Initialize(const GLFWwindow* pWindow)
{
	UT_ASSERT_NULL(pWindow, "Window pointer cannot be null!");

	CreateInstance();
	CreateSurface(pWindow);

	SetupDebugMessenger();
	//RunShaderCompiler("Assets/Shaders");

	m_pVulkanRenderer = new VulkanRenderer();
	m_pVulkanRenderer->Initialize(pWindow, m_vkInstance, m_vkSurface);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Update(float dt)
{
	m_pVulkanRenderer->Update(dt);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Render()
{
	m_pVulkanRenderer->Render();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::CleanupOnWindowResize()
{
	m_pVulkanRenderer->CleanupOnWindowsResize();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::RecreateOnWindowResize(const GLFWwindow* pWindow)
{
	m_pVulkanRenderer->RecreateOnWindowsResize(pWindow, m_vkSurface);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::HandleWindowResizedCallback(const GLFWwindow* pWindow)
{
	CleanupOnWindowResize();
	RecreateOnWindowResize(pWindow);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::CreateInstance()
{
	// Create basic application information!
	vk::ApplicationInfo appInfo = {};
	appInfo.sType = vk::StructureType::eApplicationInfo;
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Ultimate Game Engine";
	appInfo.pEngineName = "Ultimate Renderer";
	appInfo.pNext = nullptr;

	// Get list of extensions required by GLFW
	uint32_t instanceExtnCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&instanceExtnCount);
	std::vector<const char*> vecExtensions(glfwExtensions, glfwExtensions + instanceExtnCount);

	if (m_bEnableValidation)
	{
		// Add this to required application extensions!
		vecExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Check if extensions required by GLFW are supported by our Vulkan instance!
	if (UT::VkUtility::CheckInstanceExtensionSupport(vecExtensions))
	{
		// Debug Validation layer!
		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

		// Supported! Create Vulkan Instance!
		vk::InstanceCreateInfo* instCreateInfo = new vk::InstanceCreateInfo();
		instCreateInfo->sType = vk::StructureType::eInstanceCreateInfo;
		instCreateInfo->pApplicationInfo = &appInfo;
		instCreateInfo->enabledExtensionCount = static_cast<uint32_t>(vecExtensions.size());
		instCreateInfo->ppEnabledExtensionNames = vecExtensions.data();
		instCreateInfo->pNext = nullptr;

		//--- list of validation layers...
		const std::vector<const char*> strValidationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		vk::DebugUtilsMessengerCreateInfoEXT createInfo = {};
		if (m_bEnableValidation)
		{
			instCreateInfo->enabledLayerCount = static_cast<uint32_t>(strValidationLayers.size());
			instCreateInfo->ppEnabledLayerNames = strValidationLayers.data();

			PopulateDebugMessengerCreateInfo(createInfo);
			instCreateInfo->pNext = (vk::DebugUtilsMessengerCreateInfoEXT*)&createInfo;
		}
		else
		{
			instCreateInfo->enabledLayerCount = 0;
			instCreateInfo->ppEnabledLayerNames = nullptr;
			instCreateInfo->pNext = nullptr;
		}

		UT_ASSERT_VK(vk::createInstance(instCreateInfo, nullptr, &m_vkInstance), "Vulkan Instance creation failed!");

		LOG_INFO("Vulkan Instance created!");
	}
	else
	{
		LOG_WARNING("GLFW Extensions are not supported by Vulkan this Instance!");
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::CreateSurface(const GLFWwindow* pWindow)
{
	// Create surface!
	VkSurfaceKHR rawSurface;
	vk::Result result = static_cast<vk::Result>(glfwCreateWindowSurface(m_vkInstance, const_cast<GLFWwindow*>(pWindow), nullptr, &(rawSurface)));
	m_vkSurface = vk::createResultValueType(result, rawSurface);

	int width, height = 0;
	glfwGetWindowSize(const_cast<GLFWwindow*>(pWindow), &width, &height);

	// Store windows width & height for future usage!
	m_uiAppWidth = static_cast<uint16_t>(width);
	m_uiAppHeight = static_cast<uint16_t>(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::SetupDebugMessenger()
{
	if (m_bEnableValidation)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_vkDebugMessenger);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::RunShaderCompiler(const std::string& directoryPath)
{
	// First check if shader compiler exists at the path mentioned?
	std::filesystem::path compilerPath("C:/VulkanSDK/1.3.250.1/Bin/glslc.exe");

	if (std::filesystem::exists(compilerPath))
	{
		for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
		{
			if (entry.is_regular_file() &&
			   (entry.path().extension().string() == ".vert" || entry.path().extension().string() == ".frag") || 
			    entry.path().extension().string() == ".rchit" || entry.path().extension().string() == ".rmiss" || entry.path().extension().string() == ".rgen")
			{
				std::string cmd = compilerPath.string() + " --target-env=vulkan1.3" + " -c" + " " + entry.path().string() + " -o " + entry.path().string() + ".spv";
				LOG_INFO("Compiling shader " + entry.path().filename().string());
				std::system(cmd.c_str());
			}
		}
	}
	else
	{
		UT_ASSERT_BOOL(false, "Shader Compiler Not Found!");
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.flags = 0;
	createInfo.messageSeverity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |	
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
}
