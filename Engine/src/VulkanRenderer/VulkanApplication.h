#pragma once

#include "../Core/Core.h"
#include "../Core/EngineApplication.h"
#include "GLFW/glfw3.h"

class UT_API VulkanApplication : public EngineApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

	virtual void Cleanup() override;

	virtual void Initialize();
	virtual void Update(float dt);
	virtual void Render();

private:
	VulkanApplication(const VulkanApplication&);
	VulkanApplication& operator=(const VulkanApplication&);
};

