#include "UltimateEnginePCH.h"
#include "EngineApplication.h"

#include "..\EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
EngineApplication::EngineApplication()
{
	m_pGLFWWindow = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
EngineApplication::~EngineApplication()
{
	//SAFE_DELETE(m_pGLFWWindow);
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_pGLFWWindow = glfwCreateWindow(gWindowWidht, gWindowHeight, "The Ultimate Engine", nullptr, nullptr);
	UT_ASSERT(m_pGLFWWindow, "Creating Window!");
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Run()
{
	while (!glfwWindowShouldClose(m_pGLFWWindow))
	{
		glfwPollEvents();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void EngineApplication::Destroy()
{
	glfwDestroyWindow(m_pGLFWWindow);
	glfwTerminate();
}
