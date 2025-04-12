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
			IDXGIFactory6*		g_pFactory = nullptr;
			ID3D12Device*		g_pDevice = nullptr;
			ID3D12CommandQueue* g_pCommandQueue = nullptr;

			constexpr IDXGIFactory6*		const		GetFactory()		{ return g_pFactory; }
			constexpr ID3D12Device*			const		GetDevice()			{ return g_pDevice; }
			constexpr ID3D12CommandQueue*	const		GetCommandQueue()	{ return g_pCommandQueue; }

			bool Initialize()
			{
				//--- 1. Create factory
				UINT dxgiFactoryFlags = 0;

			#if defined _DEBUG
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
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

				//--- 3. Create command queue!
				D3D12_COMMAND_QUEUE_DESC queueDesc = {};
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

				Hr = g_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_pCommandQueue));

				UT_CHECK_HRESULT(Hr, "CreateCommandQueue", magic_enum::enum_name(queueDesc.Type));
				UT_NAME_D3D_OBJECT(g_pCommandQueue, "Command Queue");
				
				return true;
			}

			void Cleanup()
			{
				SAFE_RELEASE(g_pCommandQueue);
				SAFE_RELEASE(g_pDevice);
				SAFE_RELEASE(g_pFactory);
			}
		}

		//-------------------------------------------------------------------------------------------------------------------
		namespace HelperFunc
		{
			std::string GetExecutablePath()
			{
				std::string returnPath{};

				WCHAR ownPath[MAX_PATH];

#ifdef UT_PLATFORM_WINDOWS
				const HMODULE hModule = GetModuleHandle(0);
				if(hModule != nullptr)
				{
					GetModuleFileNameW(hModule, ownPath, (sizeof(ownPath)));
					std::wstring tempW(&ownPath[0]);
					returnPath = std::string(tempW.begin(), tempW.end());
				}

				return returnPath;
#endif
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateVertexShader(const std::string& vsFile, ID3DBlob** vertexShaderBlob)
			{
				//UT_ASSERT_NULL(vertexShaderBlob);

				ID3DBlob* errorBlob;

				const std::wstring sTemp = std::wstring(vsFile.begin(), vsFile.end());
				const LPCWSTR ws = sTemp.c_str();

				HRESULT Hr = D3DCompileFromFile(ws, nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vertexShaderBlob, &errorBlob);

				if (FAILED(Hr) && errorBlob != nullptr)
				{
					const char* ErrorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
					UT_ASSERT_HRESULT(Hr, "CreateVertexShader", ErrorMsg);
				}

				// Extract the shader name getting compiled...
				std::size_t pos = vsFile.find_last_of("/");
				std::string shaderName = vsFile.substr(++pos);
				LOG_DEBUG("{0} => Vertex Shader Compiled", shaderName);
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateFragmentShader(const std::string& fsFile, ID3DBlob** fragmentShaderBlob)
			{
				//UT_ASSERT_NULL(fragmentShaderBlob);

				ID3DBlob* errorBlob;

				const std::wstring sTemp = std::wstring(fsFile.begin(), fsFile.end());
				const LPCWSTR ws = sTemp.c_str();

				HRESULT Hr = D3DCompileFromFile(ws, nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, fragmentShaderBlob, &errorBlob);

				if (FAILED(Hr) && errorBlob != nullptr)
				{
					const char* ErrorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
					UT_ASSERT_HRESULT(Hr, "CreateFragmentShader", ErrorMsg);
				}

				// Extract the shader name getting compiled...
				std::size_t pos = fsFile.find_last_of("/");
				std::string shaderName = fsFile.substr(++pos);
				LOG_DEBUG("{0} => Fragment Shader Compiled", shaderName);
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateVertexInputLayoutDesc(D3D12_INPUT_LAYOUT_DESC& outLayoutDesc)
			{
				D3D12_INPUT_ELEMENT_DESC inputLayout[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
				};

				// fill out an input layout description structure
				outLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
				outLayoutDesc.pInputElementDescs = inputLayout;
			}

			//-------------------------------------------------------------------------------------------------------------------
			unsigned char* Load_STB_Image(const std::string& fileName, int& width, int& height, int& channels)
			{
				const std::string gamePath = GetExecutablePath();
				int pos = gamePath.find_last_of("\\");
				std::string imgPath = gamePath.substr(0, pos+1) + fileName;
				unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &channels, 4);

				UT_ASSERT_NULL(data);

				return data;
			}

			//-------------------------------------------------------------------------------------------------------------------
			void CreateTexture2D(const std::string& fileName, ID3D12Resource** pTexture)
			{
				HRESULT Hr = 0;
				ID3D12Device* const pDevice = CORE::g_pDevice;
				ID3D12CommandQueue* const pCmdQueue = CORE::g_pCommandQueue;

				UT_ASSERT_NULL(pDevice);
				UT_ASSERT_NULL(pTexture);

				int imgWidth, imgHeight, imgChannels = 0;
				unsigned char* imgData = Load_STB_Image(fileName.c_str(), imgWidth, imgHeight, imgChannels);

				// Create the texture resource
				D3D12_RESOURCE_DESC textureDesc = {};
				textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				textureDesc.Alignment = 0;
				textureDesc.Width = imgWidth;
				textureDesc.Height = imgHeight;
				textureDesc.DepthOrArraySize = 1;
				textureDesc.MipLevels = 1;
				textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				textureDesc.SampleDesc.Count = 1;
				textureDesc.SampleDesc.Quality = 0;
				textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				D3D12_HEAP_PROPERTIES texHeapProperties = {};
				texHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
				texHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				texHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

				Hr = pDevice->CreateCommittedResource(
					&texHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&textureDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(pTexture));

				UT_CHECK_HRESULT(Hr, "CreateCommittedResource", "Texture Resource");
				UT_NAME_D3D_OBJECT(*pTexture, "Texture Resource");

				// Create an upload heap for texture data
				ID3D12Resource* textureUploadHeap;
				UINT64 textureUploadBufferSize;
				pDevice->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

				D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
				uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

				D3D12_RESOURCE_DESC uploadResourceDescription = {};
				uploadResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				uploadResourceDescription.Alignment = 0;
				uploadResourceDescription.Width = textureUploadBufferSize;
				uploadResourceDescription.Height = 1;
				uploadResourceDescription.DepthOrArraySize = 1;
				uploadResourceDescription.MipLevels = 1;
				uploadResourceDescription.Format = DXGI_FORMAT_UNKNOWN;
				uploadResourceDescription.SampleDesc.Count = 1;
				uploadResourceDescription.SampleDesc.Quality = 0;
				uploadResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				uploadResourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

				Hr = pDevice->CreateCommittedResource(
					&uploadHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&uploadResourceDescription,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&textureUploadHeap));

				UT_CHECK_HRESULT(Hr, "CreateCommittedResource", "Texture Upload Heap");
				UT_NAME_D3D_OBJECT(textureUploadHeap, "Texture Upload Heap");

				// Upload the texture data
				void* mappedTextureData;

				textureUploadHeap->Map(0, nullptr, &mappedTextureData);
				memcpy(mappedTextureData, imgData, imgWidth * imgHeight * 4);
				textureUploadHeap->Unmap(0, nullptr);

				// Free stb-image allocated memory!
				stbi_image_free(imgData);

				D3D12_TEXTURE_COPY_LOCATION dst = {};
				dst.pResource = *pTexture;
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = 0;

				D3D12_TEXTURE_COPY_LOCATION src = {};
				src.pResource = textureUploadHeap;
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				pDevice->GetCopyableFootprints(&textureDesc, 0, 1, 0, &src.PlacedFootprint, nullptr, nullptr, nullptr);

				ID3D12CommandAllocator* pCmdAllocator;
				Hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCmdAllocator));
				UT_NAME_D3D_OBJECT(pCmdAllocator, "CreateTexture2D Command Allocator");
				UT_ASSERT_HRESULT(Hr, "CreateCommandAllocator", magic_enum::enum_name(D3D12_COMMAND_LIST_TYPE_DIRECT));

				ID3D12GraphicsCommandList* pCmdList;
				Hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCmdAllocator, nullptr, IID_PPV_ARGS(&pCmdList));
				UT_ASSERT_HRESULT(Hr, "CreateCommandList", magic_enum::enum_name(D3D12_COMMAND_LIST_TYPE_DIRECT));

				pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

				// Transition the texture to the PIXEL_SHADER_RESOURCE state
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = *pTexture;
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

				pCmdList->ResourceBarrier(1, &barrier);
				pCmdList->Close();

				// Execute command list
				std::array<ID3D12CommandList*, 1> cmdLists = { pCmdList };
				pCmdQueue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());

				//SAFE_RELEASE(textureUploadHeap);
				SAFE_RELEASE(pCmdList);
				SAFE_RELEASE(pCmdAllocator);
			}
		}
	}

}
