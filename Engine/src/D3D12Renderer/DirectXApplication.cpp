
#include "UltimateEnginePCH.h"
#include "DirectXApplication.h"
#include "D3DRenderDevice.h"

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::DirectXApplication()
{
	m_uiAppWidth = 0;
	m_uiAppHeight = 0;

	m_pD3D12DebugController = nullptr;
	m_pDXGIFactory = nullptr;

	m_pRenderDeviceD3D = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
DirectXApplication::~DirectXApplication()
{
	DirectXApplication::Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
void DirectXApplication::Cleanup()
{
	m_pRenderDeviceD3D->Cleanup();
	SAFE_DELETE(m_pRenderDeviceD3D);

	SAFE_RELEASE(m_pD3D12DebugController);
	SAFE_RELEASE(m_pDXGIFactory);
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

	// Create factory
	UINT dxgiFactoryFlags = 0;

#if defined _DEBUG
	ID3D12Debug* pDebugController;
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
	{
		pDebugController->EnableDebugLayer();
	}

	if(SUCCEEDED(pDebugController->QueryInterface(IID_PPV_ARGS(&m_pD3D12DebugController))))
	{
		m_pD3D12DebugController->EnableDebugLayer();
		m_pD3D12DebugController->SetEnableGPUBasedValidation(true);
	}

	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

	pDebugController->Release();
	pDebugController = nullptr;
#endif

	if(FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_pDXGIFactory))))
		return false;

	m_pRenderDeviceD3D = new D3DRenderDevice();
	CHECK(m_pRenderDeviceD3D->Initialize(m_pDXGIFactory))

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

