#include "UltimateEnginePCH.h"
#include "DXRenderer.h"

#include <ks.h>
#include <wrl/client.h>

#include "DirectXHelpers.h"
#include "BufferHelpers.h"
#include "CommonStates.h"
#include "ResourceUploadBatch.h"
#include "VertexTypes.h"
#include "D3DGlobals.h"
#include "DXRenderDevice.h"
#include "EngineHeader.h"
#include "stb_image.h"
#include "../../ThirdParty/DirectXTK12/Src/d3dx12.h"
#include "UI/UIRenderer.h"
#include "World/FreeCamera.h"

//---------------------------------------------------------------------------------------------------------------------
DXRenderer::DXRenderer() :
	m_pD3DGraphicsCommandList(nullptr),
	m_pPSO(nullptr),
	m_pRootSignature(nullptr),
	m_pVBuffer(nullptr),
	m_pIBuffer(nullptr),
	m_pDXRenderDevice(nullptr),
	m_pUIRenderer(nullptr),
	m_uiCurrentFrameIndex(0),
	m_handleFenceEvent(0),
	m_bIsCurrentFrameRunning(false),
	m_pConstantBuffer(nullptr),
	m_pCamera(nullptr),
	m_pConstBufferData(nullptr)
{
	m_pListFences.clear();
	m_pListFenceValue.clear();
	m_pListD3DCommandAllocator.clear();

	m_colorClear = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
}

