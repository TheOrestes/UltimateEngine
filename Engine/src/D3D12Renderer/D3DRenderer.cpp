#include "UltimateEnginePCH.h"
#include "D3DRenderer.h"
#include "D3DGlobals.h"
#include "World/Scene.h"
#include "World/Camera.h"


//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::D3DRenderer()
{
	
}

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::~D3DRenderer()
{
	
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::Cleanup()
{
	LOG_INFO("D3DRenderer::Cleanup() called");
	LOG_INFO("m_pPSO        = {0}", (void*)m_pPSO);
	LOG_INFO("m_pRootSignature = {0}", (void*)m_pRootSignature);
	LOG_INFO("m_pHeapSampler   = {0}", (void*)m_pHeapSampler);

	Scene::getInstance().Cleanup();

	SAFE_RELEASE(m_pHeapSampler);
	SAFE_RELEASE(m_pPSO);
	SAFE_RELEASE(m_pRootSignature);
	SAFE_RELEASE(m_pTransformBuffer);
	SAFE_RELEASE(m_pVSBlob);
	SAFE_RELEASE(m_pVSCode);
	SAFE_RELEASE(m_pPSBlob);
	SAFE_RELEASE(m_pPSCode);

	// RT buffers & DS buffers — GetBuffer() AddRefs each one
	for (uint16_t i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
	{
		if (m_listRTBuffers[i])
		{
			m_listRTBuffers[i]->AddRef();
			ULONG ref = m_listRTBuffers[i]->Release();
			LOG_INFO("RTBuffer[{0}] refcount = {1}", i, ref);
		}

		if (m_listDSBuffers[i])
		{
			m_listDSBuffers[i]->AddRef();
			ULONG ref = m_listDSBuffers[i]->Release();
			LOG_INFO("DSBuffer[{0}] refcount = {1}", i, ref);
		}
	}

	// Now release
	for (uint16_t i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
		SAFE_RELEASE(m_listRTBuffers[i]);

	for (uint16_t i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
		SAFE_RELEASE(m_listDSBuffers[i]);

	// Descriptor heaps
	SAFE_RELEASE(m_pHeapRTV);
	SAFE_RELEASE(m_pHeapDSV);

	
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::Initialize()
{
	m_pRootSignature	= nullptr;
	m_pPSO				= nullptr;
	m_pHeapSampler		= nullptr;
	m_pTransformBuffer	= nullptr;
	m_pPSCode			= nullptr;
	m_pVSCode			= nullptr;
	m_pVSBlob			= nullptr;
	m_pPSBlob			= nullptr;

	UT_CHECK_BOOL(CreateRTV());
	UT_CHECK_BOOL(CreateDSV());
	UT_CHECK_BOOL(CreatePSO());

	UT_CHECK_BOOL(Scene::getInstance().Initialize());

	//UT_CHECK_BOOL(CreateTriangle());
	//UT_CHECK_BOOL(CreateCube());
	//UT_CHECK_BOOL(CreateConstantBuffer());

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
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
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
	Scene::getInstance().Update(dt);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::Render()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	// Set heap ONCE here — objects never call SetDescriptorHeaps again
	ID3D12DescriptorHeap* heaps[] = 
	{
		UT::D3D12::CORE::GetGlobalDescriptorHeap(),		// CBV-SRV-UAV
		m_pHeapSampler									// Sampler
	};

	pCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// Set pipeline
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	pCommandList->SetPipelineState(m_pPSO);

	// Draw
	Scene::getInstance().Render();
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::OnKeyPressed(UT::GLOBALS::InputAction action)
{
	Camera::GetInstance().OnKeyPressed(action);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::OnKeyReleased(UT::GLOBALS::InputAction action)
{
	Camera::GetInstance().OnKeyReleased(action);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::OnMouseMove(float x, float y, bool bMouseClicked)
{
	Camera::GetInstance().OnMouseMove(x, y, bMouseClicked);
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
		UT_NAME_D3D_OBJECT_INDEXED(m_listRTBuffers[i], i, "BackBuffer");

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
		UT_NAME_D3D_OBJECT_INDEXED(m_listDSBuffers[i], i, "DepthBuffer");

		pDevice->CreateDepthStencilView(m_listDSBuffers[i], &dsvDesc, dsvHandle);
		m_handlesDSV[i] = dsvHandle;
		dsvHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
/*
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
	SAFE_RELEASE(vbUploadBuffer);

	// Setup Vertex Buffer View
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPC);
	m_VertexBufferView.SizeInBytes = _countof(vertices) * sizeof(UT::D3D12::DAS::VertexPC);

	return true;
}
*/

//-------------------------------------------------------------------------------------------------------------------
/*
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
	SAFE_RELEASE(vbUploadBuffer);

	// Create and upload index buffer
	ID3D12Resource* ibUploadBuffer = nullptr;
	UT::D3D12::HELPER::CreateGPUBuffer(ibSize, &m_pIndexBuffer);
	UT::D3D12::HELPER::CreateUploadBuffer(ibSize, &ibUploadBuffer);
	UT::D3D12::HELPER::CopyDataFromUploadBufferToGPU(ibSize, indices, ibUploadBuffer, m_pIndexBuffer);
	SAFE_RELEASE(ibUploadBuffer);

	// Setup Vertex & Index Buffer View
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPC);
	m_VertexBufferView.SizeInBytes = _countof(vertices) * sizeof(UT::D3D12::DAS::VertexPC);

	m_IndexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = static_cast<UINT>(ibSize);
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

	return true;
}
*/

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::CreatePSO()
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

	// Bindless Root Signature
	D3D12_ROOT_PARAMETER rootParam			= {};
	rootParam.ParameterType					= D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam.Constants.ShaderRegister		= 0;
	rootParam.Constants.RegisterSpace		= 0;
	rootParam.Constants.Num32BitValues		= sizeof(UT::D3D12::DAS::DrawConstants) / 4;
	rootParam.ShaderVisibility				= D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC samplerDesc	= {};
	samplerDesc.Filter						= D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU					= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV					= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW					= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias					= 0;
	samplerDesc.MaxAnisotropy				= 0;
	samplerDesc.ComparisonFunc				= D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.BorderColor					= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD						= 0;
	samplerDesc.MaxLOD						= D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister				= 0; // s0
	samplerDesc.RegisterSpace				= 0;
	samplerDesc.ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
												D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED |
												D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Create Root Signature
	UT::D3D12::HELPER::CreateRootSignatue(1, &rootParam, 1, &samplerDesc, flags, &m_pRootSignature);

	// Create Sampler Heap — required by SAMPLER_HEAP_DIRECTLY_INDEXED flag
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc	= {};
	samplerHeapDesc.NumDescriptors				= 8;
	samplerHeapDesc.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	samplerHeapDesc.NodeMask					= 0;

	UT_ASSERT_HRESULT(pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pHeapSampler)));
	UT_NAME_D3D_OBJECT(m_pHeapSampler, "Descriptor Heap Sampler");

	// Compile Shaders
	D3D12_SHADER_BYTECODE vsByteCode = {};
	UT::D3D12::HELPER::CompileShader("CubeTextured.hlsl", "VSMain", "vs_6_6", nullptr, vsByteCode, &m_pVSCode, &m_pVSBlob);

	D3D12_SHADER_BYTECODE psByteCode = {};
	UT::D3D12::HELPER::CompileShader("CubeTextured.hlsl", "PSMain", "ps_6_6", nullptr, psByteCode, &m_pPSCode, &m_pPSBlob);

	// Input layput  Description
	D3D12_INPUT_ELEMENT_DESC inputLayoutDesc[] =

	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPNBT, Position),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPNBT, Normal),    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UT::D3D12::DAS::VertexPNBT, BiNormal),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(UT::D3D12::DAS::VertexPNBT, TexCoord),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	const D3D12_INPUT_LAYOUT_DESC inputLayout = { inputLayoutDesc, _countof(inputLayoutDesc) };

	// Create PSO!
	UT::D3D12::HELPER::CreatePSO(m_pRootSignature, vsByteCode, psByteCode, inputLayout, &m_pPSO);

	// Create Global Transform buffer!
	UT::D3D12::DAS::TransformData* pTransformData = nullptr;
	UT::D3D12::HELPER::CreateTransformBuffer(256, &m_pTransformBuffer, &pTransformData);
	UT::D3D12::DAS::SetGlobalTransformDataPtr(pTransformData);

	return true;
}



