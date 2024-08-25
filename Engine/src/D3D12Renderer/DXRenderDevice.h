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

	bool									Initialize(HWND hwnd, ComPtr<IDXGIFactory6> pFactory);
	void									Cleanup();
	void									CleanupOnWindowResize();
	void									RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight);

	void									CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdListType, ComPtr<ID3D12CommandAllocator>& pOutCmdAllocator);
	void									CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE cmdListType, const ComPtr<ID3D12CommandAllocator>& pCmdAllocator, ComPtr<ID3D12GraphicsCommandList>& pOutCmdList);
	void									CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS fenceFlags, ComPtr<ID3D12Fence>& pOutFence);

	void									SignalFence(ComPtr<ID3D12Fence> pFence, uint64_t uiFenceValue) const;
	void									Present() const;
	void									ExecuteCommandLists(const std::vector<ComPtr<ID3D12CommandList>>& vecCommandList);

private:
	bool									CreateDevice(ComPtr<IDXGIFactory6> pFactory);
	bool									CreateSwapchain(HWND hwnd, ComPtr<IDXGIFactory6> pFactory);
	bool									CreateDescriptorHeap();
	bool									CreateCommandQueue();
	bool									CreateRenderTargetView();

	

public:
	inline std::string						GetGPUAdapterName() const				{ return m_strGPUName; }
	inline ComPtr<ID3D12Device>				GetD3DDevice() const					{ return m_pD3DDevice; };
	inline ComPtr<IDXGISwapChain4>			GetD3DSwapChain() const					{ return m_pSwapchain; }
	inline ComPtr<ID3D12Resource>			GetRenderTarget(uint32_t index) const	{ return m_pListD3DRenderTargets.at(index); }
	inline uint32_t							GetCurrentBackbufferIndex() const		{ return m_pSwapchain->GetCurrentBackBufferIndex(); }
	inline uint32_t							GetRTVDescriptorSize() const			{ return m_uiDescriptorSizeRTV; }

	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapRTV() const			{ return m_pD3DDescriptorHeapRTV; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleRTV() const		{ return m_pD3DDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(); }

	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapSRV() const			{ return m_pD3DDescriptorHeapSRV; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleSRV() const		{ return m_pD3DDescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE		GetGPUDescriptorHandleSRV() const		{ return m_pD3DDescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart(); }

private:
	std::string								m_strGPUName;

	ComPtr<ID3D12Device>					m_pD3DDevice;


	ComPtr<ID3D12DebugDevice>				m_pD3DDebugDevice;

	ComPtr<IDXGISwapChain4>					m_pSwapchain;
	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapRTV;
	uint32_t								m_uiDescriptorSizeRTV;

	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapSRV;

	std::vector<ComPtr<ID3D12Resource>>		m_pListD3DRenderTargets;

	ComPtr<ID3D12CommandQueue>				m_pD3DCommandQueue;
};


