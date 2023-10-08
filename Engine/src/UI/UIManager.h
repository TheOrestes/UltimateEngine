#pragma once

struct GLFWwindow;
class VulkanDevice;

class UIManager
{
public:
	UIManager();
	~UIManager();

	bool			Initialize(const GLFWwindow* pWindow, vk::Instance vkInstance, vk::RenderPass renderPass, const VulkanDevice* pDevice);

	void			HandleWindowResize();
	void			BeginRender();
	void			EndRender(const VulkanDevice* pDevice, uint32_t imageIndex);
	void			Render();
};

