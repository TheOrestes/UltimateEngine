#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>


#include "Interfaces/IRenderDevice.h"
#include "D3DGlobals.h"
#include "Core/EngineApplication.h"

class UT_API DXRenderDevice : public IRenderDevice
{
public:
	DXRenderDevice();
	virtual ~DXRenderDevice() override;

	const char*								GetAPIName() override;
	const char*								GetGPUName() override;

	bool									Initialize(HWND hwnd, IDXGIFactory6* pFactory);
	void									Cleanup();
	void									CleanupOnWindowResize();

	void									SignalFence(ID3D12Fence* pFence, uint64_t uiFenceValue) const;
	void									Present() const;
	void									ExecuteCommandLists(ID3D12CommandList** ppCommandLists);

private:
	bool									CreateDevice(IDXGIFactory6* pFactory);
	bool									CreateSwapchain(HWND hwnd, IDXGIFactory6* pFactory);
	bool									CreateDescriptorHeap();
	bool									CreateCommandQueue();
	bool									CreateRenderTargetView();

public:
	inline std::string						GetGPUAdapterName() const				{ return m_strGPUName; }
	inline ID3D12Device*					GetD3DDevice() const					{ return m_pD3DDevice; };
	inline IDXGISwapChain4*					GetD3DSwapChain() const					{ return m_pSwapchain; }
	inline ID3D12Resource*					GetRenderTarget(uint32_t index) const	{ return m_pListD3DRenderTargets.at(index); }
	inline uint32_t							GetCurrentBackbufferIndex() const		{ return m_pSwapchain->GetCurrentBackBufferIndex(); }
	inline uint32_t							GetRTVDescriptorSize() const			{ return m_rtvDescriptorSize; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandle() const			{ return m_pD3DDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); }

private:
	std::string								m_strGPUName;
	ID3D12Device*							m_pD3DDevice;


	ID3D12DebugDevice*						m_pD3DDebugDevice;

	IDXGISwapChain4*						m_pSwapchain;
	ID3D12DescriptorHeap*					m_pD3DDescriptorHeap;
	uint32_t								m_rtvDescriptorSize;

	std::vector<ID3D12Resource*>			m_pListD3DRenderTargets;

	ID3D12CommandQueue*						m_pD3DCommandQueue;
};


