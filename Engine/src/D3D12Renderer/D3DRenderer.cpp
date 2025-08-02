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
	UT_CHECK_BOOL(CreateTriangle());

	// Fill out the Viewport
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = UT::GLOBALS::GWindowWidth;
	m_Viewport.Height = UT::GLOBALS::GWindowHeight;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = UT::GLOBALS::GWindowWidth;
	m_ScissorRect.bottom = UT::GLOBALS::GWindowHeight;

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

	// Set Render Target
	pCommandList->OMSetRenderTargets(1, &m_handlesRTV[frameIndex], false, nullptr);
	pCommandList->ClearRenderTargetView(m_handlesRTV[frameIndex], clearColor, 0, nullptr);

	//-- RENDER!
	Render();

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
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	// Set pipeline
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	pCommandList->SetPipelineState(m_pPSO);

	// Setup viewport and scissor
	pCommandList->RSSetViewports(1, &m_Viewport);
	pCommandList->RSSetScissorRects(1, &m_ScissorRect);

	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw
	pCommandList->DrawInstanced(3, 1, 0, 0);
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

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreateTriangle()
{
	// Create Root Signature
	UT::D3D12::HELPER::CreateRootSignatue(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, &m_pRootSignature);

	// Compile Shaders
	D3D12_SHADER_BYTECODE vsByteCode = {};
	UT::D3D12::HELPER::CompileShader("Triangle.hlsl", "VSMain", "vs_5_0", nullptr, vsByteCode);

	D3D12_SHADER_BYTECODE psByteCode = {};
	UT::D3D12::HELPER::CompileShader("Triangle.hlsl", "PSMain", "ps_5_0", nullptr, psByteCode);

	// Input layput  Description
	D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPC, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPC, Color),    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	const D3D12_INPUT_LAYOUT_DESC inputLayout = { inputLayoutDesc, _countof(inputLayoutDesc) };

	// Create PSO!
	UT::D3D12::HELPER::CreatePSO(m_pRootSignature, vsByteCode, psByteCode, inputLayout, &m_pPSO);

	// Create vertex buffer
	const UT::D3D12::DAS::VertexPC vertices[]  =
	{
		{ XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1, 0, 0, 1) },
		{ XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0, 1, 0, 1) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0, 0, 1, 1) }
	};
	constexpr UINT64 vbSize = sizeof(vertices);

	// Upload data to Vertex buffer on GPU!
	ID3D12Resource* vbUploadBuffer;
	UT::D3D12::HELPER::CreateGPUBuffer(vbSize, &m_pVertexBuffer);
	UT::D3D12::HELPER::CreateUploadBuffer(vbSize, &vbUploadBuffer);
	UT::D3D12::HELPER::CopyDataFromUploadBufferToGPU(vbSize, vertices, vbUploadBuffer, m_pVertexBuffer);

	// Setup Vertex Buffer View
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPC);
	m_VertexBufferView.SizeInBytes = _countof(vertices) * sizeof(UT::D3D12::DAS::VertexPC);

	return true;
}


