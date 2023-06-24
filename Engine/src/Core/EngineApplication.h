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
	virtual void Destroy();

private:
	GLFWwindow* m_pGLFWWindow;
};

