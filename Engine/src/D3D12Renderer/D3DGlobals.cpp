#include "UltimateEnginePCH.h"
#include "D3DGlobals.h"

namespace UT
{
	namespace D3D12
	{
		//-------------------------------------------------------------------------------------------------------------------
		namespace CORE
		{
			IDXGIFactory6*		g_pFactory			= nullptr;
			ID3D12Device*		g_pDevice			= nullptr;
			ID3D12Debug1*		g_pD3D12Debug		= nullptr;
			ID3D12CommandQueue* g_pD3DCommandQueue	= nullptr;
			IDXGISwapChain4*	g_pD3DSwapChain		= nullptr;

			std::array<ID3D12CommandAllocator*, GLOBALS::GFramesInFlight>						m_ListCommandAllocators;
			std::array<ID3D12GraphicsCommandList*, GLOBALS::GFramesInFlight>					m_ListCommandListsGraphics;
			std::array<ID3D12Fence*, GLOBALS::GFramesInFlight>									m_ListFences;
			std::array<HANDLE, GLOBALS::GFramesInFlight>										m_ListFenceEvents;
			std::array<UINT64, GLOBALS::GFramesInFlight>										m_ListFenceValues;

			constexpr IDXGIFactory6*				const GetFactory()							{ return g_pFactory; }
			constexpr ID3D12Device*					const GetDevice()							{ return g_pDevice; }
			constexpr ID3D12CommandQueue*			const GetCommandQueue()						{ return g_pD3DCommandQueue; }
			constexpr IDXGISwapChain4*				const GetSwapchain()						{ return g_pD3DSwapChain; }

			constexpr ID3D12CommandAllocator*		const GetCommandAllocator(uint16_t index)	{ return m_ListCommandAllocators[index]; }
			constexpr ID3D12GraphicsCommandList*	const GetCommandList(uint16_t index)		{ return m_ListCommandListsGraphics[index]; }

			//-------------------------------------------------------------------------------------------------------------------
			bool Initialize()
			{
				//--- 1. Create factory
				UINT dxgiFactoryFlags = 0;

#if defined _DEBUG
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

				ID3D12Debug* pDebugController;
				if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
				{
					pDebugController->EnableDebugLayer();
				}

				if (SUCCEEDED(pDebugController->QueryInterface(IID_PPV_ARGS(&g_pD3D12Debug))))
				{
					g_pD3D12Debug->EnableDebugLayer();
					g_pD3D12Debug->SetEnableGPUBasedValidation(true);
				}

				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

				SAFE_RELEASE(pDebugController);
#endif

				HRESULT Hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&g_pFactory));
				UT_ASSERT_HRESULT(Hr, "CreateDXGIFactory2");

