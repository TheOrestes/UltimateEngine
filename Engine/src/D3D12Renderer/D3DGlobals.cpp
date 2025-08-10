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

			constexpr ID3D12Fence*					const GetFence(uint16_t index)				{ return m_ListFences[index]; }
			constexpr HANDLE						const GetFenceEvent(uint16_t index)			{ return m_ListFenceEvents[index]; }
			constexpr UINT64						const GetFenceValue(uint16_t index)			{ return m_ListFenceValues[index]; }

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

				Hr = pTempSwapchain->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&g_pD3DSwapChain);

				if (SUCCEEDED(Hr))
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
				//LOG_INFO("=== BeginFrame[{0}] ====", currFrameIndex);

				// Wait for the previous frame to finish!
				if (m_ListFences[currFrameIndex]->GetCompletedValue() < m_ListFenceValues[currFrameIndex])
				{
					m_ListFences[currFrameIndex]->SetEventOnCompletion(m_ListFenceValues[currFrameIndex], m_ListFenceEvents[currFrameIndex]);
					WaitForSingleObject(m_ListFenceEvents[currFrameIndex], INFINITE);
				}

				// Reset command allocator & command list
				m_ListCommandAllocators[currFrameIndex]->Reset();
				m_ListCommandListsGraphics[currFrameIndex]->Reset(m_ListCommandAllocators[currFrameIndex], nullptr);

				//LOG_DEBUG("CommandList Reset");
			}

			//-------------------------------------------------------------------------------------------------------------------
			void EndFrame()
			{
				IDXGISwapChain4* const pSwapchain = UT::D3D12::CORE::GetSwapchain();
				UT_ASSERT_NULL(pSwapchain);

				// Use the current frame index BEFORE calling Present
				const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

				m_ListCommandListsGraphics[frameIndex]->Close();
				//LOG_DEBUG("CommandList Close");

				// create an array of command lists (only one command list here)
				ID3D12CommandList* ppCommandLists[] = { m_ListCommandListsGraphics[frameIndex] };
				g_pD3DCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				//LOG_DEBUG("CommandList Execute");

				// Signal Fence
				m_ListFenceValues[frameIndex]++;
				g_pD3DCommandQueue->Signal(m_ListFences[frameIndex], m_ListFenceValues[frameIndex]);

				//LOG_DEBUG("FenceValue = {0}", m_ListFenceValues[UT::GLOBALS::GCurrentFrameId]);

				pSwapchain->Present(1, 0);
				//LOG_ERROR("=== EndFrame[{0}] ====", frameIndex);

				// Now advance the global frame index
				UT::GLOBALS::GCurrentFrameId = pSwapchain->GetCurrentBackBufferIndex();
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

			//-------------------------------------------------------------------------------------------------------------------
			void FenceIncrement()
			{
				//LOG_DEBUG("Frame[{0}] | FenceValue = {1}", UT::GLOBALS::GCurrentFrameId, m_ListFenceValues[UT::GLOBALS::GCurrentFrameId]);
				++m_ListFenceValues[UT::GLOBALS::GCurrentFrameId];
				//LOG_DEBUG("Frame[{0}] | FenceValue = {1}", UT::GLOBALS::GCurrentFrameId, m_ListFenceValues[UT::GLOBALS::GCurrentFrameId]);
			}
		}

		namespace HELPER
		{
			//-------------------------------------------------------------------------------------------------------------------
			void CreateGPUBuffer(UINT64 byteSize, ID3D12Resource** outGPUBuffer)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

				// Default buffer (GPU only)
				D3D12_HEAP_PROPERTIES defaultHeapProps = {};
				defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				defaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				defaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

				D3D12_RESOURCE_DESC bufferDesc = {};
				bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				bufferDesc.Alignment = 0;
				bufferDesc.Width = byteSize;
				bufferDesc.Height = 1;
				bufferDesc.DepthOrArraySize = 1;
				bufferDesc.MipLevels = 1;
				bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
				bufferDesc.SampleDesc.Count = 1;
				bufferDesc.SampleDesc.Quality = 0;
				bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				// Create default heap resource
				const HRESULT Hr = pDevice->CreateCommittedResource(&defaultHeapProps,
															D3D12_HEAP_FLAG_NONE,
															&bufferDesc,
															D3D12_RESOURCE_STATE_COPY_DEST,
															nullptr,
															IID_PPV_ARGS(outGPUBuffer));

				UT_ASSERT_HRESULT(Hr, "CreateResource => GPU Buffer");
				//UT_NAME_D3D_OBJECT(outGPUBuffer, "GPU Buffer");
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateUploadBuffer(UINT64 byteSize, ID3D12Resource** outUploadBuffer)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

				D3D12_RESOURCE_DESC desc = {};
				desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				desc.Alignment = 0;
				desc.Width = byteSize;
				desc.Height = 1;
				desc.DepthOrArraySize = 1;
				desc.MipLevels = 1;
				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				desc.Flags = D3D12_RESOURCE_FLAG_NONE;

				const HRESULT Hr = pDevice->CreateCommittedResource(&heapProps,
																	D3D12_HEAP_FLAG_NONE,
																	&desc,
																	D3D12_RESOURCE_STATE_GENERIC_READ,
																	nullptr,
																	IID_PPV_ARGS(outUploadBuffer));

				UT_ASSERT_HRESULT(Hr, "CreateResource => Upload Buffer");
				//UT_NAME_D3D_OBJECT(outUploadBuffer, "Upload Buffer");
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateReadbackBuffer(UINT64 byteSize, ID3D12Resource** outReadbackBuffer)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_READBACK;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

				D3D12_RESOURCE_DESC desc = {};
				desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				desc.Alignment = 0;
				desc.Width = byteSize;
				desc.Height = 1;
				desc.DepthOrArraySize = 1;
				desc.MipLevels = 1;
				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				desc.Flags = D3D12_RESOURCE_FLAG_NONE;

				HRESULT Hr = pDevice->CreateCommittedResource(&heapProps,
															D3D12_HEAP_FLAG_NONE,
															&desc,
															D3D12_RESOURCE_STATE_COPY_DEST,
															nullptr,
															IID_PPV_ARGS(outReadbackBuffer));

				UT_ASSERT_HRESULT(Hr, "CreateResource => Readback Buffer");
				//UT_NAME_D3D_OBJECT(outReadbackBuffer, "Readback Buffer");
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CopyDataFromUploadBufferToGPU(UINT64 byteSize, const void* data, ID3D12Resource* pUploadBuffer, ID3D12Resource* pGpuBuffer)
			{
				const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
				ID3D12CommandAllocator* const pCmdAlloc = UT::D3D12::CORE::GetCommandAllocator(frameIndex);
				ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);
				ID3D12CommandQueue* const pCmdQueue = UT::D3D12::CORE::GetCommandQueue();
				ID3D12Fence* const pFence = UT::D3D12::CORE::GetFence(frameIndex);
				HANDLE const fenceEvent = UT::D3D12::CORE::GetFenceEvent(frameIndex);

				UINT8* mappedData = nullptr;
				constexpr D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on CPU.

				UT_ASSERT_HRESULT(pUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
				memcpy(mappedData, data, byteSize);
				pUploadBuffer->Unmap(0, nullptr);

				// 4) Schedule copy into default heap
				pCmdAlloc->Reset();
				pCommandList->Reset(pCmdAlloc, nullptr);
				
				pCommandList->CopyBufferRegion(pGpuBuffer, 0, pUploadBuffer, 0, byteSize);

				// 5) Resource barrier to transition to GENERIC_READ (for shader access)
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = pGpuBuffer;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

				pCommandList->ResourceBarrier(1, &barrier);

				pCommandList->Close();

				ID3D12CommandList* lists[] = { pCommandList };
				pCmdQueue->ExecuteCommandLists(1, lists);

				// Signal Fence
				UT::D3D12::CORE::FenceIncrement();
				const UINT64 fenceValue = UT::D3D12::CORE::GetFenceValue(frameIndex);

				HRESULT Hr = pCmdQueue->Signal(pFence, fenceValue);
				//LOG_DEBUG("CommandList Reset");
				
				const UINT64 completedFenceValue = pFence->GetCompletedValue();
				if(completedFenceValue < fenceValue)
				{
					Hr = pFence->SetEventOnCompletion(fenceValue, fenceEvent);
					WaitForSingleObject(fenceEvent, INFINITE);
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CompileShader(const std::string& srcFile, const std::string& entryPoint, const std::string& target, const D3D_SHADER_MACRO* pDefines, D3D12_SHADER_BYTECODE& outByteCode)
			{
				ID3DBlob* pCodeBlob;
				ID3DBlob* pErrorBlob;

				UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

				// Convert to the full path & widestring before passing it to the D3D function!
				const std::string shaderPath = UT::GLOBALS::GetExecutableFolderPath() + "Assets/Shaders/" + srcFile;
				const std::wstring wideStr = UT::GLOBALS::ToWString(shaderPath);

#if defined(_DEBUG)
				compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

				HRESULT Hr = D3DCompileFromFile(wideStr.c_str(), pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), target.c_str(), compileFlags, 0, &pCodeBlob, &pErrorBlob);

				if (FAILED(Hr))
				{
					// Retrieve and throw compiler error messages if available
					if (pErrorBlob)
					{
						const std::string msg( static_cast<const char*>(pErrorBlob->GetBufferPointer()), pErrorBlob->GetBufferSize());
						throw std::runtime_error("Shader compilation failed: " + msg);
					}
					else
					{
						throw std::runtime_error("Shader compilation failed with HRESULT 0x" + std::to_string(Hr));
					}
				}

				// Fill the D3D12_SHADER_BYTECODE
				outByteCode.pShaderBytecode = pCodeBlob->GetBufferPointer();
				outByteCode.BytecodeLength = pCodeBlob->GetBufferSize();

				LOG_INFO("{0} => {1} Shader compiled successfully!", srcFile.c_str(), entryPoint.c_str());
			}


			//-------------------------------------------------------------------------------------------------------------------
			void CreateRootSignatue(UINT numRootParams, const D3D12_ROOT_PARAMETER* pRootParams, UINT numStaticSamplers,
				const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers, D3D12_ROOT_SIGNATURE_FLAGS flags,
				ID3D12RootSignature** pOutRootSignature)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

				// 1) Describe the root signature
				D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
				rsDesc.NumParameters = numRootParams;
				rsDesc.pParameters = pRootParams;
				rsDesc.NumStaticSamplers = numStaticSamplers;
				rsDesc.pStaticSamplers = pStaticSamplers;
				rsDesc.Flags = flags;

				// 2) Serialize the root signature
				ID3DBlob* blob;
				ID3DBlob* error;

				HRESULT Hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
				UT_ASSERT_HRESULT(Hr, "SerializeRootSignature");

				Hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(pOutRootSignature));
				UT_ASSERT_HRESULT(Hr, "CreateRootSignature");
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreatePSO(ID3D12RootSignature* pSignature, const D3D12_SHADER_BYTECODE& vsBytecode,
				const D3D12_SHADER_BYTECODE& psBytecode, const D3D12_INPUT_LAYOUT_DESC& inputLayout,
				ID3D12PipelineState** pOutPSO)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

				// Blend state (default)
				D3D12_BLEND_DESC blendDesc = {};
				blendDesc.AlphaToCoverageEnable = FALSE;
				blendDesc.IndependentBlendEnable = FALSE;
				// Single RTV
				blendDesc.RenderTarget[0].BlendEnable = FALSE;
				blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
				blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
				blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

				// Rasterizer state (default)
				D3D12_RASTERIZER_DESC rastDesc = {};
				rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
				rastDesc.CullMode = D3D12_CULL_MODE_BACK;
				rastDesc.FrontCounterClockwise = FALSE;
				rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
				rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
				rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
				rastDesc.DepthClipEnable = TRUE;
				rastDesc.MultisampleEnable = FALSE;
				rastDesc.AntialiasedLineEnable = FALSE;
				rastDesc.ForcedSampleCount = 0;
				rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

				// 3) Depth‐stencil state (default)
				D3D12_DEPTH_STENCIL_DESC dsDesc = {};
				dsDesc.DepthEnable = TRUE;
				dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
				dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
				dsDesc.StencilEnable = FALSE;
				dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
				dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
				// Stencil ops left default (D3D12_STENCIL_OP_KEEP)

				// 4) Fill the PSO descriptor
				D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
				desc.pRootSignature = pSignature;
				desc.VS = vsBytecode;
				desc.PS = psBytecode;
				desc.BlendState = blendDesc;
				desc.RasterizerState = rastDesc;
				desc.DepthStencilState = dsDesc;
				desc.SampleMask = UINT_MAX;
				desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				desc.NumRenderTargets = 1;
				desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
				desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.InputLayout = inputLayout;
				desc.SampleDesc = { 1, 0 };

				UT_ASSERT_HRESULT(pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pOutPSO)), "CreatePSO");
				//UT_NAME_D3D_OBJECT(pOutPSO, "PSO");
			}
		}
	}

	//-------------------------------------------------------------------------------------------------------------------
	std::string GLOBALS::GetExecutableFolderPath()
	{
		WCHAR widePath[MAX_PATH] = {};
		DWORD len = GetModuleFileNameW(nullptr, widePath, MAX_PATH);
		if (len == 0 || len == MAX_PATH)
			return std::string();

		std::wstring pathStr(widePath, len);
		const size_t lastSlash = pathStr.find_last_of(L"\\/");
		if (lastSlash == std::wstring::npos)
			return std::string();

		const std::wstring folderW = pathStr.substr(0, lastSlash + 1);

		// Convert UTF-16 -> UTF-8
		const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, folderW.c_str(), (int)folderW.size(), nullptr, 0, nullptr, nullptr);
		if (sizeNeeded == 0) return std::string();

		std::string folderA(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, folderW.c_str(), (int)folderW.size(), &folderA[0], sizeNeeded, nullptr, nullptr);

		return folderA;
	}

	//-------------------------------------------------------------------------------------------------------------------
	// Convert UTF-8 std::string to std::wstring (suitable for LPCWSTR APIs)
	std::wstring GLOBALS::ToWString(const std::string& utf8Str)
	{
		if (utf8Str.empty()) return std::wstring();

		const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), nullptr, 0);
		if (sizeNeeded == 0) return std::wstring();

		std::wstring wideStr(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wideStr[0], sizeNeeded);

		return wideStr;
	}
}
