#include "UltimateEnginePCH.h"
#include "DXRenderDevice.h"
#include <dxgi1_4.h>

#include "BufferHelpers.h"
#include "D3DGlobals.h"
#include "EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
DXRenderDevice::DXRenderDevice() :
	m_pD3DDevice(nullptr),
	m_pD3DDebugDevice(nullptr),
	m_pSwapchain(nullptr),
	m_pD3DCommandQueue(nullptr),
	m_pD3DDescriptorHeapRTV(nullptr),
	m_pD3DDescriptorHeapDSV(nullptr),
	m_pD3DDepthStencilBuffer(nullptr),
	m_pD3DDescriptorHeapUI(nullptr)
{
	m_pListD3DRenderTargetBuffers.clear();
}

//---------------------------------------------------------------------------------------------------------------------
DXRenderDevice::~DXRenderDevice()
{
	Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
const char* DXRenderDevice::GetAPIName()
{
	return "D3D12";
}

//---------------------------------------------------------------------------------------------------------------------
const char* DXRenderDevice::GetGPUName()
{
	return m_strGPUName.c_str();
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::Initialize(HWND hwnd, const IDXGIFactory6* pFactory)
{
	UT_CHECK_BOOL(CreateDevice(pFactory), "D3D Device creation failed!");
	UT_CHECK_BOOL(CreateCommandQueue(), "D3D Command Queue creation failed!");
	UT_CHECK_BOOL(CreateSwapchain(hwnd, pFactory), "D3D Swapchain creation failed!");
	UT_CHECK_BOOL(CreateDescriptorHeap(), "D3D Descriptor Heap creation failed!");
	UT_CHECK_BOOL(CreateRenderTargetView(), "D3D Render Target View creation failed!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::Cleanup()
{
	// get swapchain out of full screen before exiting
	BOOL bFullscreen = false;
	if(m_pSwapchain->GetFullscreenState(&bFullscreen, nullptr))
	{
		m_pSwapchain->SetFullscreenState(false, nullptr);
	}

	for (auto element : m_pListD3DRenderTargetBuffers)
	{
		SAFE_RELEASE(element);
	}

	m_pListD3DRenderTargetBuffers.clear();

	SAFE_RELEASE(m_pD3DDescriptorHeapUI);
	SAFE_RELEASE(m_pD3DDepthStencilBuffer);
	SAFE_RELEASE(m_pD3DDescriptorHeapDSV);
	SAFE_RELEASE(m_pD3DDescriptorHeapRTV);
	SAFE_RELEASE(m_pD3DCommandQueue);
	SAFE_RELEASE(m_pSwapchain);
	SAFE_RELEASE(m_pD3DDebugDevice);
	SAFE_RELEASE(m_pD3DDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::CleanupOnWindowResize()
{
	for (auto element : m_pListD3DRenderTargetBuffers)
	{
		SAFE_RELEASE(element);
	}

	m_pListD3DRenderTargetBuffers.clear();

	SAFE_RELEASE(m_pD3DDescriptorHeapUI);
	SAFE_RELEASE(m_pD3DDepthStencilBuffer);
	SAFE_RELEASE(m_pD3DDescriptorHeapDSV);
	SAFE_RELEASE(m_pD3DDescriptorHeapRTV);
	SAFE_RELEASE(m_pD3DCommandQueue);
	SAFE_RELEASE(m_pSwapchain);
	SAFE_RELEASE(m_pD3DDebugDevice);
	SAFE_RELEASE(m_pD3DDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight)
{
	m_pSwapchain->ResizeBuffers(UT::Globals::GBackbufferCount, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
	CreateRenderTargetView();
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdListType, ID3D12CommandAllocator** pOutCmdAllocator)
{
	const HRESULT Hr = m_pD3DDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(pOutCmdAllocator));
	return UT_CHECK_HRESULT(Hr, "CreateCommandAllocator", magic_enum::enum_name(cmdListType));
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE cmdListType, ID3D12CommandAllocator* pCmdAllocator, ID3D12GraphicsCommandList** pOutCmdList)
{
	const HRESULT Hr = m_pD3DDevice->CreateCommandList(0, cmdListType, pCmdAllocator, nullptr, IID_PPV_ARGS(pOutCmdList));
	
	return UT_CHECK_HRESULT(Hr, "CreateCommandList", magic_enum::enum_name(cmdListType));
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS fenceFlags, ID3D12Fence** pOutFence)
{
	const HRESULT Hr = m_pD3DDevice->CreateFence(initialValue, fenceFlags, IID_PPV_ARGS(pOutFence));
	return UT_CHECK_HRESULT(Hr, "CreateFence", magic_enum::enum_name(fenceFlags));
}

//---------------------------------------------------------------------------------------------------------------------
//bool DXRenderDevice::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rootSignDesc, ComPtr<ID3D12RootSignature>& pOutRootSignature)
//{
//	ComPtr<ID3DBlob> pError;
//	UT_CHECK_HRESULT(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pError, nullptr), "Root Signature Serialize Failed!");
//
//	UT_CHECK_HRESULT(m_pD3DDevice->CreateRootSignature(0, pError->GetBufferPointer(), pError->GetBufferSize(), IID_PPV_ARGS(&pOutRootSignature)),
//					"Root Signature creation failed!");
//}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::SignalFence(ID3D12Fence* pFence, uint64_t uiFenceValue) const
{
	HRESULT Hr = m_pD3DCommandQueue->Signal(pFence, uiFenceValue);
	UT_ASSERT_HRESULT(Hr, "Signalling fence FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::Present() const 
{
	HRESULT Hr = m_pSwapchain->Present(0, 0);
	UT_ASSERT_HRESULT(Hr, "Swapchain Present FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::ExecuteCommandLists(std::vector<ID3D12CommandList*> vecCommandList)
{
	ID3D12CommandList* listCommandLists[] = { vecCommandList[0]};

	// execute the array of command lists
	m_pD3DCommandQueue->ExecuteCommandLists(static_cast<UINT>(vecCommandList.size()), listCommandLists);
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateDevice(const IDXGIFactory6* pFactory)
{
	// Create Adapter
	IDXGIAdapter1* pD3DAdapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != const_cast<IDXGIFactory6*>(pFactory)->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pD3DAdapter)); ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		pD3DAdapter->GetDesc1(&desc);

		std::wstring w_description(desc.Description);
		std::string description(w_description.begin(), w_description.end());

		LOG_INFO("Device Chosen = {0}", description);

		// check if adapter supports D3D12
		if(SUCCEEDED(D3D12CreateDevice(pD3DAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pD3DDevice))))
		{
			break;
		}
	}

	SAFE_RELEASE(pD3DAdapter);

	LOG_INFO("D3D Device created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateCommandQueue()
{
	UT_CHECK_NULL(m_pD3DDevice, "ID3DDevice pointer");

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	const HRESULT Hr = m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pD3DCommandQueue));
	UT_CHECK_HRESULT(Hr, "CreateCommandQueue", magic_enum::enum_name(queueDesc.Type));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateSwapchain(HWND hwnd, const IDXGIFactory6* pFactory)
{
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = UT::Globals::GWindowWidth;
	swapchainDesc.Height = UT::Globals::GWindowHeight;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc = { 1,0 };
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = UT::Globals::GBackbufferCount;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* pTempSwapchain;
	const HRESULT Hr = const_cast<IDXGIFactory6*>(pFactory)->CreateSwapChainForHwnd(m_pD3DCommandQueue, hwnd, &swapchainDesc, nullptr, nullptr, &pTempSwapchain);
	UT_CHECK_HRESULT(Hr, "CreateSwapChain",magic_enum::enum_name(swapchainDesc.Format));

	if(SUCCEEDED(pTempSwapchain->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&m_pSwapchain)))
	{
		m_pSwapchain = static_cast<IDXGISwapChain4*>(pTempSwapchain);
		LOG_INFO("Swapchain Created...");
	}

	SAFE_RELEASE(pTempSwapchain);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateDescriptorHeap()
{
	// Descriptor heap for RTV...
	D3D12_DESCRIPTOR_HEAP_DESC descRTV = {};
	descRTV.NumDescriptors = UT::Globals::GBackbufferCount;
	descRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT Hr = m_pD3DDevice->CreateDescriptorHeap(&descRTV, IID_PPV_ARGS(&m_pD3DDescriptorHeapRTV));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(descRTV.Type));

	// Descriptor heap for DepthStencilView
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.NumDescriptors = 1;
	depthStencilViewDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depthStencilViewDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Hr = m_pD3DDevice->CreateDescriptorHeap(&depthStencilViewDesc, IID_PPV_ARGS(&m_pD3DDescriptorHeapDSV));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(depthStencilViewDesc.Type));

	// Descriptor heap for Shader Resource view, Unordered Access view & Constant Buffer view...
	D3D12_DESCRIPTOR_HEAP_DESC descSRV = {};
	descSRV.NumDescriptors = 1;
	descSRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descSRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	Hr = m_pD3DDevice->CreateDescriptorHeap(&descSRV, IID_PPV_ARGS(&m_pD3DDescriptorHeapUI));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(descSRV.Type));

	LOG_INFO("Descriptor heaps created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateRenderTargetView()
{
	// Query vendor-specific size of single descriptor
	m_uiDescriptorSizeRTV = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_uiDescriptorSizeDSV = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// Get handle to first descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE descHandle = m_pD3DDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();

	m_pListD3DRenderTargetBuffers.reserve(UT::Globals::GBackbufferCount);

	// DepthStencil buffer setup
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES depthStencilHeapProps = {};
	depthStencilHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	DXGI_SAMPLE_DESC depthStencilSampleDesc = {};
	depthStencilSampleDesc.Count = 1;
	depthStencilSampleDesc.Quality = 0;

	D3D12_RESOURCE_DESC depthStencilResourceDesc = {};
	depthStencilResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilResourceDesc.Width = UT::Globals::GWindowWidth;
	depthStencilResourceDesc.Height = UT::Globals::GWindowHeight;
	depthStencilResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthStencilResourceDesc.Alignment = 0;
	depthStencilResourceDesc.MipLevels = 1;
	depthStencilResourceDesc.DepthOrArraySize = 1;
	depthStencilResourceDesc.SampleDesc = depthStencilSampleDesc;
	depthStencilResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;


	for(uint16_t i = 0 ; i < UT::Globals::GBackbufferCount ; ++i)
	{
		// Render target buffers...
		ID3D12Resource* backBuffer;
		if(SUCCEEDED(m_pSwapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer))))
		{
			m_pD3DDevice->CreateRenderTargetView(backBuffer, nullptr, descHandle);
			m_pListD3DRenderTargetBuffers.emplace_back(backBuffer);

			descHandle.ptr += (1 * m_uiDescriptorSizeRTV);
		}
	}

	LOG_INFO("RenderTarget views created...");

	// Depth stencil buffers...
	UT_ASSERT_HRESULT(m_pD3DDevice->CreateCommittedResource(&depthStencilHeapProps, D3D12_HEAP_FLAG_NONE, &depthStencilResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&m_pD3DDepthStencilBuffer)));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_pD3DDevice->CreateDepthStencilView(m_pD3DDepthStencilBuffer, &depthStencilViewDesc, m_pD3DDescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart());
	LOG_INFO("DepthStencil view created...");

	return true;
}




