#include "UltimateEnginePCH.h"
#include "../VulkanRenderer/VulkanDevice.h"
#include "UIManager.h"

#include "vulkan/vulkan.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan_hpp.h"

#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
UIManager::UIManager()
{

}

//---------------------------------------------------------------------------------------------------------------------
UIManager::~UIManager()
{
}

//---------------------------------------------------------------------------------------------------------------------
bool UIManager::Initialize(const GLFWwindow* pWindow, vk::Instance vkInstance, vk::RenderPass renderPass, const VulkanDevice* pDevice)
{
	// create descriptor pool for imgui
	// the size of the pool is oversized, but it's copied from the demo!
	vk::DescriptorPoolSize poolSizes[] =
	{
		{vk::DescriptorType::eSampler, 1000},
		{vk::DescriptorType::eCombinedImageSampler, 1000},
		{vk::DescriptorType::eSampledImage, 1000},
		{vk::DescriptorType::eStorageImage, 1000},
		{vk::DescriptorType::eUniformTexelBuffer, 1000},
		{vk::DescriptorType::eStorageTexelBuffer, 1000},
		{vk::DescriptorType::eUniformBuffer, 1000},
		{vk::DescriptorType::eStorageBuffer, 1000},
		{vk::DescriptorType::eUniformBufferDynamic, 1000},
		{vk::DescriptorType::eStorageBufferDynamic, 1000},
		{vk::DescriptorType::eInputAttachment, 1000}
	};

	vk::DescriptorPoolCreateInfo poolInfo = {};
	poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	vk::Device vkDevice = pDevice->GetDevice();
	vk::PhysicalDevice vkPhysicalDevice = pDevice->GetPhysicalDevice();

	VkDescriptorPool imguiPool = vkDevice.createDescriptorPool(poolInfo);

	// Initialize imgui library
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.Fonts->AddFontFromFileTTF("D:/Development/UltimateEngine/Game/Assets/Fonts/SFMONO.ttf", 14.0f);

	// setup ImGui style
	ImGui::StyleColorsDark();
	
	ImGui_ImplGlfw_InitForVulkan(const_cast<GLFWwindow*>(pWindow), true);

	// this initializes imgui for vulkan
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = vkInstance;
	initInfo.PhysicalDevice = vkPhysicalDevice;
	initInfo.Device = vkDevice;
	initInfo.Queue = pDevice->GetGraphicsQueue();
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = pDevice->GetSwapchainImageCount();
	initInfo.ImageCount = pDevice->GetSwapchainImageCount();
	initInfo.MSAASamples = vk::SampleCountFlagBits::e1;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo, renderPass);

	// execute GPU commands to upload imgui fonts to textures
	VkCommandBuffer cmdBuffer = pDevice->BeginTransferCommandBuffer();
	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
	pDevice->EndAndSubmitTransferCommandBuffer(cmdBuffer);

	// clear fonts from the cpu memory!
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::HandleWindowResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::BeginRender()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::EndRender(const VulkanDevice* pDevice, uint32_t imageIndex)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),  pDevice->GetGraphicsCommandBuffer(imageIndex));
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::Render()
{
	//ImGui::ShowDemoWindow();

	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | 
									ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

	bool open_flag = false;
	ImGui::Begin("DockSpace Demo", &open_flag, window_flags);
 
	ImGui::ShowAboutWindow(&open_flag);
 
	ImGui::End();
}
