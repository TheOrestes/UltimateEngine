#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>


#include "Interfaces/IRenderDevice.h"
#include "D3DGlobals.h"
#include "Core/EngineApplication.h"

class DXRenderDevice : public IRenderDevice
{
public:
	DXRenderDevice();
	virtual ~DXRenderDevice() override;

	const char*								GetAPIName() override;
	const char*								GetGPUName() override;

	bool									Initialize(HWND hwnd, ComPtr<IDXGIFactory6>& pFactory);
	void									Cleanup();
	void									CleanupOnWindowResize();
	void									RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight);

	bool									CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdListType, ComPtr<ID3D12CommandAllocator>& pOutCmdAllocator);
	bool									CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE cmdListType, const ComPtr<ID3D12CommandAllocator>& pCmdAllocator, ComPtr<ID3D12GraphicsCommandList>& pOutCmdList);
	bool									CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS fenceFlags, ComPtr<ID3D12Fence>& pOutFence);

	void									SignalFence(const ComPtr<ID3D12Fence>& pFence, uint64_t uiFenceValue) const;
	void									Present() const;
	void									ExecuteCommandLists(const std::vector<ComPtr<ID3D12CommandList>>& vecCommandList);

private:
	bool									CreateDevice(const ComPtr<IDXGIFactory6>& pFactory);
	bool									CreateSwapchain(HWND hwnd, const ComPtr<IDXGIFactory6>& pFactory);
	bool									CreateDescriptorHeap();
	bool									CreateCommandQueue();
	bool									CreateRenderTargetView();

	

public:
	inline std::string						GetGPUAdapterName() const							{ return m_strGPUName; }
	inline ComPtr<ID3D12Device>				GetD3DDevice() const								{ return m_pD3DDevice; };
	inline ComPtr<IDXGISwapChain4>			GetD3DSwapChain() const								{ return m_pSwapchain; }
	inline ComPtr<ID3D12Resource>			GetRenderTarget(uint32_t index) const				{ return m_pListD3DRenderTargetBuffers.at(index); }
	inline uint32_t							GetCurrentBackbufferIndex() const					{ return m_pSwapchain->GetCurrentBackBufferIndex(); }

	inline uint32_t							GetRTVDescriptorSize() const						{ return m_uiDescriptorSizeRTV; }
	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapRTV() const						{ return m_pD3DDescriptorHeapRTV; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleRTV() const					{ return m_pD3DDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(); }

	inline uint32_t							GetDSVDescriptorSize() const						{ return m_uiDescriptorSizeDSV; }
	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapDSV() const						{ return m_pD3DDescriptorHeapDSV; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleDSV() const					{ return m_pD3DDescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart(); }
		   
	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapUI() const							{ return m_pD3DDescriptorHeapUI; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleUI() const					{ return m_pD3DDescriptorHeapUI->GetCPUDescriptorHandleForHeapStart(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE		GetGPUDescriptorHandleUI() const					{ return m_pD3DDescriptorHeapUI->GetGPUDescriptorHandleForHeapStart(); }

	inline ComPtr<ID3D12CommandQueue>		GetCommandQueue() const								{ return m_pD3DCommandQueue; }

private:
	std::string								m_strGPUName;

	ComPtr<ID3D12Device>					m_pD3DDevice;
	ComPtr<ID3D12DebugDevice>				m_pD3DDebugDevice;
	ComPtr<IDXGISwapChain4>					m_pSwapchain;
	ComPtr<ID3D12CommandQueue>				m_pD3DCommandQueue;

	// Descriptor setup RTV
	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapRTV;
	uint32_t								m_uiDescriptorSizeRTV;
	std::vector<ComPtr<ID3D12Resource>>		m_pListD3DRenderTargetBuffers;

	// Descriptor setup DSV
	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapDSV;
	uint32_t								m_uiDescriptorSizeDSV;
	ComPtr<ID3D12Resource>					m_pD3DDepthStencilBuffer;

	// Descriptor setup CBV | SRV | UAV
	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapUI;
};


