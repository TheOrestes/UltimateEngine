#pragma once

#include "Core.h"

struct GLFWwindow;
class VulkanApplication;

class UT_API EngineApplication
{
public:
	EngineApplication();
	virtual ~EngineApplication();

	void Initialize(const std::string& name);
	void Run();
	virtual void Cleanup();

	//-- EVENTS
	static void				WindowClosedCallback(GLFWwindow* pWindow);
	static void				WindowResizedCallback(GLFWwindow* pWindow, int width, int height);
	static void				KeyHandlerCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods);
	static void				MousePositionCallback(GLFWwindow* pWindow, double xPos, double yPos);
	static void				MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods);
	static void				MouseScrollCallback(GLFWwindow* pWindow, double xOffset, double yOffset);

private:
	GLFWwindow*				m_pGLFWWindow;
	VulkanApplication*		m_pVulkanApp;
};

