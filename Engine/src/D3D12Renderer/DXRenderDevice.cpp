#include "UltimateEnginePCH.h"
#include "DXRenderDevice.h"
#include <dxgi1_4.h>

#include "BufferHelpers.h"
#include "D3DGlobals.h"
#include "EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
DXRenderDevice::DXRenderDevice() :
	m_pD3DDebugDevice(nullptr),
	m_pSwapchain(nullptr),
	m_pD3DDescriptorHeapRTV(nullptr),
	m_pD3DDescriptorHeapDSV(nullptr),
	m_pD3DDepthStencilBuffer(nullptr),
	m_pD3DDescriptorHeapUI(nullptr),
	m_pD3DDescriptorHeapGlobal(nullptr)
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
bool DXRenderDevice::Initialize(HWND hwnd)
{
	UT_CHECK_BOOL(CreateSwapchain(hwnd), "D3D Swapchain creation failed!");
	UT_CHECK_BOOL(CreateDescriptorHeaps(), "D3D Descriptor Heap creation failed!");
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

	SAFE_RELEASE(m_pD3DDescriptorHeapGlobal);
	SAFE_RELEASE(m_pD3DDescriptorHeapUI);
	SAFE_RELEASE(m_pD3DDepthStencilBuffer);
	SAFE_RELEASE(m_pD3DDescriptorHeapDSV);
	SAFE_RELEASE(m_pD3DDescriptorHeapRTV);
	SAFE_RELEASE(m_pSwapchain);
	SAFE_RELEASE(m_pD3DDebugDevice);
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
	SAFE_RELEASE(m_pSwapchain);
	SAFE_RELEASE(m_pD3DDebugDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight)
{
	m_pSwapchain->ResizeBuffers(UT::Globals::GBackbufferCount, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
	CreateRenderTargetView();
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdListType,
                                            ID3D12CommandAllocator** pOutCmdAllocator)
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	const HRESULT Hr = pDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(pOutCmdAllocator));

	UT_ASSERT_HRESULT(Hr, "CreateCommandAllocator", magic_enum::enum_name(cmdListType));
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE cmdListType,
                                               ID3D12CommandAllocator* pCmdAllocator,
                                               ID3D12GraphicsCommandList** pOutCmdList)
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	const HRESULT Hr = pDevice->CreateCommandList(0, cmdListType, pCmdAllocator, nullptr, IID_PPV_ARGS(pOutCmdList));
	
	UT_ASSERT_HRESULT(Hr, "CreateCommandList", magic_enum::enum_name(cmdListType));
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS fenceFlags, ID3D12Fence** pOutFence)
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	const HRESULT Hr = pDevice->CreateFence(initialValue, fenceFlags, IID_PPV_ARGS(pOutFence));

	UT_ASSERT_HRESULT(Hr, "CreateFence", magic_enum::enum_name(fenceFlags));
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
	ID3D12CommandQueue* const pCmdQueue = UT::D3D12::CORE::GetCommandQueue();
	HRESULT Hr = pCmdQueue->Signal(pFence, uiFenceValue);

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
	ID3D12CommandQueue* const pCmdQueue = UT::D3D12::CORE::GetCommandQueue();

	ID3D12CommandList* listCommandLists[] = { vecCommandList[0]};

	// execute the array of command lists
	pCmdQueue->ExecuteCommandLists(static_cast<UINT>(vecCommandList.size()), listCommandLists);
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateSwapchain(HWND hwnd)
{
	IDXGIFactory6* const pFactory = UT::D3D12::CORE::GetFactory();
	ID3D12CommandQueue* const pCmdQueue = UT::D3D12::CORE::GetCommandQueue();

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
	HRESULT Hr = pFactory->CreateSwapChainForHwnd(pCmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, &pTempSwapchain);
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
bool DXRenderDevice::CreateDescriptorHeaps()
{
	// Descriptor heap for RTV...
	D3D12_DESCRIPTOR_HEAP_DESC descRTV = {};
	descRTV.NumDescriptors = UT::Globals::GBackbufferCount;
	descRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

	HRESULT Hr = pDevice->CreateDescriptorHeap(&descRTV, IID_PPV_ARGS(&m_pD3DDescriptorHeapRTV));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(descRTV.Type));
	UT_NAME_D3D_OBJECT(m_pD3DDescriptorHeapRTV, "RTV Heap");

	// Descriptor heap for DepthStencilView
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.NumDescriptors = 1;
	depthStencilViewDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depthStencilViewDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Hr = pDevice->CreateDescriptorHeap(&depthStencilViewDesc, IID_PPV_ARGS(&m_pD3DDescriptorHeapDSV));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(depthStencilViewDesc.Type));
	UT_NAME_D3D_OBJECT(m_pD3DDescriptorHeapDSV, "DSV Heap");

	// Descriptor heap for Shader Resource view, Unordered Access view & Constant Buffer view...
	D3D12_DESCRIPTOR_HEAP_DESC descSRV = {};
	descSRV.NumDescriptors = 1;
	descSRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descSRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	Hr = pDevice->CreateDescriptorHeap(&descSRV, IID_PPV_ARGS(&m_pD3DDescriptorHeapUI));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(descSRV.Type));
	UT_NAME_D3D_OBJECT(m_pD3DDescriptorHeapUI, "UI Heap");

	// Global Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC descGlobal = {};
	descGlobal.NumDescriptors = 2;
	descGlobal.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descGlobal.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Hr = pDevice->CreateDescriptorHeap(&descGlobal, IID_PPV_ARGS(&m_pD3DDescriptorHeapGlobal));
	UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(descGlobal.Type));
	UT_NAME_D3D_OBJECT(m_pD3DDescriptorHeapGlobal, "Global Heap");

	LOG_INFO("Descriptor heaps created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateRenderTargetView()
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

	// Query vendor-specific size of single descriptor
	m_uiDescriptorSizeRTV = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_uiDescriptorSizeDSV = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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
			pDevice->CreateRenderTargetView(backBuffer, nullptr, descHandle);
			m_pListD3DRenderTargetBuffers.emplace_back(backBuffer);

			descHandle.ptr += (1 * m_uiDescriptorSizeRTV);

			UT_NAME_D3D_OBJECT_INDEXED(m_pListD3DRenderTargetBuffers[i], i, "RT Buffer");
		}
	}

	LOG_INFO("RenderTarget views created...");

	// Depth stencil buffers...
	UT_ASSERT_HRESULT(pDevice->CreateCommittedResource(&depthStencilHeapProps, D3D12_HEAP_FLAG_NONE, &depthStencilResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&m_pD3DDepthStencilBuffer)));
	UT_NAME_D3D_OBJECT(m_pD3DDepthStencilBuffer, "Depth-Stencil Buffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	pDevice->CreateDepthStencilView(m_pD3DDepthStencilBuffer, &depthStencilViewDesc, m_pD3DDescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart());
	LOG_INFO("DepthStencil view created...");

	return true;
}




