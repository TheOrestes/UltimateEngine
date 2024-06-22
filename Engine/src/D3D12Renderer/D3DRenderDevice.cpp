#include "UltimateEnginePCH.h"
#include "D3DRenderDevice.h"
#include <dxgi1_4.h>

#include "EngineHeader.h"

//---------------------------------------------------------------------------------------------------------------------
D3DRenderDevice::D3DRenderDevice()
{
	m_pD3DDevice = nullptr;
	m_pD3DDebugDevice = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
D3DRenderDevice::~D3DRenderDevice()
{
	SAFE_RELEASE(m_pD3DDebugDevice);
	SAFE_RELEASE(m_pD3DDevice);
}

//---------------------------------------------------------------------------------------------------------------------
bool D3DRenderDevice::Initialize(IDXGIFactory6* pFactory)
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
	if (FAILED(m_pD3DDevice->QueryInterface(&m_pD3DDebugDevice)))
		return false;
#endif

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void D3DRenderDevice::Cleanup()
{
}

//---------------------------------------------------------------------------------------------------------------------
void D3DRenderDevice::CleanupOnWindowResize()
{
}
