#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "D3DGlobals.h"
#include "DXRenderDevice.h"
#include "EngineHeader.h"
#include "Core/EngineApplication.h"

class DXRenderDevice;
class UIRenderer;

class DXRenderer
{
public:
	DXRenderer();
	virtual ~DXRenderer();

	bool										Initialize(const GLFWwindow* pWindow, ComPtr<IDXGIFactory6> pFactory);
	void										Render();
	void										Cleanup();

	void										CleanupOnWindowResize();
	void										RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight);

private:
	bool										CreateCommandList();
	bool										CreateCommandAllocator();

	uint32_t									WaitForPreviousFrame();
	uint32_t									BeginFrame();
	void										EndFrame(uint32_t currRenderTargetID);
	void										RecordCommands(uint32_t currFrameIndex);
	void										DrawCommands();
	bool										CreateFences();

	void										ResetCommandAllocator(uint32_t renderTargetID) const;
	void										ResetCommandList(uint32_t renderTargetID) const;

private:
	std::vector<ComPtr<ID3D12CommandAllocator>>	m_pListD3DCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>			m_pD3DGraphicsCommandList;

	std::vector<ComPtr<ID3D12Fence>>			m_pListFences;
	std::vector<uint64_t>						m_pListFenceValue;
	HANDLE										m_handleFenceEvent;

	uint32_t									m_uiCurrentFrameIndex;
	bool										m_bIsCurrentFrameRunning;

	DXRenderDevice*								m_pDXRenderDevice;
	UIRenderer*									m_pUIRenderer;

	DirectX::XMFLOAT4							m_colorClear;

	ComPtr<ID3D12PipelineState>					m_pPSO;
	ComPtr<ID3D12RootSignature>					m_pRootSignature;
	D3D12_VIEWPORT								m_Viewport;
	D3D12_RECT									m_ScissorRect;
	ComPtr<ID3D12Resource>						m_pVBuffer;
	ComPtr<ID3D12Resource>						m_pIBuffer;
	D3D12_VERTEX_BUFFER_VIEW					m_VBView;
	D3D12_INDEX_BUFFER_VIEW						m_IBView;
};

