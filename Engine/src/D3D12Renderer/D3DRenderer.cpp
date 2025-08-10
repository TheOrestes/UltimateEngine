#include "UltimateEnginePCH.h"
#include "D3DRenderer.h"
#include "D3DGlobals.h"
#include "StaticMesh/D3DCube.h"

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::D3DRenderer()
{
	
}

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::~D3DRenderer()
{
	SAFE_DELETE(m_pCubeRed);
	SAFE_DELETE(m_pCubeGreen);
	SAFE_DELETE(m_pCubeBlue);
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::Initialize()
{
	UT_CHECK_BOOL(CreateRTV());
	UT_CHECK_BOOL(CreateDSV());
	UT_CHECK_BOOL(CreatePSO());
	//UT_CHECK_BOOL(CreateTriangle());
	//UT_CHECK_BOOL(CreateCube());
	//UT_CHECK_BOOL(CreateConstantBuffer());

	D3DCube::CreateStaticGeometry();

	m_pCubeRed = new D3DCube();
	m_pCubeRed->SetWorld(XMMatrixTranslation(-2, 0, 0));
	m_pCubeRed->SetColor(XMFLOAT4(1, 0, 0, 1));

	m_pCubeGreen = new D3DCube();
	m_pCubeGreen->SetWorld(XMMatrixTranslation(0, 0, 0));
	m_pCubeGreen->SetColor(XMFLOAT4(0, 1, 0, 1));

	m_pCubeBlue = new D3DCube();
	m_pCubeBlue->SetWorld(XMMatrixTranslation(2, 0, 0));
	m_pCubeBlue->SetColor(XMFLOAT4(0, 0, 1, 1));


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

	// Setup viewport and scissor
	pCommandList->RSSetViewports(1, &m_Viewport);
	pCommandList->RSSetScissorRects(1, &m_ScissorRect);

	//-- Transition to RENDER_TARGET before rendering
	D3D12_RESOURCE_BARRIER rtBarrier = {};
	rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rtBarrier.Transition.pResource = m_listRTBuffers[frameIndex];
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &rtBarrier);

	// Set Render Target
	pCommandList->OMSetRenderTargets(1, &m_handlesRTV[frameIndex], false, &m_handlesDSV[frameIndex]);

	constexpr float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f }; // RGBA (Blueish)

	pCommandList->ClearRenderTargetView(m_handlesRTV[frameIndex], clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(m_handlesDSV[frameIndex], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//-- RENDER!
	Render();

	//-- Transition to PRESENT before swapping!
	D3D12_RESOURCE_BARRIER presentBarrier = {};
	presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentBarrier.Transition.pResource = m_listRTBuffers[frameIndex];
	presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &presentBarrier);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::Update(double dt)
{
	static float rotationAngle = 0.0f;

	const XMVECTOR eyePos = XMVectorSet(0.0f, 1.0f, -3.0f, 0.0f);
	const XMVECTOR focusPoint = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	const XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	constexpr float fovY = 45.0f;
	const float aspectRatio = static_cast<float>(UT::GLOBALS::GWindowWidth) / UT::GLOBALS::GWindowHeight;
	constexpr float nearZ = 0.1f;
	constexpr float farZ = 100.0f;

	rotationAngle += 2.0f * dt;

	// Keep angle within 0 to 2*PI
	if (rotationAngle > XM_2PI) rotationAngle -= XM_2PI;

	const XMMATRIX rotation = XMMatrixRotationY(rotationAngle);

	// Compute world, view, and projection matrices (example)
	const XMMATRIX world = rotation;
	const XMMATRIX view = XMMatrixLookAtLH(eyePos, focusPoint, upDir);
	const XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ);

	//UpdateConstantBuffer(world, view, proj);

	m_pCubeRed->UpdateConstantBuffer(view, proj);
	m_pCubeGreen->UpdateConstantBuffer(view, proj);
	m_pCubeBlue->UpdateConstantBuffer(view, proj);
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

	// Bind constant buffer pointing to updated matrix
	//pCommandList->SetGraphicsRootConstantBufferView(0, m_listConstantBuffers[frameIndex]->GetGPUVirtualAddress());

	//pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	//pCommandList->IASetIndexBuffer(&m_IndexBufferView);
	

	// Draw
	//pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
	m_pCubeRed->Render();
	m_pCubeGreen->Render();
	m_pCubeBlue->Render();
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
		pSwapchain->GetBuffer(i, IID_PPV_ARGS(&m_listRTBuffers[i]));

		pDevice->CreateRenderTargetView(m_listRTBuffers[i], nullptr, rtvHandle);
		m_handlesRTV[i] = rtvHandle;
		rtvHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreateDSV()
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	IDXGISwapChain4* const pSwapchain = UT::D3D12::CORE::GetSwapchain();

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = UT::GLOBALS::GFramesInFlight;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // No shader visibility for DSV

	UT_ASSERT_HRESULT(pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pHeapDSV)));
	UT_NAME_D3D_OBJECT(m_pHeapDSV, "Descriptor Heap DSV");

	// Describe the heap properties: Default GPU memory
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// Describe the resource: Texture2D with depth format
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = UT::GLOBALS::GWindowWidth;
	depthStencilDesc.Height = UT::GLOBALS::GWindowHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // Typical depth/stencil format
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Clear value optimized for depth stencil
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++)
	{
		UT_CHECK_HRESULT(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_listDSBuffers[i])), "Depth Buffer Resource creation failed!");

		pDevice->CreateDepthStencilView(m_listDSBuffers[i], &dsvDesc, dsvHandle);
		m_handlesDSV[i] = dsvHandle;
		dsvHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
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

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreateCube()
{
	// Define Vertices (8 unique corners of a cube)
	const UT::D3D12::DAS::VertexPC vertices[] =
	{
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1, 0, 0, 1) }, // 0
		{ XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT4(0, 1, 0, 1) }, // 1
		{ XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT4(0, 0, 1, 1) }, // 2
		{ XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT4(1, 1, 0, 1) }, // 3
		{ XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT4(1, 0, 1, 1) }, // 4
		{ XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT4(0, 1, 1, 1) }, // 5
		{ XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT4(1, 1, 1, 1) }, // 6
		{ XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT4(0, 0, 0, 1) }, // 7
	};
	constexpr UINT64 vbSize = sizeof(vertices);

	//  Define Indices (12 triangles, 2 per cube face)
	const uint16_t indices[] =
	{
		// front face
		0, 1, 2, 0, 2, 3,
		// back face
		4, 6, 5, 4, 7, 6,
		// left face
		4, 5, 1, 4, 1, 0,
		// right face
		3, 2, 6, 3, 6, 7,
		// top face
		1, 5, 6, 1, 6, 2,
		// bottom face
		4, 0, 3, 4, 3, 7
	};
	constexpr UINT64 ibSize = sizeof(indices);


	// Upload data to Vertex buffer on GPU!
	ID3D12Resource* vbUploadBuffer;
	UT::D3D12::HELPER::CreateGPUBuffer(vbSize, &m_pVertexBuffer);
	UT::D3D12::HELPER::CreateUploadBuffer(vbSize, &vbUploadBuffer);
	UT::D3D12::HELPER::CopyDataFromUploadBufferToGPU(vbSize, vertices, vbUploadBuffer, m_pVertexBuffer);

	// Create and upload index buffer
	ID3D12Resource* ibUploadBuffer = nullptr;
	UT::D3D12::HELPER::CreateGPUBuffer(ibSize, &m_pIndexBuffer);
	UT::D3D12::HELPER::CreateUploadBuffer(ibSize, &ibUploadBuffer);
	UT::D3D12::HELPER::CopyDataFromUploadBufferToGPU(ibSize, indices, ibUploadBuffer, m_pIndexBuffer);

	// Setup Vertex & Index Buffer View
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPC);
	m_VertexBufferView.SizeInBytes = _countof(vertices) * sizeof(UT::D3D12::DAS::VertexPC);

	m_IndexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = static_cast<UINT>(ibSize);
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreatePSO()
{
	// Describe a single CBV (b0) root parameter ----
	std::array<D3D12_ROOT_PARAMETER, 1> rootParams = {};

	//D3D12_ROOT_PARAMETER rootParams[1] = {};
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // constant buffer
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // visible to VS
	rootParams[0].Descriptor.ShaderRegister = 0;  // b0 in HLSL
	rootParams[0].Descriptor.RegisterSpace = 0;   // register space 0

	// Create Root Signature
	UT::D3D12::HELPER::CreateRootSignatue(rootParams.size(), rootParams.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, &m_pRootSignature);

	// Compile Shaders
	D3D12_SHADER_BYTECODE vsByteCode = {};
	UT::D3D12::HELPER::CompileShader("Cube.hlsl", "VSMain", "vs_5_0", nullptr, vsByteCode);

	D3D12_SHADER_BYTECODE psByteCode = {};
	UT::D3D12::HELPER::CompileShader("Cube.hlsl", "PSMain", "ps_5_0", nullptr, psByteCode);

	// Input layput  Description
	D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPC, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPC, Color),    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	const D3D12_INPUT_LAYOUT_DESC inputLayout = { inputLayoutDesc, _countof(inputLayoutDesc) };

	// Create PSO!
	UT::D3D12::HELPER::CreatePSO(m_pRootSignature, vsByteCode, psByteCode, inputLayout, &m_pPSO);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreateConstantBuffer()
{
	m_pCubesData = new UT::D3D12::DAS::CubesCB();

	// Each constant buffer must be 256-byte aligned
	constexpr UINT64 cbSize = (sizeof(m_pCubesData) + 255) & ~255;

	// Map Constant buffer memory once for write access
	constexpr D3D12_RANGE readRange = { 0, 0 };	// We do not intent to read this resource on the CPU!

	for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++)
	{
		UT::D3D12::HELPER::CreateUploadBuffer(cbSize, &m_listConstantBuffers[i]);

	
		UT_CHECK_HRESULT(m_listConstantBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCBDataBegin[i])), "Failed to Map Constant Buffer!");

		// Zero initialize the mapped Constant Buffer!
		memset(m_pCBDataBegin[i], 0, cbSize);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::UpdateConstantBuffer(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj)
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	if (!m_pCBDataBegin[frameIndex] || !m_pCubesData)
		return;

	const XMMATRIX mvp = XMMatrixTranspose(world * view * proj);
	XMStoreFloat4x4(&m_pCubesData->WVP, mvp);

	memcpy(m_pCBDataBegin[frameIndex], m_pCubesData, sizeof(UT::D3D12::DAS::CubesCB));
}


