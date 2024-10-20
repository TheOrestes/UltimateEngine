#include "UltimateEnginePCH.h"
#include "DXRenderDevice.h"
#include <dxgi1_4.h>

#include "BufferHelpers.h"
#include "D3DGlobals.h"
#include "EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
DXRenderDevice::DXRenderDevice()
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
bool DXRenderDevice::Initialize(HWND hwnd, ComPtr<IDXGIFactory6>& pFactory)
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

	m_pListD3DRenderTargetBuffers.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::CleanupOnWindowResize()
{
	m_pListD3DRenderTargetBuffers.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight)
{
	m_pSwapchain->ResizeBuffers(UT::Globals::GBackbufferCount, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
	CreateRenderTargetView();
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdListType, ComPtr<ID3D12CommandAllocator>& pOutCmdAllocator)
{
	const HRESULT Hr = m_pD3DDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&pOutCmdAllocator));
	return UT_CHECK_HRESULT(Hr, "Command Allocator creation failed!");
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE cmdListType, const ComPtr<ID3D12CommandAllocator>& pCmdAllocator, ComPtr<ID3D12GraphicsCommandList>& pOutCmdList)
{
	const HRESULT Hr = m_pD3DDevice->CreateCommandList(0, cmdListType, pCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&pOutCmdList));
	return UT_CHECK_HRESULT(Hr, "Cannot create command list!");
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS fenceFlags, ComPtr<ID3D12Fence>& pOutFence)
{
	const HRESULT Hr = m_pD3DDevice->CreateFence(initialValue, fenceFlags, IID_PPV_ARGS(&pOutFence));
	return UT_CHECK_HRESULT(Hr, "Failed to create Fence!");
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
void DXRenderDevice::SignalFence(const ComPtr<ID3D12Fence>& pFence, uint64_t uiFenceValue) const
{
	HRESULT Hr = m_pD3DCommandQueue->Signal(pFence.Get(), uiFenceValue);
	UT_ASSERT_HRESULT(Hr, "Signalling fence FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::Present() const 
{
	HRESULT Hr = m_pSwapchain->Present(0, 0);
	UT_ASSERT_HRESULT(Hr, "Swapchain Present FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderDevice::ExecuteCommandLists(const std::vector<ComPtr<ID3D12CommandList>>& vecCommandList)
{
	ID3D12CommandList* listCommandLists[] = { vecCommandList.data()->Get() };

	// execute the array of command lists
	m_pD3DCommandQueue->ExecuteCommandLists(vecCommandList.size(), listCommandLists);
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateDevice(const ComPtr<IDXGIFactory6>& pFactory)
{
	// Create Adapter
	ComPtr<IDXGIAdapter1> pD3DAdapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pD3DAdapter)); ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		pD3DAdapter->GetDesc1(&desc);

		std::wstring w_description(desc.Description);
		std::string description(w_description.begin(), w_description.end());

		LOG_INFO("Device Chosen = {0}", description);

		// check if adapter supports D3D12
		if (SUCCEEDED(D3D12CreateDevice(pD3DAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pD3DDevice))))
		{
			break;
		}
	}

	LOG_DEBUG("D3D Device created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateCommandQueue()
{
	UT_CHECK_NULL(m_pD3DDevice.Get(), "ID3DDevice pointer");

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	const HRESULT Hr = m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pD3DCommandQueue));
	UT_CHECK_HRESULT(Hr, "Command Queue creation failed!");

	LOG_DEBUG("Command Queue created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateSwapchain(HWND hwnd, const ComPtr<IDXGIFactory6>& pFactory)
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
	const HRESULT Hr = pFactory->CreateSwapChainForHwnd(m_pD3DCommandQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, &pTempSwapchain);
	UT_CHECK_HRESULT(Hr, "Swapchain creation failed!");

	if(SUCCEEDED(pTempSwapchain->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&m_pSwapchain)))
	{
		m_pSwapchain = static_cast<IDXGISwapChain4*>(pTempSwapchain);
		LOG_DEBUG("Swapchain Created...");
	}

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

	HRESULT Hr = m_pD3DDevice->CreateDescriptorHeap(&descRTV, IID_PPV_ARGS(&m_pD3DDescriptorHeapRenderTargetView));
	UT_CHECK_HRESULT(Hr, "Descriptor Heap RTV creation failed!");

	// Descriptor heap for DepthStencilView
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.NumDescriptors = 1;
	depthStencilViewDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depthStencilViewDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Hr = m_pD3DDevice->CreateDescriptorHeap(&depthStencilViewDesc, IID_PPV_ARGS(&m_pD3DDescriptorHeapDepthStencilView));
	UT_CHECK_HRESULT(Hr, "Descriptor Heap creation for Depth Stencil View failed!");

	// Descriptor heap for Shader Resource view, Unordered Access view & Constant Buffer view...
	D3D12_DESCRIPTOR_HEAP_DESC descSRV = {};
	descSRV.NumDescriptors = 1;
	descSRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descSRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	Hr = m_pD3DDevice->CreateDescriptorHeap(&descSRV, IID_PPV_ARGS(&m_pD3DDescriptorHeapShaderResourceView));
	UT_CHECK_HRESULT(Hr, "Descriptor Heap SRV creation failed!");

	LOG_DEBUG("Descriptor heaps created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderDevice::CreateRenderTargetView()
{
	// Query vendor-specific size of single descriptor
	m_uiDescriptorSizeRenderTargetView = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_uiDescriptorSizeDepthStencilView = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// Get handle to first descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE descHandle = m_pD3DDescriptorHeapRenderTargetView->GetCPUDescriptorHandleForHeapStart();

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

			descHandle.ptr += (1 * m_uiDescriptorSizeRenderTargetView);
		}
	}

	LOG_DEBUG("RenderTarget views created...");

	// Depth stencil buffers...
	UT_ASSERT_HRESULT(m_pD3DDevice->CreateCommittedResource(&depthStencilHeapProps, D3D12_HEAP_FLAG_NONE, &depthStencilResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&m_pD3DDepthStencilBuffer)));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_pD3DDevice->CreateDepthStencilView(m_pD3DDepthStencilBuffer.Get(), &depthStencilViewDesc, m_pD3DDescriptorHeapDepthStencilView->GetCPUDescriptorHandleForHeapStart());
	LOG_DEBUG("DepthStencil view created...");

	return true;
}




