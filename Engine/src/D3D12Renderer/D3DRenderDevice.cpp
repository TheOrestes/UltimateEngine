#include "UltimateEnginePCH.h"
#include "D3DRenderDevice.h"
#include <dxgi1_4.h>

#include "D3DGlobals.h"
#include "EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
D3DRenderDevice::D3DRenderDevice()
{
	m_pD3DDevice = nullptr;
	m_pD3DDebugDevice = nullptr;
	m_pD3DCommandQueue = nullptr;
	m_pSwapchain = nullptr;
	m_pD3DDescriptorHeap = nullptr;

	m_pArrD3DCommandAllocator.clear();
	m_pArrD3DRenderTargets.clear();

	m_uiCurrentFrameIndex = 0;
}

//---------------------------------------------------------------------------------------------------------------------
D3DRenderDevice::~D3DRenderDevice()
{
	Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::Initialize(HWND hwnd, IDXGIFactory6* pFactory)
{
	CHECK(CreateDevice(pFactory))
	CHECK(CreateCommandQueue())
	CHECK(CreateCommandAllocator())
	CHECK(CreateSwapchain(hwnd, pFactory))
	CHECK(CreateDescriptorHeap())
	CHECK(CreateRenderTargetView())
	CHECK(CreateCommandList())
}

//---------------------------------------------------------------------------------------------------------------------
void D3DRenderDevice::Cleanup()
{
	m_pArrD3DRenderTargets.clear();
	m_pArrD3DCommandAllocator.clear();

	SAFE_RELEASE(m_pD3DDescriptorHeap)
	SAFE_RELEASE(m_pSwapchain)
	SAFE_RELEASE(m_pD3DCommandQueue)
	SAFE_RELEASE(m_pD3DDebugDevice)
	SAFE_RELEASE(m_pD3DDevice)
}

//---------------------------------------------------------------------------------------------------------------------
void D3DRenderDevice::CleanupOnWindowResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateDevice(IDXGIFactory6* pFactory)
{
	// Create Adapter
	IDXGIAdapter1* pD3DAdapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pD3DAdapter)); ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		pD3DAdapter->GetDesc1(&desc);

		std::wstring w_description(desc.Description);
		std::string description(w_description.begin(), w_description.end());

		LOG_INFO("Device Chosen = {0}", description);

		// check if adapter supports D3D12
		if (SUCCEEDED(D3D12CreateDevice(pD3DAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pD3DDevice))))
		{
			break;
		}

		pD3DAdapter->Release();
	}

#if defined (_DEBUG)
	const HRESULT Hr = m_pD3DDevice->QueryInterface(&m_pD3DDebugDevice);
	UT_CHECK_HRESULT(Hr, "D3D DebugDevice creation failed!");
#endif

	LOG_DEBUG("D3D Device created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateCommandQueue()
{
	UT_CHECK_NULL(m_pD3DDevice, "ID3DDevice pointer");

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	const HRESULT Hr = m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pD3DCommandQueue));
	UT_CHECK_HRESULT(Hr, "Command Queue creation failed!");

	LOG_DEBUG("Command Queue created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateCommandAllocator()
{
	for (uint16_t i = 0; i < UT::D3DGlobals::GBackbufferCount; ++i)
	{
		ID3D12CommandAllocator* pCmdAllocator;
		const HRESULT Hr = m_pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCmdAllocator));
		UT_CHECK_HRESULT(Hr, "Command Allocator creation failed!");

		m_pArrD3DCommandAllocator.emplace_back(pCmdAllocator);
	}

	LOG_DEBUG("Command Allocator created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateSwapchain(HWND hwnd, IDXGIFactory6* pFactory)
{
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = UT::D3DGlobals::GWindowWidth;
	swapchainDesc.Height = UT::D3DGlobals::GWindowHeight;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc = { 1,0 };
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = UT::D3DGlobals::GBackbufferCount;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* pTempSwapchain;
	const HRESULT Hr = pFactory->CreateSwapChainForHwnd(m_pD3DCommandQueue, hwnd, &swapchainDesc, nullptr, nullptr, &pTempSwapchain);
	UT_CHECK_HRESULT(Hr, "Swapchain creation failed!");

	if(SUCCEEDED(pTempSwapchain->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&m_pSwapchain)))
	{
		m_pSwapchain = static_cast<IDXGISwapChain4*>(pTempSwapchain);
		m_uiCurrentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();

		LOG_DEBUG("Swapchain Created...");
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = UT::D3DGlobals::GBackbufferCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	const HRESULT Hr = m_pD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pD3DDescriptorHeap));
	UT_CHECK_HRESULT(Hr, "Descriptor Heap creation failed!");

	LOG_DEBUG("Descriptor heap created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateRenderTargetView()
{
	// Query vendor-specific size of single descriptor
	const uint32_t rtvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Get handle to first descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE descHandle = m_pD3DDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pArrD3DRenderTargets.reserve(UT::D3DGlobals::GBackbufferCount);

	for(uint16_t i = 0 ; i < UT::D3DGlobals::GBackbufferCount ; ++i)
	{
		ID3D12Resource* backBuffer;
		if(SUCCEEDED(m_pSwapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer))))
		{
			m_pD3DDevice->CreateRenderTargetView(backBuffer, nullptr, descHandle);
			m_pArrD3DRenderTargets.emplace_back(backBuffer);

			descHandle.ptr += (1 * rtvDescriptorSize);
		}
	}

	LOG_DEBUG("RenderTargetViews created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::CreateCommandList()
{
	// create the command list with the first allocator
	HRESULT Hr = m_pD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pArrD3DCommandAllocator.at(0), nullptr, IID_PPV_ARGS(&m_pD3DGraphicsCommandList));
	UT_CHECK_HRESULT(Hr, "Cannot create command list!");

	// Command lists are created in "Recording" state. We do not want to record the command list yet, so we close it. 
	Hr = m_pD3DGraphicsCommandList->Close();
	UT_CHECK_HRESULT(Hr, "Command List Close() failed!");

	LOG_DEBUG("Graphics command list created...");
	return true;
}



