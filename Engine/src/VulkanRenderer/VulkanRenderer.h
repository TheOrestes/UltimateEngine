#pragma once

#include "../Core/Core.h"

class VulkanContext;

class UT_API VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool								Initialize(VulkanContext* pContext);
	void								Update(float dt);
	void								Render();
	void								HandleWindowsResize();
	void								Cleanup();
	void								CleanupOnWindowsResize();
};

