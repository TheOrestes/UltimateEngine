
#include "UltimateEnginePCH.h"
#include "DirectXApplication.h"
#include "DXRenderer.h"

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::DirectXApplication()
{
	m_uiAppWidth = 0;
	m_uiAppHeight = 0;

	m_pD3DDebug = nullptr;
	m_pDXGIFactory = nullptr;

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
	DisableDebug();
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

	m_hwnd = glfwGetWin32Window(const_cast<GLFWwindow*>(pWindow));

	EnableDebug();

	// Create factory
	UINT dxgiFactoryFlags = 0;

#if defined _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	UT_CHECK_HRESULT(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_pDXGIFactory)));

	m_pDXRenderer = new DXRenderer();
	UT_CHECK_BOOL(m_pDXRenderer->Initialize(m_hwnd, m_pDXGIFactory));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::EnableDebug()
{
#if _DEBUG
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pDXGIDebug));
	D3D12GetDebugInterface(IID_PPV_ARGS(&m_pD3DDebug));

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
	UT_ASSERT_NULL(m_pDXRenderer, "DXRenderDevice is null!");

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
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::RecreateOnWindowResize(const GLFWwindow* pWindow)
{
	LOG_DEBUG("Recreating on windows resize...");
}