//---------------------------------------------------------------------------------------------------------------------
DXRenderer::~DXRenderer()
{
	Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::Initialize(const GLFWwindow* pWindow)
{
	HRESULT Hr = 0;

	IDXGIFactory6* const pFactory = UT::D3D12::CORE::GetFactory();
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12CommandQueue* const pCmdQueue = UT::D3D12::CORE::GetCommandQueue();

	m_pDXRenderDevice = new DXRenderDevice();
	UT_CHECK_NULL(m_pDXRenderDevice, ": DXRenderDevice object");

	const HWND hwnd = glfwGetWin32Window(const_cast<GLFWwindow*>(pWindow));

	UT_CHECK_BOOL(m_pDXRenderDevice->Initialize(hwnd), "DXRenderDevice Initialization failed!");
	UT_CHECK_BOOL(CreateCommandAllocator(), "D3D Command Allocator creation failed!");
	UT_CHECK_BOOL(CreateCommandList(), "D3D Command List creation failed!");
	UT_CHECK_BOOL(CreateFences(), "D3D Fence creation failed!");

	m_pUIRenderer = new UIRenderer();
	UT_CHECK_BOOL(m_pUIRenderer->Initialize(pWindow, m_pDXRenderDevice));

	//---- Copy Constant Buffer data onto ID3D12Resource
	// Transpose matrices before storing them in the buffer, as HLSL expects row - major format :
	m_pCamera = new FreeCamera();
	m_pCamera->SetPosition(0.0f, 2.0f, 10.0f);
	m_pCamera->SetRotation(0.0f, 0.0f, 0.0f);

  //XMMatrixTranspose(FreeCamera::getInstance().m_matProjection);

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = (sizeof(UT::D3D12::DAS::ConstantBuffer) + 255) & ~255;		// 256-bit alignment!
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Hr = pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantBuffer));

	UT_CHECK_HRESULT(Hr, "CreateCommittedResource", "Constant Buffer");
	UT_NAME_D3D_OBJECT(m_pConstantBuffer, "Constant Buffer");

	//m_pConstBufferData = new UT::D3D12::DAS::ConstantBuffer();
	Hr = m_pConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstBufferData));
	if(SUCCEEDED(Hr))
	{

		XMMATRIX worldMatrix = XMMatrixIdentity(); // No transformation (for testing)
		XMMATRIX viewMatrix = XMMatrixLookAtLH(
			XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f), // Camera position
			XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),  // Look-at point
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)   // Up vector
		);
		XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
			XM_PIDIV4, // 45-degree field of view
			UT::Globals::GWindowWidth / UT::Globals::GWindowHeight,
			0.1f,      // Near plane
			1000.0f    // Far plane
		);

		//m_pConstBufferData->matWorld = XMMatrixTranspose(worldMatrix);
		//m_pConstBufferData->matView = XMMatrixTranspose(viewMatrix); //XMMatrixTranspose(m_pCamera->GetViewMatrix());
		//m_pConstBufferData->matProjection = XMMatrixTranspose(projectionMatrix); //XMMatrixTranspose(m_pCamera->GetProjectionMatrix(UT::Globals::GWindowWidth, UT::Globals::GWindowHeight, 0.1f, 1000.0f));
		m_pConstBufferData->matWVP = XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);
		m_pConstBufferData->ambientColor = XMFLOAT4(1, 0, 0, 1);

		//m_pConstantBuffer->Unmap(0, nullptr);
	}
	else
	{
		LOG_CRITICAL("Failed to map constant buffer!");
	}
	
	

	//---- Load Image as texture
	UT::D3D12::HelperFunc::CreateTexture2D("Assets\\Textures\\Debug_Purple.png", &m_pImageTexture);

	//---- TRIANGLE RENDERING START

	// ---- 3 ROOT PARAMETERS ----
	std::array<D3D12_ROOT_PARAMETER1, 2> rootParams;

	// -- 1. Root constants --
	D3D12_ROOT_CONSTANTS rootConstants = {};
	rootConstants.Num32BitValues = 4;	// RGB + Delta-Time
	rootConstants.ShaderRegister = 0;
	rootConstants.RegisterSpace = 0;
	
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParams[0].Constants = rootConstants;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// -- 2. Descriptor Table --
	std::array<D3D12_DESCRIPTOR_RANGE1, 2> descriptorRanges;

	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// Shader Resource View range
	descriptorRanges[0].NumDescriptors = 1;								// Single descriptor for the texture
	descriptorRanges[0].BaseShaderRegister = 0;							// t0 in the shader
	descriptorRanges[0].RegisterSpace = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRanges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptorRanges[1].NumDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = 1;
	descriptorRanges[1].RegisterSpace = 0;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRanges[1].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

	D3D12_ROOT_DESCRIPTOR_TABLE1 descriptorTable = {};
	descriptorTable.NumDescriptorRanges = static_cast<uint32_t>(descriptorRanges.size());
	descriptorTable.pDescriptorRanges = descriptorRanges.data();

	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].DescriptorTable = descriptorTable;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Static sampler
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.ShaderRegister = 0; // Matches the shader's s0 register
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = static_cast<uint32_t>(rootParams.size());
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pParameters = rootParams.data();
	rootSignatureDesc.pStaticSamplers = &samplerDesc;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignDesc = {};
	rootSignDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	rootSignDesc.Desc_1_1 = rootSignatureDesc;

	ID3DBlob* pSignature;
	ID3DBlob* pError;
	
	Hr = D3DX12SerializeVersionedRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &pSignature, &pError);

	if(FAILED(Hr))
	{
		if(pError)
		{
			const char* ErrorMsg = static_cast<const char*>(pError->GetBufferPointer());
			OutputDebugStringA(ErrorMsg);
		}
	}

	Hr = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
	UT_CHECK_HRESULT(Hr, "CreateRootSignature", "D3D_ROOT_SIGNATURE_VERSION_1_1");
	UT_NAME_D3D_OBJECT(m_pRootSignature, "Root Signature");

	// 2. Create Vertex & Fragment shaders
	ID3DBlob* vertexShader;
	UT::D3D12::HelperFunc::CreateVertexShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicVS.hlsl", &vertexShader);

	ID3DBlob* pixelShader;
	UT::D3D12::HelperFunc::CreateFragmentShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicFS.hlsl", &pixelShader);

	D3D12_SHADER_BYTECODE vsByteCode = {};
	vsByteCode.BytecodeLength = vertexShader->GetBufferSize();
	vsByteCode.pShaderBytecode = vertexShader->GetBufferPointer();

	D3D12_SHADER_BYTECODE psByteCode = {};
	psByteCode.BytecodeLength = pixelShader->GetBufferSize();
	psByteCode.pShaderBytecode = pixelShader->GetBufferPointer();

	//D3D12_SHADER_BYTECODE vsByteCode = {};
	//UT::HelperFunc::CreateVertexShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicVS.hlsl", &vsByteCode);
	//
	//D3D12_SHADER_BYTECODE fsByteCode = {};
	//UT::HelperFunc::CreateFragmentShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicFS.hlsl", &fsByteCode);

	// 3. Create Input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	// fill out an input layout description structure
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	// create depth-stencil state...
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	// Stencil states won't matter now as it is disabled. 
	D3D12_DEPTH_STENCILOP_DESC depthStencilOpDesc = {};
	depthStencilOpDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilOpDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilOpDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilOpDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	depthStencilDesc.FrontFace = depthStencilOpDesc;
	depthStencilDesc.BackFace = depthStencilOpDesc;


	//UT::HelperFunc::CreateVertexInputLayoutDesc(inputLayoutDesc);

	// 4. Create PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc;										// structure describing our input layout.
	psoDesc.pRootSignature = m_pRootSignature;									// input data that this pso needs
	psoDesc.VS = vsByteCode;													// struct describing where to find vs bytecode & how large it is.
	psoDesc.PS = psByteCode;													// struct describing where to find fs bytecode & how large it is.
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		// topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;							// format of render target
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;												// same as swapchain
	psoDesc.SampleMask = 0xffffffff;											// multi-sampling, here we are choosing point sampling.
	psoDesc.RasterizerState = DirectX::CommonStates::CullCounterClockwise;		// default rasterizer state
	psoDesc.BlendState = DirectX::CommonStates::Opaque;							// default blend state
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.NumRenderTargets = 1;												// we are binding only one render target


	Hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO));
	UT_ASSERT_HRESULT(Hr, "CreateGraphicsPipelineState Failed!!!");
	UT_NAME_D3D_OBJECT(m_pPSO, "Graphics Pipeline");

	// 5. Create Vertex Buffer & transfer the data to GPU!
	DirectX::ResourceUploadBatch vbResourceUpload(pDevice);
	vbResourceUpload.Begin();

	// vertex data...
	std::array<UT::D3D12::DAS::VertexPT, 8> vertices;

	// first quad
	vertices[0] = { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0,0) };
	vertices[1] = { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1,0) };
	vertices[2] = { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1,1) };
	vertices[3] = { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0,1) };
	vertices[4] = { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0,0) };
	vertices[5] = { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1,0) };
	vertices[6] = { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1,1) };
	vertices[7] = { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0,1) };

	Hr = DirectX::CreateStaticBuffer(pDevice, vbResourceUpload, vertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pVBuffer);
	UT_CHECK_HRESULT(Hr, "Vertex Buffer", "D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER");
	UT_NAME_D3D_OBJECT(m_pVBuffer, "Vertex Buffer");

	auto vbUploadResourcesFinished = vbResourceUpload.End(pCmdQueue);
	vbUploadResourcesFinished.wait();

	// 6. Create Indices data & transfer the data to GPU!
	DirectX::ResourceUploadBatch ibResourceUpload(pDevice);
	ibResourceUpload.Begin();

	// index data...
	std::array<uint16_t, 36> indices;
	
	indices[0] = 0;		indices[1] = 1;		indices[2] = 2;		indices[3] = 0,		indices[4] = 2;		indices[5] = 3;
	indices[6] = 5;		indices[7] = 4;		indices[8] = 7;		indices[9] = 5,		indices[10] = 7;	indices[11] = 6;
	indices[12] = 4;	indices[13] = 0;	indices[14] = 3;	indices[15] = 4,	indices[16] = 3;	indices[17] = 7;
	indices[18] = 1;	indices[19] = 5;	indices[20] = 6;	indices[21] = 1,	indices[22] = 6;	indices[23] = 2;
	indices[24] = 3;	indices[25] = 2;	indices[26] = 6;	indices[27] = 3,	indices[28] = 6;	indices[29] = 7;
	indices[30] = 2;	indices[31] = 5;	indices[32] = 1;	indices[33] = 4,	indices[34] = 1;	indices[35] = 0;

	DirectX::CreateStaticBuffer(pDevice, ibResourceUpload, indices, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pIBuffer);
	UT_CHECK_HRESULT(Hr, "Index Buffer Created", "D3D12_RESOURCE_STATE_INDEX_BUFFER");
	UT_NAME_D3D_OBJECT(m_pIBuffer, "Index Buffer");

	auto ibUploadResourcesFinished = ibResourceUpload.End(pCmdQueue);
	ibUploadResourcesFinished.wait();

	// // Create a default heap. This will be created on the GPU & only GPU will have access to this.
	// m_pDXRenderDevice->CreateBuffer(D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, vBufferSize, D3D12_RESOURCE_STATE_COPY_DEST, L"VB Resource Heap", m_pVBuffer);
	// 
	// // To get data into this heap, we will have to upload the data using an Upload heap!
	// m_pDXRenderDevice->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, vBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, L"VB Upload Heap", m_pUploadBuffer);
	// 
	// // store vertex buffer in upload heap
	// D3D12_SUBRESOURCE_DATA vertexData = {};
	// vertexData.pData = reinterpret_cast<BYTE*>(vertices);
	// vertexData.RowPitch = vBufferSize;
	// vertexData.SlicePitch = vBufferSize;
	// 
	// // create command with command list to copy data from the upload heap to default heap!
	// UpdateSubresources(m_pD3DGraphicsCommandList.Get(), m_pVBuffer.Get(), m_pUploadBuffer.Get(), 0, 0, 1, &vertexData);
	// 
	// // transition the vertex buffer data from copy destination state to vertex buffer state
	// CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pVBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	// m_pD3DGraphicsCommandList->ResourceBarrier(1, &transitionBarrier);
	// 
	// // Now we execute the command list to upload the triangle data
	// m_pD3DGraphicsCommandList->Close();
	// 
	// // create an array of command lists (only one command list here)
	// const std::vector<ComPtr<ID3D12CommandList>> vecCommandList = { m_pD3DGraphicsCommandList };
	// 
	// // execute the array of command lists
	// m_pDXRenderDevice->ExecuteCommandLists(vecCommandList);
	// 
	// const uint32_t uiCurrentFrameIndex = m_pDXRenderDevice->GetCurrentBackbufferIndex();
	// m_pDXRenderDevice->SignalFence(m_pListFences.at(uiCurrentFrameIndex), m_pListFenceValue.at(uiCurrentFrameIndex));

	// create vertex buffer view for the quad
	m_VBView.BufferLocation = m_pVBuffer->GetGPUVirtualAddress();
	m_VBView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPT);
	m_VBView.SizeInBytes = vertices.size() * sizeof(UT::D3D12::DAS::VertexPT);

	// create index buffer view for the quad
	m_IBView.BufferLocation = m_pIBuffer->GetGPUVirtualAddress();
	m_IBView.Format = DXGI_FORMAT_R16_UINT;
	m_IBView.SizeInBytes = indices.size() * sizeof(uint16_t);

	// Fill out the Viewport
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = UT::Globals::GWindowWidth;
	m_Viewport.Height = UT::Globals::GWindowHeight;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = UT::Globals::GWindowWidth;
	m_ScissorRect.bottom = UT::Globals::GWindowHeight;

	// Create the SRV for the texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_pDXRenderDevice->GetCPUDescriptorHandleGlobal();
	pDevice->CreateShaderResourceView(m_pImageTexture, &srvDesc, srvHandle);

	// Create CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (sizeof(UT::D3D12::DAS::ConstantBuffer) + 255) & ~255;

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_pDXRenderDevice->GetCPUDescriptorHandleGlobal();
	cbvHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

	//---- TRIANGLE RENDERING END

	SAFE_RELEASE(pError);
	SAFE_RELEASE(pSignature);

	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::Update(double dt)
{
	// Update the World matrix (rotate the cube over time)
	static float angle = 0.0f;								// Rotation angle
	angle += dt * XM_PI / 4;								// Increment angle (speed = pi/4 radians/sec)
	XMMATRIX worldMatrix = XMMatrixRotationY(angle);		// Rotate the cube around

	// Update the View matrix (camera setup)
	XMMATRIX viewMatrix = m_pCamera->GetViewMatrix();
	XMMATRIX projMatrix = m_pCamera->GetProjectionMatrix(UT::Globals::GWindowWidth, UT::Globals::GWindowHeight, 0.1f, 1000.0f);

	HRESULT Hr = m_pConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstBufferData));
	if (SUCCEEDED(Hr))
	{
		XMMATRIX worldMatrix = XMMatrixIdentity(); // No transformation (for testing)
		XMMATRIX viewMatrix = XMMatrixLookAtLH(
			XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f), // Camera position
			XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),  // Look-at point
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)   // Up vector
		);
		XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
			XM_PIDIV4, // 45-degree field of view
			UT::Globals::GWindowWidth / UT::Globals::GWindowHeight,
			0.1f,      // Near plane
			1000.0f    // Far plane
		);

		//m_pConstBufferData->matWorld = XMMatrixTranspose(worldMatrix);
		//m_pConstBufferData->matView = XMMatrixTranspose(viewMatrix); //XMMatrixTranspose(m_pCamera->GetViewMatrix());
		//m_pConstBufferData->matProjection = XMMatrixTranspose(projectionMatrix); //XMMatrixTranspose(m_pCamera->GetProjectionMatrix(UT::Globals::GWindowWidth, UT::Globals::GWindowHeight, 0.1f, 1000.0f));
		m_pConstBufferData->matWVP = XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);
		m_pConstBufferData->ambientColor = XMFLOAT4(1, 0, 0, 1);

		//m_pConstBufferData->matWorld = XMMatrixTranspose(XMMatrixIdentity()); //XMMatrixTranspose(worldMatrix);
		//m_pConstBufferData->matView = XMMatrixTranspose(XMMatrixIdentity()); //XMMatrixTranspose(viewMatrix);
		//m_pConstBufferData->matProjection = XMMatrixTranspose(XMMatrixIdentity()); //XMMatrixTranspose(projMatrix);
		//m_pConstBufferData->ambientColor = XMFLOAT4(1, 0, 0, 1);
	}
	else
	{
		LOG_CRITICAL("Failed to map constant buffer!");
	}
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::CreateRootConstants()
{
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::CreateRootDescriptorCBV()
{
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::CreateRootDescriptorSRV()
{
}


//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::Render()
{
	const uint32_t currRenderTargetID = BeginFrame();

	RecordCommands(currRenderTargetID);
	EndFrame(currRenderTargetID);
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::Cleanup()
{
	const uint32_t currRenderTargetIndex = WaitForPreviousFrame();
	m_pDXRenderDevice->SignalFence(m_pListFences.at(currRenderTargetIndex), m_pListFenceValue.at(currRenderTargetIndex));

	SAFE_DELETE(m_pCamera);
	SAFE_DELETE(m_pUIRenderer);
	SAFE_RELEASE(m_pImageTexture);
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pPSO);
	SAFE_RELEASE(m_pRootSignature);
	SAFE_RELEASE(m_pVBuffer);
	SAFE_RELEASE(m_pIBuffer);
	
	for (uint16_t i = 0; i < UT::Globals::GBackbufferCount; ++i)
	{
		SAFE_RELEASE(m_pListFences.at(i));
		SAFE_RELEASE(m_pListD3DCommandAllocator.at(i));
	}

	m_pListFences.clear();
	m_pListFenceValue.clear();
	m_pListD3DCommandAllocator.clear();

	SAFE_DELETE(m_pDXRenderDevice);
	SAFE_RELEASE(m_pD3DGraphicsCommandList);
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::HandleInput(const GLFWwindow* pWindow, UT::Globals::InputAction action, float mousePosX, float mousePosY, bool isMouseClicked) const
{

}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::CleanupOnWindowResize()
{
	const uint32_t currRenderTargetIndex = WaitForPreviousFrame();
	m_pDXRenderDevice->SignalFence(m_pListFences.at(currRenderTargetIndex), m_pListFenceValue.at(currRenderTargetIndex));

	m_pDXRenderDevice->CleanupOnWindowResize();
	m_pUIRenderer->CleanupOnWindowResize();
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight)
{
	m_pDXRenderDevice->RecreateOnWindowResize(newWidth, newHeight);
	m_pUIRenderer->RecreateOnWindowResize(newWidth, newHeight);
}

//---------------------------------------------------------------------------------------------------------------------
uint32_t DXRenderer::BeginFrame()
{
	HRESULT Hr = 0;

	// We have to wait for the GPU to finish with the command allocator before we reset it.
	const uint32_t currRenderTargetIndex = WaitForPreviousFrame();

	// we can only reset an allocator once the GPU is done with it.
	// resetting an allocator frees the memory that the command list was stored in.
	ResetCommandAllocator(currRenderTargetIndex);

	m_pUIRenderer->Begin();

	return currRenderTargetIndex;
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::EndFrame(uint32_t currFrameIndex)
{
	HRESULT Hr = 0;

	// create an array of command lists (only one command list here)
	const std::vector<ID3D12CommandList*> vecCommandList = { m_pD3DGraphicsCommandList };

	// execute the array of command lists
	m_pDXRenderDevice->ExecuteCommandLists(vecCommandList);

	// this command goes in at the end of our command queue. we will know when our command queue has finished because
	// the fence value will be set to "fenceValue" from the GPU since the command queue is being executed on the GPU!
	m_pDXRenderDevice->SignalFence(m_pListFences.at(currFrameIndex), m_pListFenceValue.at(currFrameIndex));
	
	// Present the current backbuffer
	m_pDXRenderDevice->Present();
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::DrawCommands()
{
	m_pD3DGraphicsCommandList->SetGraphicsRootSignature(m_pRootSignature);

	// Set root constant value!
	float gameDelta = static_cast<float>(UT::Globals::GDeltaTime);
	float rootConstantsData[4] = { 1.0f, 1.0f, 0.0f, gameDelta };
	m_pD3DGraphicsCommandList->SetGraphicsRoot32BitConstants(0, 4, rootConstantsData, 0);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_pDXRenderDevice->GetDescriptorHeapGlobal() };
	m_pD3DGraphicsCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pDXRenderDevice->GetGPUDescriptorHandleGlobal();
	m_pD3DGraphicsCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle);

	m_pD3DGraphicsCommandList->RSSetViewports(1, &m_Viewport);
	m_pD3DGraphicsCommandList->RSSetScissorRects(1, &m_ScissorRect);
	m_pD3DGraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3DGraphicsCommandList->IASetVertexBuffers(0, 1, &m_VBView);
	m_pD3DGraphicsCommandList->IASetIndexBuffer(& m_IBView);
	m_pD3DGraphicsCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	m_pUIRenderer->Render(m_pDXRenderDevice, m_pD3DGraphicsCommandList, m_colorClear);
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::RecordCommands(uint32_t currFrameIndex)
{
	HRESULT Hr = 0;

	// reset the command list. By doing so, we are putting it into a recording state, so we can start recording commands
	// into the command allocator. The command allocator that we reference here may have multiple command lists associated
	// with it, but only one can be recording at any time. Make sure that any other command list associated to this command
	// allocator are in the closed state i.e. not recording.
	ResetCommandList(currFrameIndex);

	// Get cuurent Render Target
	ID3D12Resource* pRenderTarget = m_pDXRenderDevice->GetRenderTarget(currFrameIndex);
	UT_ASSERT_NULL(pRenderTarget, "Current RenderTarget is NULL!");

	//-- Start recording commands into command list
	D3D12_RESOURCE_BARRIER rtBarrier = {};
	rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rtBarrier.Transition.pResource = pRenderTarget;
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pD3DGraphicsCommandList->ResourceBarrier(1, &rtBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = m_pDXRenderDevice->GetCPUDescriptorHandleRTV();
	rtHandle.ptr += currFrameIndex * m_pDXRenderDevice->GetRTVDescriptorSize();

	const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pDXRenderDevice->GetCPUDescriptorHandleDSV();

	m_pD3DGraphicsCommandList->OMSetRenderTargets(1, &rtHandle, false, &dsvHandle);

	const float clearColor[] = { m_colorClear.x, m_colorClear.y, m_colorClear.z, m_colorClear.w };
	m_pD3DGraphicsCommandList->ClearRenderTargetView(rtHandle, clearColor, 0, nullptr);
	m_pD3DGraphicsCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	DrawCommands();

	D3D12_RESOURCE_BARRIER presentBarrier = {};
	presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentBarrier.Transition.pResource = pRenderTarget;
	presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pD3DGraphicsCommandList->ResourceBarrier(1, &presentBarrier);

	UT_ASSERT_HRESULT(m_pD3DGraphicsCommandList->Close(), "Failed to close the Command List!");

	m_pUIRenderer->End(m_pD3DGraphicsCommandList);
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::CreateCommandAllocator()
{
	for (uint16_t i = 0; i < UT::Globals::GBackbufferCount; ++i)
	{
		ID3D12CommandAllocator* pCmdAllocator;
		m_pDXRenderDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, &pCmdAllocator);

		//HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCmdAllocator));
		//UT_CHECK_HRESULT(Hr, "Command Allocator creation failed!");

		m_pListD3DCommandAllocator.emplace_back(pCmdAllocator);
		UT_NAME_D3D_OBJECT_INDEXED(m_pListD3DCommandAllocator[i], i, "Command Allocator");
	}

	LOG_INFO("Command Allocator created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::CreateCommandList()
{
	// create the command list with the first allocator
	m_pDXRenderDevice->CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, m_pListD3DCommandAllocator.at(0), &m_pD3DGraphicsCommandList);

	// Command lists are created in "Recording" state. We do not want to record the command list yet, so we close it. 
	HRESULT Hr = m_pD3DGraphicsCommandList->Close();
	UT_ASSERT_HRESULT(Hr, "Command List Close() failed!");

	UT_NAME_D3D_OBJECT(m_pD3DGraphicsCommandList, "Graphics Command-List");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::ResetCommandAllocator(uint32_t renderTargetID) const
{
	HRESULT Hr = m_pListD3DCommandAllocator.at(renderTargetID)->Reset();
	UT_ASSERT_HRESULT(Hr, "Command Allocator Reset FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
void DXRenderer::ResetCommandList(uint32_t renderTargetID) const
{
	HRESULT	Hr = 0;
	ID3D12CommandAllocator* pCmdAllocator = m_pListD3DCommandAllocator.at(renderTargetID);

	UT_ASSERT_NULL(pCmdAllocator, ": Command Allocator");

	Hr = m_pD3DGraphicsCommandList->Reset(pCmdAllocator, m_pPSO);
	UT_ASSERT_HRESULT(Hr, "CommandList Reset FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::CreateFences()
{
	m_pListFences.reserve(UT::Globals::GBackbufferCount);
	m_pListFenceValue.reserve(UT::Globals::GBackbufferCount);

	// Create the fences...
	for (uint16_t i = 0; i < UT::Globals::GBackbufferCount; ++i)
	{
		ID3D12Fence* pFence;
		uint64_t uiFenceValue = 0;

		//HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
		//UT_CHECK_HRESULT(Hr, "Failed to create Fence!");

		m_pDXRenderDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, &pFence);

		m_pListFences.emplace_back(pFence);
		m_pListFenceValue.emplace_back(uiFenceValue);	// Initialize with zero.

		UT_NAME_D3D_OBJECT_INDEXED(pFence, i, "Fence");
	}

	// create a handle to a fence event...
	m_handleFenceEvent = CreateEvent(nullptr, false, false, nullptr);
	UT_CHECK_NULL(m_handleFenceEvent, "Fence event creation failed!");

	LOG_INFO("Fences & Fence event created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
uint32_t DXRenderer::WaitForPreviousFrame()
{
	HRESULT Hr = 0;

	// swap the current rtv buffer index, so we draw on the correct buffer
	const uint32_t uiCurrentFrameIndex = m_pDXRenderDevice->GetCurrentBackbufferIndex();

	// Acquire pointers for code readibility!
	ID3D12Fence*	pFence			= m_pListFences.at(uiCurrentFrameIndex);
	uint64_t	uiFenceValue		= m_pListFenceValue.at(uiCurrentFrameIndex);

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (pFence->GetCompletedValue() < uiFenceValue)
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		Hr = pFence->SetEventOnCompletion(uiFenceValue, m_handleFenceEvent);
		UT_ASSERT_HRESULT(Hr, "");

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(m_handleFenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	m_pListFenceValue[uiCurrentFrameIndex]++;

	return uiCurrentFrameIndex;
}
