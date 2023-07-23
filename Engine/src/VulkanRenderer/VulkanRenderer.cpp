#include "UltimateEnginePCH.h"
#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "../EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_pVulkanDevice = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pVulkanDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Initialize(VulkanContext* pContext)
{
	m_pVulkanDevice = new VulkanDevice(pContext);
	m_pVulkanDevice->SetupDevice(pContext);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(float dt)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Render()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::HandleWindowsResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Cleanup(VulkanContext* pContext)
{
	vkDeviceWaitIdle(pContext->vkDevice);

	m_pVulkanDevice->Cleanup(pContext);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CleanupOnWindowsResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateVulkanDevice()
{
}
