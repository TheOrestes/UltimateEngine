#include "UltimateEnginePCH.h"
#include "DirectXApplication.h"
#include "D3DRenderer.h"
#include "D3DGlobals.h"

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::DirectXApplication()
{
	m_uiAppWidth = 0;
	m_uiAppHeight = 0;

	m_pD3DRenderer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::~DirectXApplication()
{
	DirectXApplication::Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Cleanup()
{
	SAFE_DELETE(m_pD3DRenderer);

	UT::D3D12::CORE::Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
bool DirectXApplication::Initialize(const GLFWwindow* pWindow)
{
	UT_ASSERT_NULL(pWindow, "Windows pointer cannot be NULL!");

	int width, height = 0;
	glfwGetWindowSize(const_cast<GLFWwindow*>(pWindow), &width, &height);

	// Store into globals for future usage!
	UT::GLOBALS::GWindowWidth  = static_cast<uint16_t>(width);
	UT::GLOBALS::GWindowHeight = static_cast<uint16_t>(height);
	UT::GLOBALS::GWindowHandle = glfwGetWin32Window(const_cast<GLFWwindow*>(pWindow));

	UT::D3D12::CORE::Initialize();

	m_pD3DRenderer = new D3DRenderer();

	UT_CHECK_BOOL(m_pD3DRenderer->Initialize());

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Update(double dt)
{
	m_pD3DRenderer->Update(dt);
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Render()
{
	UT_ASSERT_NULL(m_pD3DRenderer);

	UT::D3D12::CORE::BeginFrame();
	m_pD3DRenderer->RecordCommands();
	UT::D3D12::CORE::EndFrame();
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

