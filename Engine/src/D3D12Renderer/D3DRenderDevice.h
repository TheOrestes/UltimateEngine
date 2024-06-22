#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Core/EngineApplication.h"

class UT_API D3DRenderDevice
{
public:
	D3DRenderDevice();
	virtual ~D3DRenderDevice();

	bool					Initialize(IDXGIFactory6* pFactory);
	void					Cleanup();
	void					CleanupOnWindowResize();

private:
	ID3D12Device*			m_pD3DDevice;
	ID3D12DebugDevice*		m_pD3DDebugDevice;
};

