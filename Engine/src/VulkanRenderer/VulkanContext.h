#pragma once

#include "../Core/Core.h"
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
class VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

public:
	GLFWwindow*		pWindow;
};
