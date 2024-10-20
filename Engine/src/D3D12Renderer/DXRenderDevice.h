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

	inline uint32_t							GetRTVDescriptorSize() const						{ return m_uiDescriptorSizeRenderTargetView; }
	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapRenderTargetView() const			{ return m_pD3DDescriptorHeapRenderTargetView; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleRenderTargetView() const		{ return m_pD3DDescriptorHeapRenderTargetView->GetCPUDescriptorHandleForHeapStart(); }

	inline uint32_t							GetDSVDescriptorSize() const						{ return m_uiDescriptorSizeDepthStencilView; }
	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapDepthStencilView() const			{ return m_pD3DDescriptorHeapDepthStencilView; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleDepthStencilView() const		{ return m_pD3DDescriptorHeapDepthStencilView->GetCPUDescriptorHandleForHeapStart(); }
		   
	inline ComPtr<ID3D12DescriptorHeap>		GetDescriptorHeapShaderResourceView() const			{ return m_pD3DDescriptorHeapShaderResourceView; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleShaderResourceView() const	{ return m_pD3DDescriptorHeapShaderResourceView->GetCPUDescriptorHandleForHeapStart(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE		GetGPUDescriptorHandleShaderResourceView() const	{ return m_pD3DDescriptorHeapShaderResourceView->GetGPUDescriptorHandleForHeapStart(); }

	inline ComPtr<ID3D12CommandQueue>		GetCommandQueue() const								{ return m_pD3DCommandQueue; }

private:
	std::string								m_strGPUName;

	ComPtr<ID3D12Device>					m_pD3DDevice;


	ComPtr<ID3D12DebugDevice>				m_pD3DDebugDevice;

	ComPtr<IDXGISwapChain4>					m_pSwapchain;
	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapRenderTargetView;
	uint32_t								m_uiDescriptorSizeRenderTargetView;

	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapShaderResourceView;
	std::vector<ComPtr<ID3D12Resource>>		m_pListD3DRenderTargetBuffers;
	uint32_t								m_uiDescriptorSizeDepthStencilView;

	ComPtr<ID3D12DescriptorHeap>			m_pD3DDescriptorHeapDepthStencilView;
	ComPtr<ID3D12Resource>					m_pD3DDepthStencilBuffer;

	ComPtr<ID3D12CommandQueue>				m_pD3DCommandQueue;
};


