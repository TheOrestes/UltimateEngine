#pragma once

class VulkanContext;

class VulkanRenderer
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