				//--- 2. Choose the right Adapter & create D3D Device!
				IDXGIAdapter1* pD3DAdapter;
				for (UINT i = 0; DXGI_ERROR_NOT_FOUND != g_pFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pD3DAdapter)); ++i)
				{
					DXGI_ADAPTER_DESC1 desc;
					pD3DAdapter->GetDesc1(&desc);

					std::wstring w_description(desc.Description);
					std::string description(w_description.begin(), w_description.end());

					LOG_INFO("Device Chosen = {0}", description);

					// check if adapter supports D3D12
					if (SUCCEEDED(D3D12CreateDevice(pD3DAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&g_pDevice))))
					{
						break;
					}
				}

				SAFE_RELEASE(pD3DAdapter);
				UT_NAME_D3D_OBJECT(g_pDevice, "Main D3D Device");

				//--- 3. Create Command Queue
				D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
				cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
				cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

				Hr = g_pDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&g_pD3DCommandQueue));
				UT_ASSERT_HRESULT(Hr, "CreateCommandQueue");
				UT_NAME_D3D_OBJECT(g_pD3DCommandQueue, "Command Queue Direct");

				//--- 4. Create Swapchain
				DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
				swapchainDesc.Width = UT::GLOBALS::GWindowWidth;
				swapchainDesc.Height = UT::GLOBALS::GWindowHeight;
				swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapchainDesc.Stereo = false;
				swapchainDesc.SampleDesc = { 1,0 };
				swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapchainDesc.BufferCount = UT::GLOBALS::GFramesInFlight;
				swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

				IDXGISwapChain1* pTempSwapchain;
				Hr = g_pFactory->CreateSwapChainForHwnd(g_pD3DCommandQueue, UT::GLOBALS::GWindowHandle, &swapchainDesc, nullptr, nullptr, &pTempSwapchain);
				UT_CHECK_HRESULT(Hr, "Swapchain creation failed!");

				if (SUCCEEDED(pTempSwapchain->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&g_pD3DSwapChain)))
				{
					g_pD3DSwapChain = static_cast<IDXGISwapChain4*>(pTempSwapchain);
					UT::GLOBALS::GCurrentFrameId = g_pD3DSwapChain->GetCurrentBackBufferIndex();

					LOG_INFO("D3D Swapchain Created...");
				}

				//-- 5. Create Command Allocator | Command List | Fences
				for (uint16_t i = 0; i < GLOBALS::GFramesInFlight; ++i)
				{
					// Command Allocators...
					UT_ASSERT_HRESULT(g_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_ListCommandAllocators[i])));
					UT_NAME_D3D_OBJECT_INDEXED(m_ListCommandAllocators[i], i, "Command Allocator");

					// Command Lists...
					UT_ASSERT_HRESULT(g_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ListCommandAllocators[i], nullptr, IID_PPV_ARGS(&m_ListCommandListsGraphics[i])));
					UT_ASSERT_HRESULT(m_ListCommandListsGraphics[i]->Close());
					UT_NAME_D3D_OBJECT_INDEXED(m_ListCommandListsGraphics[i], i, "Graphics CommandList");

					// Fences...
					UT_ASSERT_HRESULT(g_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_ListFences[i])));
					UT_NAME_D3D_OBJECT_INDEXED(m_ListFences[i], i, "Fence");

					m_ListFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
					m_ListFenceValues[i] = 0;
				}

				return true;
			}

			//-------------------------------------------------------------------------------------------------------------------
			void BeginFrame()
			{
				const uint16_t currFrameIndex = UT::GLOBALS::GCurrentFrameId;

				// Wait for the previous frame to finish!
				if (m_ListFences[currFrameIndex]->GetCompletedValue() < m_ListFenceValues[currFrameIndex])
				{
					m_ListFences[currFrameIndex]->SetEventOnCompletion(m_ListFenceValues[currFrameIndex], m_ListFenceEvents[currFrameIndex]);
					WaitForSingleObject(m_ListFenceEvents[currFrameIndex], INFINITE);
				}

				// Reset command allocator & command list
				m_ListCommandAllocators[currFrameIndex]->Reset();
				m_ListCommandListsGraphics[currFrameIndex]->Reset(m_ListCommandAllocators[currFrameIndex], nullptr);
			}

			//-------------------------------------------------------------------------------------------------------------------
			void EndFrame()
			{
				IDXGISwapChain4* const pSwapchain = UT::D3D12::CORE::GetSwapchain();
				UT_ASSERT_NULL(pSwapchain);

				// Move to next frame
				UT::GLOBALS::GCurrentFrameId = pSwapchain->GetCurrentBackBufferIndex();
				const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

				m_ListCommandListsGraphics[frameIndex]->Close();

				// create an array of command lists (only one command list here)
				ID3D12CommandList* ppCommandLists[] = { m_ListCommandListsGraphics[frameIndex] };
				g_pD3DCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

				// Signal Fence
				m_ListFenceValues[frameIndex]++;
				g_pD3DCommandQueue->Signal(m_ListFences[frameIndex], m_ListFenceValues[frameIndex]);

				pSwapchain->Present(1, 0);
			}

			//-------------------------------------------------------------------------------------------------------------------
			void Cleanup()
			{
				for (uint16_t i = 0; i < GLOBALS::GFramesInFlight; ++i)
				{
					SAFE_RELEASE(m_ListFences[i]);
					SAFE_RELEASE(m_ListCommandListsGraphics[i]);
					SAFE_RELEASE(m_ListCommandAllocators[i]);
				}

				SAFE_RELEASE(g_pD3DSwapChain);
				SAFE_RELEASE(g_pD3DCommandQueue);
				SAFE_RELEASE(g_pD3D12Debug);
				SAFE_RELEASE(g_pDevice);
				SAFE_RELEASE(g_pFactory);
			}
		}

		//-------------------------------------------------------------------------------------------------------------------
		// namespace HelperFunc
		// {
		// 	void CreateVertexShader(const std::string& vsFile, ID3DBlob** vertexShaderBlob)
		// 	{
		// 		//UT_ASSERT_NULL(vertexShaderBlob);
		// 
		// 		ID3DBlob* errorBlob;
		// 
		// 		const std::wstring sTemp = std::wstring(vsFile.begin(), vsFile.end());
		// 		const LPCWSTR ws = sTemp.c_str();
		// 
		// 		HRESULT Hr = D3DCompileFromFile(ws, nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vertexShaderBlob, &errorBlob);
		// 
		// 		if (FAILED(Hr) && errorBlob != nullptr)
		// 		{
		// 			const char* ErrorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
		// 			UT_ASSERT_HRESULT(Hr, "CreateVertexShader", ErrorMsg);
		// 		}
		// 
		// 		// Extract the shader name getting compiled...
		// 		std::size_t pos = vsFile.find_last_of("/");
		// 		std::string shaderName = vsFile.substr(++pos);
		// 		LOG_DEBUG("{0} => Vertex Shader Compiled", shaderName);
		// 	}
		// 
		// 	//-------------------------------------------------------------------------------------------------------------------
		// 	void CreateFragmentShader(const std::string& fsFile, ID3DBlob** fragmentShaderBlob)
		// 	{
		// 		//UT_ASSERT_NULL(fragmentShaderBlob);
		// 
		// 		ID3DBlob* errorBlob;
		// 
		// 		const std::wstring sTemp = std::wstring(fsFile.begin(), fsFile.end());
		// 		const LPCWSTR ws = sTemp.c_str();
		// 
		// 		HRESULT Hr = D3DCompileFromFile(ws, nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, fragmentShaderBlob, &errorBlob);
		// 
		// 		if (FAILED(Hr) && errorBlob != nullptr)
		// 		{
		// 			const char* ErrorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
		// 			UT_ASSERT_HRESULT(Hr, "CreateFragmentShader", ErrorMsg);
		// 		}
		// 
		// 		// Extract the shader name getting compiled...
		// 		std::size_t pos = fsFile.find_last_of("/");
		// 		std::string shaderName = fsFile.substr(++pos);
		// 		LOG_DEBUG("{0} => Fragment Shader Compiled", shaderName);
		// 	}
		// 
		// 	//-------------------------------------------------------------------------------------------------------------------
		// 	void CreateVertexInputLayoutDesc(D3D12_INPUT_LAYOUT_DESC& outLayoutDesc)
		// 	{
		// 		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		// 		{
		// 			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		// 		};
		// 
		// 		// fill out an input layout description structure
		// 		outLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
		// 		outLayoutDesc.pInputElementDescs = inputLayout;
		// 	}
		// }
	}

}
