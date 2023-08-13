#include "UltimateEnginePCH.h"
#include "EngineApplication.h"
#include "../VulkanRenderer/VulkanApplication.h"
#include "../EngineHeader.h"
#include "GLFW/glfw3.h"

#define VOLK_IMPLEMENTATION

//---------------------------------------------------------------------------------------------------------------------
EngineApplication::EngineApplication()
{
	m_pGLFWWindow = nullptr;
	m_pVulkanApp = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
EngineApplication::~EngineApplication()
{
	SAFE_DELETE(m_pVulkanApp);

	glfwDestroyWindow(m_pGLFWWindow);
	glfwTerminate();
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Initialize(const std::string& name, uint16_t width, uint16_t height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_pGLFWWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	UT_ASSERT_NULL(m_pGLFWWindow, "Creating Window!");

	// Register Events!
	glfwSetWindowCloseCallback(m_pGLFWWindow, WindowClosedCallback);
	glfwSetWindowSizeCallback(m_pGLFWWindow, WindowResizedCallback);
	glfwSetKeyCallback(m_pGLFWWindow, KeyHandlerCallback);
	glfwSetCursorPosCallback(m_pGLFWWindow, MousePositionCallback);
	glfwSetMouseButtonCallback(m_pGLFWWindow, MouseButtonCallback);
	glfwSetScrollCallback(m_pGLFWWindow, MouseScrollCallback);

	m_pVulkanApp = new VulkanApplication();
	m_pVulkanApp->Initialize(reinterpret_cast<void*>(m_pGLFWWindow));

	glfwSetWindowUserPointer(m_pGLFWWindow, m_pVulkanApp);
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Run()
{
	while (!glfwWindowShouldClose(m_pGLFWWindow))
	{
		glfwPollEvents();

		float dt = 0.016f;
		m_pVulkanApp->Update(dt);
		m_pVulkanApp->Render();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Cleanup()
{
	m_pVulkanApp->Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::WindowClosedCallback(GLFWwindow* pWindow)
{
	glfwSetWindowShouldClose(pWindow, true);
	LOG_DEBUG("Window Closed!");
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::WindowResizedCallback(GLFWwindow* pWindow, int width, int height)
{
	VulkanApplication* pApp = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(pWindow));
	pApp->HandleWindowResizedCallback();

	//m_pVulkanApp->HandleWindowResizedCallback(pWindow, width, height);
	LOG_DEBUG("Window Resized to [{0}, {1}]", width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::KeyHandlerCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
	//VulkanApplication* pApp = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(pWindow));

	if ((action == GLFW_REPEAT || GLFW_PRESS))
	{
		switch (key)
		{
			case GLFW_KEY_W:
			{
				//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_FORWARD);
				break;
			}

			case GLFW_KEY_S:
			{
				//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_BACK);
				break;
			}

			case GLFW_KEY_A:
			{
				//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_LEFT);
				break;
			}

			case GLFW_KEY_D:
			{
				//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_RIGHT);
				break;
			}

			case GLFW_KEY_ESCAPE:
			{
				glfwSetWindowShouldClose(pWindow, true);
				break;
			}
		}
	}

	// Stop if key is released...
	if (action == GLFW_RELEASE)
	{
		//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_NONE);
	}

	LOG_INFO("{0} Key pressed...", key);
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::MousePositionCallback(GLFWwindow* pWindow, double xPos, double yPos)
{
	//VulkanApplication* pApp = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(pWindow));

	// Rotate only when RIGHT CLICK is down!
	if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_PAN_2D, static_cast<float>(xPos), static_cast<float>(yPos), true);
	}
	else
	{
		//pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_PAN_2D, static_cast<float>(xPos), static_cast<float>(yPos), false);
	}

	//LOG_INFO("Mouse Position = [{0}, {1}]", xPos, yPos);

}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods)
{
	LOG_INFO("{0} Mouse button pressed...", button);
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::MouseScrollCallback(GLFWwindow* pWindow, double xOffset, double yOffset)
{
	LOG_INFO("Mouse scroll [{0}, {1}]", xOffset, yOffset);
}
