
#include "UltimateEnginePCH.h"
#include "DirectXApplication.h"
#include "DXRenderer.h"

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::DirectXApplication()
{
	m_pDXGIDebug = nullptr;
	m_pD3DDebug = nullptr;

	m_pDXRenderer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::~DirectXApplication()
{
	DirectXApplication::Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Cleanup()
{
	SAFE_DELETE(m_pDXRenderer);
	SAFE_RELEASE(m_pD3DDebug);

	UT::D3D12::CORE::Cleanup();

	DisableDebug();

	SAFE_RELEASE(m_pDXGIDebug);
}

//---------------------------------------------------------------------------------------------------------------------
bool DirectXApplication::Initialize(const GLFWwindow* pWindow)
{
	UT_CHECK_NULL(pWindow, "GLFW Windows pointer");

	EnableDebug();

	UT::D3D12::CORE::Initialize();

	m_pDXRenderer = new DXRenderer();
	UT_CHECK_BOOL(m_pDXRenderer->Initialize(pWindow));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::EnableDebug()
{
#if _DEBUG
	HRESULT Hr = 0;
	Hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pDXGIDebug));
	UT_ASSERT_HRESULT(Hr, "DXGIGetDebugInterface1");

	Hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_pD3DDebug));
	UT_ASSERT_HRESULT(Hr, "D3D12GetDebugInterface");

	m_pD3DDebug->EnableDebugLayer();
	m_pD3DDebug->SetEnableGPUBasedValidation(true);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::DisableDebug()
{
#if _DEBUG
	m_pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Update(double dt)
{
	// Save it for the use within Renderer!
	UT::Globals::GDeltaTime = dt;

	m_pDXRenderer->Update(dt);
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Render()
{
	UT_ASSERT_NULL(m_pDXRenderer, "DXRenderDevice NULL!");

	m_pDXRenderer->Render();
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::HandleSceneInput(const GLFWwindow* pWindow, UT::Globals::InputAction action, float mousePosX, float mousePosY, bool isMouseClicked) const
{
	m_pDXRenderer->HandleInput(pWindow, action, mousePosX, mousePosY, isMouseClicked);
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
	m_pDXRenderer->CleanupOnWindowResize();
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::RecreateOnWindowResize(const GLFWwindow* pWindow)
{
	LOG_DEBUG("Recreating on windows resize...");

	int width, height;
	glfwGetWindowSize(const_cast<GLFWwindow*>(pWindow), &width, &height);

	m_pDXRenderer->RecreateOnWindowResize(width, height);
}

