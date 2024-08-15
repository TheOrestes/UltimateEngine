#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "D3DGlobals.h"
#include "DXRenderDevice.h"
#include "EngineHeader.h"
#include "Core/EngineApplication.h"

class DXRenderDevice;

class UT_API DXRenderer
{
public:
	DXRenderer();
	virtual ~DXRenderer();

	bool									Initialize(const GLFWwindow* pWindow, ComPtr<IDXGIFactory6> pFactory);
	void									Render();
	void									Cleanup();

	void									CleanupOnWindowResize();
	void									RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight);

private:
	bool									CreateCommandList();
	bool									CreateCommandAllocator();

	uint32_t								WaitForPreviousFrame();
	uint32_t								BeginFrame();
	void									EndFrame(uint32_t currRenderTargetID);
	void									RecordCommands(uint32_t currFrameIndex);
	void									DrawCommands();
	bool									CreateFences();

	void									ResetCommandAllocator(uint32_t renderTargetID) const;
	void									ResetCommandList(uint32_t renderTargetID) const;

private:
	std::vector<ComPtr<ID3D12CommandAllocator>>	m_pListD3DCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>			m_pD3DGraphicsCommandList;

	std::vector<ComPtr<ID3D12Fence>>			m_pListFences;
	std::vector<uint64_t>						m_pListFenceValue;
	HANDLE										m_handleFenceEvent;

	uint32_t									m_uiCurrentFrameIndex;
	bool										m_bIsCurrentFrameRunning;

	DXRenderDevice*								m_pDXRenderDevice;

	DirectX::XMFLOAT4							m_colorClear;
};

