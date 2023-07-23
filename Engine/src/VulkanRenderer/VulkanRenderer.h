#pragma once

#include "../Core/Core.h"

class VulkanContext;
class VulkanDevice;

class UT_API VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	void								Initialize(VulkanContext* pContext);
	void								Update(float dt);
	void								Render();
	void								HandleWindowsResize();
	void								Cleanup(VulkanContext* pContext);
	void								CleanupOnWindowsResize();

private:
	void								CreateVulkanDevice();

private:
	VulkanDevice*						m_pVulkanDevice;
};

