
#include "UltimateEnginePCH.h"
#include "DirectXApplication.h"
#include "DXRenderer.h"

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::DirectXApplication()
{
	m_uiAppWidth = 0;
	m_uiAppHeight = 0;

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

	int width, height = 0;
	glfwGetWindowSize(const_cast<GLFWwindow*>(pWindow), &width, &height);

	// Store windows width & height for future usage!
	m_uiAppWidth = static_cast<uint16_t>(width);
	m_uiAppHeight = static_cast<uint16_t>(height);

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
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Render()
{
	UT_ASSERT_NULL(m_pDXRenderer, "DXRenderDevice NULL!");

	m_pDXRenderer->Render();
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

