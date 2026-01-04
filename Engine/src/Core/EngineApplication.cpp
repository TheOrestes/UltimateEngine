#include "UltimateEnginePCH.h"
#include "EngineApplication.h"
#include "../D3D12Renderer/DirectXApplication.h"
#include "../EngineHeader.h"
#include "D3D12Renderer/D3DGlobals.h"

//---------------------------------------------------------------------------------------------------------------------
EngineApplication::EngineApplication()
{
	m_pGLFWWindow = nullptr;
	m_pD3DApp = nullptr;

	m_bAppInitialized = false;
}

//---------------------------------------------------------------------------------------------------------------------
EngineApplication::~EngineApplication()
{
	SAFE_DELETE(m_pD3DApp);

	glfwDestroyWindow(m_pGLFWWindow);
	glfwTerminate();
}

//---------------------------------------------------------------------------------------------------------------------
bool EngineApplication::Initialize(const std::string& name, uint16_t width, uint16_t height)
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

	m_pD3DApp = new DirectXApplication();
	m_bAppInitialized = m_pD3DApp->Initialize(m_pGLFWWindow);

	glfwSetWindowUserPointer(m_pGLFWWindow, m_pD3DApp);

	return m_bAppInitialized;
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Run() const
{
	while (!glfwWindowShouldClose(m_pGLFWWindow))
	{
		glfwPollEvents();

		static double lastTime = 0.0f;
		const double now = glfwGetTime();
		const double dt = now - lastTime;
		lastTime = now;

		m_pD3DApp->Update(dt);
		m_pD3DApp->Render();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Cleanup()
{
	m_pD3DApp->Cleanup();
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
	DirectXApplication* pApp = static_cast<DirectXApplication*>(glfwGetWindowUserPointer(pWindow));
	pApp->HandleWindowResizeCallback(pWindow);

	LOG_DEBUG("Window Resized to [{0}, {1}]", width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::KeyHandlerCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
	DirectXApplication* pApp = static_cast<DirectXApplication*>(glfwGetWindowUserPointer(pWindow));

	if ((action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		LOG_INFO("{0} Key pressed...", key);
	
		switch (key)
		{
			case GLFW_KEY_W:
			{
				pApp->OnKeyPressed(UT::GLOBALS::InputAction::FORWARD);
				break;
			}
			
			case GLFW_KEY_S:
			{
				pApp->OnKeyPressed(UT::GLOBALS::InputAction::BACK);
				break;
			}
			
			case GLFW_KEY_A:
			{
				pApp->OnKeyPressed(UT::GLOBALS::InputAction::LEFT);
				break;
			}
			
			case GLFW_KEY_D:
			{
				pApp->OnKeyPressed(UT::GLOBALS::InputAction::RIGHT);
				break;
			}
			
			case GLFW_KEY_ESCAPE:
			{
				glfwSetWindowShouldClose(pWindow, true);
				break;
			}

			case GLFW_KEY_Q: 
			{
				pApp->OnKeyPressed(UT::GLOBALS::InputAction::UP);
				break;
			}

			case GLFW_KEY_E:  
			{
				pApp->OnKeyPressed(UT::GLOBALS::InputAction::DOWN);
				break;
			}
		}
	}
	
	// Stop if key is released...
	if (action == GLFW_RELEASE)
	{
		switch (key)
		{
			case GLFW_KEY_W:
			{
				pApp->OnKeyReleased(UT::GLOBALS::InputAction::FORWARD);
				break;
			}

			case GLFW_KEY_S:
			{
				pApp->OnKeyReleased(UT::GLOBALS::InputAction::BACK);
				break;
			}

			case GLFW_KEY_A:
			{
				pApp->OnKeyReleased(UT::GLOBALS::InputAction::LEFT);
				break;
			}

			case GLFW_KEY_D:
			{
				pApp->OnKeyReleased(UT::GLOBALS::InputAction::RIGHT);
				break;
			}

			case GLFW_KEY_Q:  
			{
				pApp->OnKeyReleased(UT::GLOBALS::InputAction::UP);
				break;
			}

			case GLFW_KEY_E:  
			{
				pApp->OnKeyReleased(UT::GLOBALS::InputAction::DOWN);
				break;
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::MousePositionCallback(GLFWwindow* pWindow, double xPos, double yPos)
{
	DirectXApplication* pApp = static_cast<DirectXApplication*>(glfwGetWindowUserPointer(pWindow));

	// Rotate only when RIGHT CLICK is down!
	if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		pApp->OnMouseMove(xPos, yPos, true);
		//pApp->HandleSceneInput(pWindow, UT::GLOBALS::InputAction::MOUSE_MOVE, static_cast<float>(xPos), static_cast<float>(yPos), true);
	}
	else
	{
		pApp->OnMouseMove(xPos, yPos, false);
		//pApp->HandleSceneInput(pWindow, UT::GLOBALS::InputAction::MOUSE_MOVE, static_cast<float>(xPos), static_cast<float>(yPos), false);
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
