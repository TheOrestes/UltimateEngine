#pragma once

#include "GLFW/glfw3.h"
#include "Core.h"

class UT_API EngineApplication
{
public:
	EngineApplication();
	virtual ~EngineApplication();

	virtual void Initialize();
	virtual void Run();

	//-- EVENTS
	static void				WindowClosedCallback(GLFWwindow* pWindow);
	static void				WindowResizedCallback(GLFWwindow* pWindow, int width, int height);
	static void				KeyHandlerCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods);
	static void				MousePositionCallback(GLFWwindow* pWindow, double xPos, double yPos);
	static void				MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods);
	static void				MouseScrollCallback(GLFWwindow* pWindow, double xOffset, double yOffset);

private:
	GLFWwindow* m_pGLFWWindow;
};

