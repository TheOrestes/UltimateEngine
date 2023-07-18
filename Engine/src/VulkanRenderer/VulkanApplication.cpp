#include "UltimateEnginePCH.h"

#define VOLK_IMPLEMENTATION
#include "../Core/EngineApplication.h"
#include "VulkanApplication.h"
#include "VulkanContext.h"

#define VK_NO_PROTOTYPES 
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::VulkanApplication()
{
	m_pVKContext = nullptr;

#ifdef _DEBUG
	m_bEnableValidation = true;
#else
	m_bEnableValidation = false;
#endif

	m_vkDebugMessenger = VK_NULL_HANDLE;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::~VulkanApplication()
{
	SAFE_DELETE(m_pVKContext);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Cleanup()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Initialize(void* pWindow)
{
	UT_ASSERT_NULL(pWindow, "Window pointer cannot be null!");

	UT_ASSERT_VK(volkInitialize(), "Failed to initialize Volk!");
	LOG_INFO("Volk Initialized...");

	m_pVKContext = new VulkanContext();
	LOG_INFO("Vulkan Context Initialized...");

	// start book-keeping for all variables here!
	m_pVKContext->pWindow = reinterpret_cast<GLFWwindow*>(pWindow);

	CreateInstance();

	volkLoadInstance(m_pVKContext->vkInst);

	// Create surface!
	glfwCreateWindowSurface(m_pVKContext->vkInst, m_pVKContext->pWindow, nullptr, &(m_pVKContext->vkSurface));// , "Vulkan Surface creation failed!");

	SetupDebugMessenger();
	//RunShaderCompiler("Assets/Shaders");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Update(float dt)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Render()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::CreateInstance()
{
	// Create basic application information!
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
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
	CheckInstanceExtensionSupport(vecExtensions);

	// Debug Validation layer!
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

	// Supported! Create Vulkan Instance!
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo = &appInfo;
	instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vecExtensions.size());
	instCreateInfo.ppEnabledExtensionNames = vecExtensions.data();
	instCreateInfo.pNext = nullptr;

	//--- list of validation layers...
	const std::vector<const char*> strValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	if (m_bEnableValidation)
	{
		instCreateInfo.enabledLayerCount = static_cast<uint32_t>(strValidationLayers.size());
		instCreateInfo.ppEnabledLayerNames = strValidationLayers.data();

		PopulateDebugMessengerCreateInfo(createInfo);
		instCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&createInfo;
	}
	else
	{
		instCreateInfo.enabledLayerCount = 0;
		instCreateInfo.ppEnabledLayerNames = nullptr;
		instCreateInfo.pNext = nullptr;
	}

	UT_ASSERT_VK(vkCreateInstance(&instCreateInfo, nullptr, &(m_pVKContext->vkInst)), "Failed to create Vulkan Instance!");

	LOG_INFO("Vulkan Instance created!");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> vecExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vecExtensions.data());

#if defined _DEBUG
	// Enumerate all the extensions supported by the vulkan instance.
	// Ideally, this list should contain extensions requested by GLFW and
	// few additional ones!
	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		LOG_DEBUG(vecExtensions[i].extensionName, " extension available!");
	}
	
#endif

	// Check if given extensions are in the list of available extensions
	for (uint32_t i = 0; i < extensionCount; i++)
	{
		bool hasExtension = false;

		for (uint32_t j = 0; j < instanceExtensions.size(); j++)
		{
			if (!strcmp(vecExtensions[i].extensionName, instanceExtensions[j]))
			{
				hasExtension = true;
				break;
			}
		}

		//UT_ASSERT_BOOL(hasExtension, "VkInstance doesn't support required extension!");
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::SetupDebugMessenger()
{
	if (m_bEnableValidation)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		UT_ASSERT_VK(CreateDebugUtilsMessengerEXT(m_pVKContext->vkInst, &createInfo, nullptr, &m_vkDebugMessenger), "Debug Util Messenger Creation Failed!");
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
				(entry.path().extension().string() == ".vert" || entry.path().extension().string() == ".frag"))
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
