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

	bool									Initialize(HWND hwnd, const IDXGIFactory6* pFactory);
	void									Cleanup();
	void									CleanupOnWindowResize();
	void									RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight);

	bool									CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdListType, ID3D12CommandAllocator** pOutCmdAllocator);
	bool									CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE cmdListType, ID3D12CommandAllocator* pCmdAllocator, ID3D12GraphicsCommandList** pOutCmdList);
	bool									CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS fenceFlags, ID3D12Fence** pOutFence);

	void									SignalFence(ID3D12Fence* pFence, uint64_t uiFenceValue) const;
	void									Present() const;
	void									ExecuteCommandLists(const std::vector<ID3D12CommandList*> vecCommandList);

private:
	bool									CreateDevice(const IDXGIFactory6* pFactory);
	bool									CreateSwapchain(HWND hwnd, const IDXGIFactory6* pFactory);
	bool									CreateDescriptorHeap();
	bool									CreateCommandQueue();
	bool									CreateRenderTargetView();

	

public:
	inline std::string						GetGPUAdapterName() const							{ return m_strGPUName; }
	inline ID3D12Device*					GetD3DDevice() const								{ return m_pD3DDevice; };
	inline IDXGISwapChain4*					GetD3DSwapChain() const								{ return m_pSwapchain; }
	inline ID3D12Resource*					GetRenderTarget(uint32_t index) const				{ return m_pListD3DRenderTargetBuffers.at(index); }
	inline uint32_t							GetCurrentBackbufferIndex() const					{ return m_pSwapchain->GetCurrentBackBufferIndex(); }

	inline uint32_t							GetRTVDescriptorSize() const						{ return m_uiDescriptorSizeRTV; }
	inline ID3D12DescriptorHeap*			GetDescriptorHeapRTV() const						{ return m_pD3DDescriptorHeapRTV; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleRTV() const					{ return m_pD3DDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(); }

	inline uint32_t							GetDSVDescriptorSize() const						{ return m_uiDescriptorSizeDSV; }
	inline ID3D12DescriptorHeap*			GetDescriptorHeapDSV() const						{ return m_pD3DDescriptorHeapDSV; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleDSV() const					{ return m_pD3DDescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart(); }
		   
	inline ID3D12DescriptorHeap*			GetDescriptorHeapUI() const							{ return m_pD3DDescriptorHeapUI; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE		GetCPUDescriptorHandleUI() const					{ return m_pD3DDescriptorHeapUI->GetCPUDescriptorHandleForHeapStart(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE		GetGPUDescriptorHandleUI() const					{ return m_pD3DDescriptorHeapUI->GetGPUDescriptorHandleForHeapStart(); }

	inline ID3D12CommandQueue*				GetCommandQueue() const								{ return m_pD3DCommandQueue; }

private:
	std::string								m_strGPUName;

	ID3D12Device*							m_pD3DDevice;
	ID3D12DebugDevice*						m_pD3DDebugDevice;
	IDXGISwapChain4*						m_pSwapchain;
	ID3D12CommandQueue*						m_pD3DCommandQueue;

	// Descriptor setup RTV
	ID3D12DescriptorHeap*					m_pD3DDescriptorHeapRTV;
	uint32_t								m_uiDescriptorSizeRTV;
	std::vector<ID3D12Resource*>			m_pListD3DRenderTargetBuffers;

	// Descriptor setup DSV
	ID3D12DescriptorHeap*					m_pD3DDescriptorHeapDSV;
	uint32_t								m_uiDescriptorSizeDSV;
	ID3D12Resource*							m_pD3DDepthStencilBuffer;

	// Descriptor setup CBV | SRV | UAV
	ID3D12DescriptorHeap*					m_pD3DDescriptorHeapUI;
};


