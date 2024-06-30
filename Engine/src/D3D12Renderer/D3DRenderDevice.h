#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "D3DGlobals.h"
#include "Core/EngineApplication.h"

class UT_API D3DRenderDevice
{
public:
	D3DRenderDevice();
	virtual ~D3DRenderDevice();

	bool									Initialize(HWND hwnd, IDXGIFactory6* pFactory);
	void									Cleanup();
	void									CleanupOnWindowResize();

private:
	bool									CreateDevice(IDXGIFactory6* pFactory);
	bool									CreateCommandQueue();
	bool									CreateCommandAllocator();
	bool									CreateSwapchain(HWND hwnd, IDXGIFactory6* pFactory);
	bool									CreateDescriptorHeap();
	bool									CreateRenderTargetView();
	bool									CreateCommandList();

private:
	ID3D12Device*							m_pD3DDevice;
	ID3D12DebugDevice*						m_pD3DDebugDevice;

	IDXGISwapChain4*						m_pSwapchain;
	ID3D12DescriptorHeap*					m_pD3DDescriptorHeap;

	std::vector<ID3D12Resource*>			m_pArrD3DRenderTargets;
	
	std::vector<ID3D12CommandAllocator*>	m_pArrD3DCommandAllocator;
	ID3D12CommandQueue*						m_pD3DCommandQueue;
	ID3D12GraphicsCommandList*				m_pD3DGraphicsCommandList;

	uint32_t								m_uiCurrentFrameIndex;
};

