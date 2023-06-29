#include "UltimateEnginePCH.h"
#include "VulkanApplication.h"
#include "VulkanContext.h"
#include "../EngineHeader.h"

VulkanApplication::VulkanApplication()
{
	m_pVKContext = nullptr;
}

VulkanApplication::~VulkanApplication()
{
	SAFE_DELETE(m_pVKContext);
}

void VulkanApplication::Cleanup()
{
}

void VulkanApplication::Initialize(void* pWindow)
{
	UT_ASSERT(pWindow, "Window pointer cannot be null!");

	m_pVKContext = new VulkanContext();
	LOG_INFO("Vulkan Context Initialized...");

	// start book-keeping for all variables here!
	m_pVKContext->pWindow = reinterpret_cast<GLFWwindow*>(pWindow);
}

void VulkanApplication::Update(float dt)
{
}

void VulkanApplication::Render()
{
}
