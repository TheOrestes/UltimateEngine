#include "UltimateEnginePCH.h"
#include "DirectXApplication.h"
#include "D3DGlobals.h"

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::DirectXApplication()
{
	m_uiAppWidth = 0;
	m_uiAppHeight = 0;
}

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::~DirectXApplication()
{
	DirectXApplication::Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Cleanup()
{
	UT::D3D12::CORE::Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
bool DirectXApplication::Initialize(const GLFWwindow* pWindow)
{
	UT_ASSERT_NULL(pWindow, "Windows pointer cannot be NULL!");

	int width, height = 0;
	glfwGetWindowSize(const_cast<GLFWwindow*>(pWindow), &width, &height);

	// Store windows width & height for future usage!
	m_uiAppWidth = static_cast<uint16_t>(width);
	m_uiAppHeight = static_cast<uint16_t>(height);

	UT::D3D12::CORE::Initialize();

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Update(double dt)
{
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Render()
{
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::HandleSceneInput(const GLFWwindow* pWindow, CameraAction direction, float mousePosX, float mousePosY, bool isMouseClicked) const
{
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::HandleWindowResizeCallback(const GLFWwindow* pWindow)
{
	CleanupOnWindowResize();
	RecreateOnWindowResize(pWindow);
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::CleanupOnWindowResize()
{
	LOG_DEBUG("Cleaning up on windows resize...");
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::RecreateOnWindowResize(const GLFWwindow* pWindow)
{
	LOG_DEBUG("Recreating on windows resize...");
}

