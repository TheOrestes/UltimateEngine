#include "UltimateEnginePCH.h"
#include "D3DGlobals.h"

namespace UT
{
	namespace D3D12
	{
		//-------------------------------------------------------------------------------------------------------------------
		namespace CORE
		{
			IDXGIFactory6* g_pFactory = nullptr;
			ID3D12Device* g_pDevice = nullptr;

			constexpr IDXGIFactory6* const GetFactory() { return g_pFactory; }
			constexpr ID3D12Device* const GetDevice() { return g_pDevice; }

			bool Initialize()
			{
				//--- 1. Create factory
				UINT dxgiFactoryFlags = 0;

#if defined _DEBUG
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

				const HRESULT Hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&g_pFactory));
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

				return true;
			}

			void Cleanup()
			{
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
