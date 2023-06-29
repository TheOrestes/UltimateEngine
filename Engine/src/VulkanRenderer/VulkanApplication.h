#pragma once

#include "../Core/Core.h"
#include "../Core/EngineApplication.h"

class VulkanContext;

class UT_API VulkanApplication : public EngineApplication
{
public:
	VulkanApplication();
	virtual ~VulkanApplication();

	virtual void Cleanup() override;

	virtual void Initialize(void* pWindow);
	virtual void Update(float dt);
	virtual void Render();

private:
	VulkanApplication(const VulkanApplication&);
	VulkanApplication& operator=(const VulkanApplication&);

private:
	VulkanContext*	m_pVKContext;
};

