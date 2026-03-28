#include "UltimateEnginePCH.h"
#include "D3DGlobals.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace UT
{
	namespace D3D12
	{
		//-------------------------------------------------------------------------------------------------------------------
		namespace CORE
		{
			IDXGIFactory6*																		g_pFactory			= nullptr;
			ID3D12Device*																		g_pDevice			= nullptr;
			ID3D12Debug1*																		g_pD3D12Debug		= nullptr;
			ID3D12CommandQueue*																	g_pD3DCommandQueue	= nullptr;
			IDXGISwapChain4*																	g_pD3DSwapChain		= nullptr;
			ID3D12DescriptorHeap*																g_pDescriptorHeap	= nullptr;

			std::array<ID3D12CommandAllocator*, GLOBALS::GFramesInFlight>						m_ListCommandAllocators;
			std::array<ID3D12GraphicsCommandList*, GLOBALS::GFramesInFlight>					m_ListCommandListsGraphics;
			std::array<ID3D12Fence*, GLOBALS::GFramesInFlight>									m_ListFences;
			std::array<HANDLE, GLOBALS::GFramesInFlight>										m_ListFenceEvents;
			std::array<UINT64, GLOBALS::GFramesInFlight>										m_ListFenceValues;

			constexpr IDXGIFactory6*				const GetFactory()							{ return g_pFactory; }
			constexpr ID3D12Device*					const GetDevice()							{ return g_pDevice; }
			constexpr ID3D12CommandQueue*			const GetCommandQueue()						{ return g_pD3DCommandQueue; }
			constexpr IDXGISwapChain4*				const GetSwapchain()						{ return g_pD3DSwapChain; }
			constexpr ID3D12DescriptorHeap*			const GetGlobalDescriptorHeap()				{ return g_pDescriptorHeap; }

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
				SAFE_RELEASE(pTempSwapchain);	 // release temp regardless of QueryInterface result

				if (SUCCEEDED(Hr))
				{
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

				//-- 6. Create Global Descriptor Heap
				D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
				srvHeapDesc.NumDescriptors = UT::GLOBALS::GBindlessHeapSize;
				srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				srvHeapDesc.NodeMask = 0;	// For single-GPU setup, use 0!

				Hr = g_pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&g_pDescriptorHeap));
				UT_CHECK_HRESULT(Hr, "Global Descriptor Heap creation failed!");
				UT_NAME_D3D_OBJECT(g_pDescriptorHeap, "Global Descriptor Heap");

				return true;
			}

			//-------------------------------------------------------------------------------------------------------------------
			void WaitToFinishCurrentFrame()
			{
				const uint16_t currFrameIndex = UT::GLOBALS::GCurrentFrameId;
				//LOG_INFO("=== BeginFrame[{0}] ====", currFrameIndex);

				// Wait for the previous frame to finish!
				if (m_ListFences[currFrameIndex]->GetCompletedValue() < m_ListFenceValues[currFrameIndex])
				{
					m_ListFences[currFrameIndex]->SetEventOnCompletion(m_ListFenceValues[currFrameIndex], m_ListFenceEvents[currFrameIndex]);
					WaitForSingleObject(m_ListFenceEvents[currFrameIndex], INFINITE);
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void BeginFrame()
			{
				const uint16_t currFrameIndex = UT::GLOBALS::GCurrentFrameId;
				//LOG_INFO("=== BeginFrame[{0}] ====", currFrameIndex);

				WaitToFinishCurrentFrame();

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
				// Flush GPU ONCE here — before ANY subsystem cleanup!
				for (uint16_t i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
				{
					UT::GLOBALS::GCurrentFrameId = i;
					UT::D3D12::CORE::WaitToFinishCurrentFrame();

					LOG_INFO("Frame {0} flushed", i);
				}

				for (uint16_t i = 0; i < GLOBALS::GFramesInFlight; ++i)
				{
					SAFE_RELEASE(m_ListFences[i]);
					CloseHandle(m_ListFenceEvents[i]);
					SAFE_RELEASE(m_ListCommandListsGraphics[i]);
					SAFE_RELEASE(m_ListCommandAllocators[i]);
				}

				SAFE_RELEASE(g_pDescriptorHeap);

				if (g_pD3DSwapChain)
				{
					g_pD3DSwapChain->AddRef();
					ULONG refCount = g_pD3DSwapChain->Release();
					LOG_INFO("SwapChain RefCount before release: {0}", refCount);
				}
				SAFE_RELEASE(g_pD3DSwapChain);

				SAFE_RELEASE(g_pD3DCommandQueue);
				SAFE_RELEASE(g_pD3D12Debug);

#if defined(_DEBUG)
				ID3D12DebugDevice* pDebugDevice = nullptr;
				if (SUCCEEDED(g_pDevice->QueryInterface(IID_PPV_ARGS(&pDebugDevice))))
				{
					pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
					SAFE_RELEASE(pDebugDevice);
				}
#endif

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

			//-------------------------------------------------------------------------------------------------------------------
			void ResetCommandList()
			{
				const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
				ID3D12CommandAllocator* const pCmdAlloc = UT::D3D12::CORE::GetCommandAllocator(frameIndex);
				ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

				pCmdAlloc->Reset();
				pCommandList->Reset(pCmdAlloc, nullptr);
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CloseAndExecuteCommandList()
			{
				const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
				ID3D12CommandQueue* const pCmdQueue = UT::D3D12::CORE::GetCommandQueue();
				ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);
				ID3D12Fence* const pFence = UT::D3D12::CORE::GetFence(frameIndex);
				HANDLE const fenceEvent = UT::D3D12::CORE::GetFenceEvent(frameIndex);

				pCommandList->Close();

				ID3D12CommandList* lists[] = { pCommandList };
				pCmdQueue->ExecuteCommandLists(1, lists);

				// Signal Fence
				UT::D3D12::CORE::FenceIncrement();
				const UINT64 fenceValue = UT::D3D12::CORE::GetFenceValue(frameIndex);

				HRESULT Hr = pCmdQueue->Signal(pFence, fenceValue);
				//LOG_DEBUG("CommandList Reset");

				const UINT64 completedFenceValue = pFence->GetCompletedValue();
				if (completedFenceValue < fenceValue)
				{
					Hr = pFence->SetEventOnCompletion(fenceValue, fenceEvent);
					WaitForSingleObject(fenceEvent, INFINITE);
				}
			}
		}

		namespace DAS
		{
			TransformData* g_pTransformData = nullptr;

			constexpr TransformData* const	GetGlobalTransformDataPtr()								{ return g_pTransformData;}
			void							SetGlobalTransformDataPtr(TransformData* transformData) { g_pTransformData = transformData; }
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
				UT_NAME_D3D_OBJECT(*outUploadBuffer, "Upload Buffer");
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
				UT_NAME_D3D_OBJECT(*outReadbackBuffer, "Readback Buffer");
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
				UT::D3D12::CORE::ResetCommandList();
				
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

				UT::D3D12::CORE::CloseAndExecuteCommandList();
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateTransformBuffer(uint32_t maxObjects,  ID3D12Resource** outBuffer, UT::D3D12::DAS::TransformData** outMappedPtr)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
				ID3D12DescriptorHeap* pHeap = UT::D3D12::CORE::GetGlobalDescriptorHeap();

				const UINT64 bufferSize = sizeof(UT::D3D12::DAS::TransformData) * maxObjects;

				// Persistently mapped upload buffer (updated every frame per object)
				CreateUploadBuffer(bufferSize, outBuffer);

				constexpr D3D12_RANGE readRange = { 0, 0 };
				(*outBuffer)->Map(0, &readRange, reinterpret_cast<void**>(outMappedPtr));

				// Register as SRV (StructuredBuffer) at slot 0
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Buffer.NumElements = maxObjects;
				srvDesc.Buffer.StructureByteStride = sizeof(UT::D3D12::DAS::TransformData);
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = pHeap->GetCPUDescriptorHandleForHeapStart();

				// Slot 0 is reserved — do NOT increment GNextDescriptorSlot here
				pDevice->CreateShaderResourceView(*outBuffer, &srvDesc, cpuHandle);

				UT::GLOBALS::GNextDescriptorSlot = 1; // slot 0 taken, start textures from 1
			}

			//-------------------------------------------------------------------------------------------------------------------
			// Returns the bindless index. Replaces CreateTextureSRV() in D3DCube/D3DMesh.
			uint32_t RegisterTextureSRV(ID3D12Resource* pTexture)
			{
				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
				ID3D12DescriptorHeap* pHeap = UT::D3D12::CORE::GetGlobalDescriptorHeap();

				const UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				const UINT slotIndex = UT::GLOBALS::GNextDescriptorSlot++;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;

				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = pHeap->GetCPUDescriptorHandleForHeapStart();
				cpuHandle.ptr += slotIndex * descriptorSize;
				pDevice->CreateShaderResourceView(pTexture, &srvDesc, cpuHandle);

				return slotIndex; // this is the bindless index the object stores
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CompileShader(const std::string& srcFile,
				const std::string& entryPoint,
				const std::string& target,
				const D3D_SHADER_MACRO* pDefines,
				D3D12_SHADER_BYTECODE& outByteCode,
				IDxcBlob** ppCodeOut,
				IDxcBlob** ppSignedBlobOut)
			{
				IDxcUtils*				pUtils		= nullptr;
				IDxcCompiler3*			pCompiler	= nullptr;
				IDxcIncludeHandler*		pInclude	= nullptr;
				IDxcResult*				pResult		= nullptr;
				IDxcBlob*				pCode		= nullptr;
				IDxcValidator*			pValidator	= nullptr;
				IDxcOperationResult*	pValResult	= nullptr;
				IDxcBlob*				pSignedBlob = nullptr;
				std::vector<LPCWSTR>	args;

				// Build full shader path
				const std::string shaderPath = UT::GLOBALS::GetExecutableFolderPath() + "Assets/Shaders/" + srcFile;
				const std::wstring wShaderPath = UT::GLOBALS::ToWString(shaderPath);
				const std::wstring wEntryPoint(entryPoint.begin(), entryPoint.end());
				const std::wstring wTarget(target.begin(), target.end());

				// 1. Create DXC utilities
				HRESULT Hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
				UT_ASSERT_HRESULT(Hr, "DxcCreateInstance => Utils");

				Hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
				UT_ASSERT_HRESULT(Hr, "DxcCreateInstance => Compiler");

				Hr = pUtils->CreateDefaultIncludeHandler(&pInclude);
				UT_ASSERT_HRESULT(Hr, "CreateDefaultIncludeHandler");

				// 2. Load source file
				IDxcBlobEncoding* pSource = nullptr;
				Hr = pUtils->LoadFile(wShaderPath.c_str(), nullptr, &pSource);
				if (FAILED(Hr))
				{
					LOG_CRITICAL("CompileShader => Failed to load shader file: {0}", shaderPath);
					goto cleanup;
				}

				// 3. Build compile arguments
				args.push_back(wShaderPath.c_str());
				args.push_back(L"-E"); args.push_back(wEntryPoint.c_str());
				args.push_back(L"-T"); args.push_back(wTarget.c_str());
				args.push_back(L"-HV"); args.push_back(L"2021");
#if defined(_DEBUG)
				args.push_back(L"-Zi");
				args.push_back(L"-Od");
				args.push_back(L"-Qembed_debug");
#else
				args.push_back(L"-O3");
#endif

				// 4. Compile
				{
					DxcBuffer srcBuffer{};
					srcBuffer.Ptr = pSource->GetBufferPointer();
					srcBuffer.Size = pSource->GetBufferSize();
					srcBuffer.Encoding = DXC_CP_ACP;

					Hr = pCompiler->Compile(&srcBuffer, args.data(), (UINT32)args.size(), pInclude, IID_PPV_ARGS(&pResult));
					SAFE_RELEASE(pSource);

					if (FAILED(Hr))
					{
						LOG_CRITICAL("CompileShader => Compile call failed for: {0}", srcFile);
						goto cleanup;
					}

					// Check for compile errors
					IDxcBlobUtf8* pErrors = nullptr;
					pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
					if (pErrors && pErrors->GetStringLength() > 0)
						LOG_ERROR("Shader Compile Warning/Error [{0}]:\n{1}", srcFile, pErrors->GetStringPointer());
					SAFE_RELEASE(pErrors);

					HRESULT hrStatus = S_OK;
					pResult->GetStatus(&hrStatus);
					if (FAILED(hrStatus))
					{
						LOG_CRITICAL("CompileShader => Shader compilation failed: {0}", srcFile);
						goto cleanup;
					}

					// Get compiled DXIL
					Hr = pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pCode), nullptr);
					if (FAILED(Hr) || !pCode)
					{
						LOG_CRITICAL("CompileShader => Failed to get compiled DXIL: {0}", srcFile);
						goto cleanup;
					}
				}

				// 5. Validate & Sign
				{
					Hr = DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&pValidator));
					UT_ASSERT_HRESULT(Hr, "DxcCreateInstance => Validator");

					DxcBuffer dxilBuffer{};
					dxilBuffer.Ptr = pCode->GetBufferPointer();
					dxilBuffer.Size = pCode->GetBufferSize();
					dxilBuffer.Encoding = 0;

					Hr = pValidator->Validate(pCode, DxcValidatorFlags_InPlaceEdit, &pValResult);
					if (FAILED(Hr))
					{
						LOG_CRITICAL("CompileShader => Validation call failed: {0}", srcFile);
						goto cleanup;
					}

					HRESULT hrValidate = S_OK;
					pValResult->GetStatus(&hrValidate);
					if (FAILED(hrValidate))
					{
						IDxcBlobEncoding* pValErrors = nullptr;
						pValResult->GetErrorBuffer(&pValErrors);
						if (pValErrors)
						{
							LOG_CRITICAL("CompileShader => DXIL Validation failed [{0}]: {1}",
								srcFile,
								static_cast<const char*>(pValErrors->GetBufferPointer()));
							SAFE_RELEASE(pValErrors);
						}
						goto cleanup;
					}

					// Get the signed blob
					pValResult->GetResult(&pSignedBlob);
					if (!pSignedBlob)
					{
						LOG_CRITICAL("CompileShader => Failed to get signed blob: {0}", srcFile);
						goto cleanup;
					}
				}

				// 6. Fill bytecode — point into signed blob's memory
				outByteCode.pShaderBytecode = pSignedBlob->GetBufferPointer();
				outByteCode.BytecodeLength = pSignedBlob->GetBufferSize();

				// 7. Hand ownership to caller — BOTH must stay alive until after CreateGraphicsPipelineState!
				if (ppCodeOut)       *ppCodeOut = pCode;       // caller owns, do NOT release
				else                 SAFE_RELEASE(pCode);

				if (ppSignedBlobOut) *ppSignedBlobOut = pSignedBlob; // caller owns, do NOT release
				else                 SAFE_RELEASE(pSignedBlob);

				LOG_INFO("{0} => {1} compiled + signed OK! [{2} bytes]",
					srcFile.c_str(), entryPoint.c_str(), outByteCode.BytecodeLength);

			cleanup:
				SAFE_RELEASE(pValResult);
				SAFE_RELEASE(pValidator);
				SAFE_RELEASE(pResult);
				SAFE_RELEASE(pInclude);
				SAFE_RELEASE(pCompiler);
				SAFE_RELEASE(pUtils);
				// NOTE: pCode and pSignedBlob are intentionally NOT released here
				//       — they are either owned by caller or released above via ppXxxOut == nullptr path
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
				ID3DBlob* blob	= nullptr;
				ID3DBlob* error	= nullptr;

				HRESULT Hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
				UT_ASSERT_HRESULT(Hr, "SerializeRootSignature");

				Hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(pOutRootSignature));
				UT_ASSERT_HRESULT(Hr, "CreateRootSignature");

				SAFE_RELEASE(blob);
				SAFE_RELEASE(error);
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

			//-------------------------------------------------------------------------------------------------------------------
			void LoadImageData(const std::string& filePath, int* width, int* height, int* channels, void** outImagaData)
			{
				// Convert to the full path & widestring before passing it to the D3D function!
				const std::string imagePath = UT::GLOBALS::GetExecutableFolderPath() + filePath;
				//const std::wstring wideStr = UT::GLOBALS::ToWString(imagePath);

				int uWidth, uHeight, uChannels = 0;
				stbi_set_flip_vertically_on_load(true);
				unsigned char* data = stbi_load(imagePath.c_str(), width, height, channels, 4);

				if(data)
				{
					*outImagaData = reinterpret_cast<void*>(data);
					*channels = 4;
				}
				else
				{
					LOG_ERROR("{0} Image Loading failed!", filePath.c_str());
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateTexture(const std::string& filePath, ID3D12Resource** outTexture)
			{
				ID3D12Resource* pTexture = nullptr;

				ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
				const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
				ID3D12CommandAllocator* const pCmdAlloc = UT::D3D12::CORE::GetCommandAllocator(frameIndex);
				ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

				int width, height, channels = 0;
				void* imgData = nullptr;

				// Load image data from the file!
				LoadImageData(filePath, &width, &height, &channels, &imgData);

				// Create Texture resource!
				if (imgData)
				{
					// Describe Texture resource
					D3D12_RESOURCE_DESC texDesc = {};
					texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
					texDesc.Width = width;
					texDesc.Height = height;
					texDesc.DepthOrArraySize = 1;
					texDesc.MipLevels = 1;
					texDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
					texDesc.SampleDesc.Count = 1;
					texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
					texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

					D3D12_HEAP_PROPERTIES heapProps = {};
					heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

					// Create default heap resource
					HRESULT Hr = pDevice->CreateCommittedResource(&heapProps,
																 D3D12_HEAP_FLAG_NONE,
																 &texDesc,
																 D3D12_RESOURCE_STATE_COPY_DEST,
																 nullptr,
																 IID_PPV_ARGS(&pTexture));

					UT_ASSERT_HRESULT(Hr, "CreateResource => Texture Buffer");
					UT_NAME_D3D_OBJECT(pTexture, "TextureResource: " + UT::GLOBALS::GetFileNameWithoutExtension(filePath));

					//-- Create upload buffer for the texture!
					UINT64 uploadBufferSize = 0;
					D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
					UINT numRows = 0;
					UINT64 rowSizeInBytes = 0; UINT64 totalBytes = 0;

					pDevice->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

					ID3D12Resource* pUploadHeap = nullptr;
					UT::D3D12::HELPER::CreateUploadBuffer(totalBytes, &pUploadHeap);

					//-- Write image to the upload heap!
					UINT8* pUploadData = nullptr;

					Hr = pUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&pUploadData));
					UT_CHECK_HRESULT(Hr, "Map Texture buffer failed!");

					for (UINT row = 0; row < numRows; ++row)
					{
						memcpy(pUploadData + footprint.Offset + row * footprint.Footprint.RowPitch,
							static_cast<const UINT8*>(imgData) + row * rowSizeInBytes,
							rowSizeInBytes);
					}

					pUploadHeap->Unmap(0, nullptr);

					//-- Copy to the Default Heap (GPU)!
					D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
					dstLocation.pResource = pTexture;
					dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dstLocation.SubresourceIndex = 0;

					D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
					srcLocation.pResource = pUploadHeap;
					srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					srcLocation.PlacedFootprint = footprint;

					UT::D3D12::CORE::ResetCommandList();

					pCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = pTexture;
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
					barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

					pCommandList->ResourceBarrier(1, &barrier);

					UT::D3D12::CORE::CloseAndExecuteCommandList();

					SAFE_RELEASE(pUploadHeap);
				}

				// pass out the created texture resource!
				*outTexture = pTexture;
			}
		}
	}

	//-------------------------------------------------------------------------------------------------------------------
	std::string GLOBALS::GetFileNameWithoutExtension(const std::string& fileName)
	{
		// Find last slash or backslash
		const size_t lastSlash = fileName.find_last_of("/\\");

		// Extract filename with extension
		const std::string filenameWithExt = (lastSlash != std::string::npos) ? fileName.substr(lastSlash + 1) : fileName;

		// Find last dot for extension
		const size_t lastDot = filenameWithExt.find_last_of('.');
		std::string filenameWithoutExt = (lastDot != std::string::npos) ? filenameWithExt.substr(0, lastDot) : filenameWithExt;

		return filenameWithoutExt;
	}

	//-------------------------------------------------------------------------------------------------------------------
	std::string GLOBALS::GetExecutableFolderPath()
	{
		WCHAR widePath[MAX_PATH] = {};
		DWORD len = GetModuleFileNameW(nullptr, widePath, MAX_PATH);
		if (len == 0 || len == MAX_PATH)
			return std::string();

		const std::wstring pathStr(widePath, len);
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
