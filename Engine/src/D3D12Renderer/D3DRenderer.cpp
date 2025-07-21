#include "UltimateEnginePCH.h"
#include "D3DRenderer.h"
#include "D3DGlobals.h"

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::D3DRenderer()
{
	
}

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::~D3DRenderer()
{
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::Initialize()
{
	UT_CHECK_BOOL(CreateRTV());
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::RecordCommands()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	//-- Transition to RENDER_TARGET before rendering
	D3D12_RESOURCE_BARRIER rtBarrier = {};
	rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rtBarrier.Transition.pResource = m_ResourceRT[frameIndex];
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &rtBarrier);

	constexpr float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f }; // RGBA (Blueish)

	//-- RENDER!
	Render();

	// Set Render Target
	pCommandList->OMSetRenderTargets(1, &m_handlesRTV[frameIndex], false, nullptr);
	pCommandList->ClearRenderTargetView(m_handlesRTV[frameIndex], clearColor, 0, nullptr);

	//-- Transition to PRESENT before swapping!
	D3D12_RESOURCE_BARRIER presentBarrier = {};
	presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentBarrier.Transition.pResource = m_ResourceRT[frameIndex];
	presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &presentBarrier);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::Render()
{
	
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreateRTV()
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	IDXGISwapChain4* const pSwapchain = UT::D3D12::CORE::GetSwapchain();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = UT::GLOBALS::GFramesInFlight;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	UT_ASSERT_HRESULT(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pHeapRTV)));
	UT_NAME_D3D_OBJECT(m_pHeapRTV, "Descriptor Heap RTV");

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++) 
	{
		pSwapchain->GetBuffer(i, IID_PPV_ARGS(&m_ResourceRT[i]));

		pDevice->CreateRenderTargetView(m_ResourceRT[i], nullptr, rtvHandle);
		m_handlesRTV[i] = rtvHandle;
		rtvHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return true;
}


