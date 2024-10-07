#include "UltimateEnginePCH.h"
#include "DXRenderer.h"

#include <wrl/client.h>

#include "DirectXHelpers.h"
#include "BufferHelpers.h"
#include "CommonStates.h"
#include "ResourceUploadBatch.h"
#include "VertexTypes.h"

#include "DXRenderDevice.h"
#include "EngineHeader.h"
#include "UI/UIRenderer.h"

//---------------------------------------------------------------------------------------------------------------------
DXRenderer::DXRenderer()
{
	m_pListFences.clear();
	m_pListFenceValue.clear();

	m_uiCurrentFrameIndex = 0;
	m_handleFenceEvent = 0;
	m_bIsCurrentFrameRunning = false;

	m_pDXRenderDevice = nullptr;
	m_pUIRenderer = nullptr;

	m_colorClear = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

//---------------------------------------------------------------------------------------------------------------------
DXRenderer::~DXRenderer()
{
	Cleanup();
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::Initialize(const GLFWwindow* pWindow, ComPtr<IDXGIFactory6> pFactory)
{
	m_pDXRenderDevice = new DXRenderDevice();
	UT_CHECK_NULL(m_pDXRenderDevice, ": DXRenderDevice object");

	const HWND hwnd = glfwGetWin32Window(const_cast<GLFWwindow*>(pWindow));

	UT_CHECK_BOOL(m_pDXRenderDevice->Initialize(hwnd, pFactory), "DXRenderDevice Initialization failed!");
	UT_CHECK_BOOL(CreateCommandAllocator(), "D3D Command Allocator creation failed!");
	UT_CHECK_BOOL(CreateCommandList(), "D3D Command List creation failed!");
	UT_CHECK_BOOL(CreateFences(), "D3D Fence creation failed!");

	m_pUIRenderer = new UIRenderer();
	UT_CHECK_BOOL(m_pUIRenderer->Initialize(pWindow, m_pDXRenderDevice));

	//---- TRIANGLE RENDERING START

	// 1. Create Root signature
	D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
	rootDesc.NumParameters = 0;
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootDesc.NumStaticSamplers = 0;
	rootDesc.pParameters = nullptr;
	rootDesc.pStaticSamplers = nullptr;

	DirectX::CreateRootSignature(m_pDXRenderDevice->GetD3DDevice().Get(), &rootDesc, &m_pRootSignature);

	//m_pDXRenderDevice->CreateRootSignature(rootDesc, m_pRootSignature);

	// 2. Create Vertex & Fragment shaders
	ComPtr<ID3DBlob> vertexShader = UT::HelperFunc::CreateVertexShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicVS.hlsl");
	ComPtr<ID3DBlob> pixelShader = UT::HelperFunc::CreateFragmentShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicFS.hlsl");

	D3D12_SHADER_BYTECODE vsByteCode = {};
	vsByteCode.BytecodeLength = vertexShader.Get()->GetBufferSize();
	vsByteCode.pShaderBytecode = vertexShader.Get()->GetBufferPointer();

	D3D12_SHADER_BYTECODE psByteCode = {};
	psByteCode.BytecodeLength = pixelShader.Get()->GetBufferSize();
	psByteCode.pShaderBytecode = pixelShader.Get()->GetBufferPointer();

	//D3D12_SHADER_BYTECODE vsByteCode = {};
	//UT::HelperFunc::CreateVertexShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicVS.hlsl", &vsByteCode);
	//
	//D3D12_SHADER_BYTECODE fsByteCode = {};
	//UT::HelperFunc::CreateFragmentShader("D:/Development/UltimateEngine/Game/Assets/Shaders/BasicFS.hlsl", &fsByteCode);



	// 3. Create Input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	// fill out an input layout description structure
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;


	//UT::HelperFunc::CreateVertexInputLayoutDesc(inputLayoutDesc);

	// 4. Create PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc;										// structure describing our input layout.
	psoDesc.pRootSignature = m_pRootSignature.Get();							// input data that this pso needs
	psoDesc.VS = vsByteCode;													// struct describing where to find vs bytecode & how large it is.
	psoDesc.PS = psByteCode;													// struct describing where to find fs bytecode & how large it is.
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		// topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;							// format of render target
	psoDesc.SampleDesc.Count = 1;												// same as swapchain
	psoDesc.SampleMask = 0xffffffff;											// multi-sampling, here we are choosing point sampling.
	psoDesc.RasterizerState = DirectX::CommonStates::CullCounterClockwise;		// default rasterizer state
	psoDesc.BlendState = DirectX::CommonStates::Opaque;							// default blend state
	psoDesc.NumRenderTargets = 1;												// we are binding only one render target

	//m_pDXRenderDevice->CreatePSO(psoDesc, m_pPSO);

	HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO));

	// 5. Create Vertex Buffer & transfer the data to GPU!
	DirectX::ResourceUploadBatch vbResourceUpload(m_pDXRenderDevice->GetD3DDevice().Get());
	vbResourceUpload.Begin();

	// vertex data...
	std::array<UT::DAS::VertexPC, 4> vertices;

	vertices[0] = { XMFLOAT3(-0.5f,  0.5f, 0.5f), XMFLOAT4(1,0,0,1) };
	vertices[1] = { XMFLOAT3( 0.5f, -0.5f, 0.5f), XMFLOAT4(0,1,0,1) };
	vertices[2] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(0,0,1,1) };
	vertices[3] = { XMFLOAT3( 0.5f,  0.5f, 0.5f), XMFLOAT4(1,0,1,1) };

	DirectX::CreateStaticBuffer(m_pDXRenderDevice->GetD3DDevice().Get(), vbResourceUpload, vertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pVBuffer);

	auto vbUploadResourcesFinished = vbResourceUpload.End(m_pDXRenderDevice->GetCommandQueue().Get());
	vbUploadResourcesFinished.wait();

	// 6. Create Indices data & transfer the data to GPU!
	DirectX::ResourceUploadBatch ibResourceUpload(m_pDXRenderDevice->GetD3DDevice().Get());
	ibResourceUpload.Begin();

	// index data...
	std::array<uint16_t, 6> indices;
	
	indices[0] = 0;		indices[1] = 1;		indices[2] = 2;
	indices[3] = 0,		indices[4] = 3;		indices[5] = 1;

	DirectX::CreateStaticBuffer(m_pDXRenderDevice->GetD3DDevice().Get(), ibResourceUpload, indices, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pIBuffer);

	auto ibUploadResourcesFinished = ibResourceUpload.End(m_pDXRenderDevice->GetCommandQueue().Get());
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
	m_VBView.StrideInBytes = sizeof(UT::DAS::VertexPC);
	m_VBView.SizeInBytes = vertices.size() * sizeof(UT::DAS::VertexPC);

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

	//---- TRIANGLE RENDERING END

	return true;
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

	SAFE_DELETE(m_pUIRenderer);
	SAFE_DELETE(m_pDXRenderDevice);
	//SAFE_RELEASE(m_pD3DGraphicsCommandList);
	//
	//for (uint16_t i = 0; i < UT::Globals::GBackbufferCount; ++i)
	//{
	//	SAFE_RELEASE(m_pListFences.at(i));
	//	SAFE_RELEASE(m_pListD3DCommandAllocator.at(i));
	//}

	//m_pListFences.clear();
	//m_pListFenceValue.clear();
	//m_pListD3DCommandAllocator.clear();
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
	const std::vector<ComPtr<ID3D12CommandList>> vecCommandList = { m_pD3DGraphicsCommandList };

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
	m_pD3DGraphicsCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
	m_pD3DGraphicsCommandList->RSSetViewports(1, &m_Viewport);
	m_pD3DGraphicsCommandList->RSSetScissorRects(1, &m_ScissorRect);
	m_pD3DGraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3DGraphicsCommandList->IASetVertexBuffers(0, 1, &m_VBView);
	m_pD3DGraphicsCommandList->IASetIndexBuffer(& m_IBView);
	m_pD3DGraphicsCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	

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
	const ComPtr<ID3D12Resource> pRenderTarget = m_pDXRenderDevice->GetRenderTarget(currFrameIndex);
	UT_ASSERT_NULL(pRenderTarget, "Current RenderTarget is NULL!");

	//-- Start recording commands into command list
	D3D12_RESOURCE_BARRIER rtBarrier = {};
	rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rtBarrier.Transition.pResource = pRenderTarget.Get();
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pD3DGraphicsCommandList->ResourceBarrier(1, &rtBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = m_pDXRenderDevice->GetCPUDescriptorHandleRTV();
	rtHandle.ptr += currFrameIndex * m_pDXRenderDevice->GetRTVDescriptorSize();

	m_pD3DGraphicsCommandList->OMSetRenderTargets(1, &rtHandle, false, nullptr);

	const float clearColor[] = { m_colorClear.x, m_colorClear.y, m_colorClear.z, m_colorClear.w };
	m_pD3DGraphicsCommandList->ClearRenderTargetView(rtHandle, clearColor, 0, nullptr);

	DrawCommands();

	D3D12_RESOURCE_BARRIER presentBarrier = {};
	presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentBarrier.Transition.pResource = pRenderTarget.Get();
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
		ComPtr<ID3D12CommandAllocator> pCmdAllocator;
		m_pDXRenderDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, pCmdAllocator);

		//HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCmdAllocator));
		//UT_CHECK_HRESULT(Hr, "Command Allocator creation failed!");

		m_pListD3DCommandAllocator.emplace_back(pCmdAllocator);
	}

	LOG_DEBUG("Command Allocator created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::CreateCommandList()
{
	// create the command list with the first allocator
	m_pDXRenderDevice->CreateGraphicsCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, m_pListD3DCommandAllocator.at(0), m_pD3DGraphicsCommandList);
	//HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pListD3DCommandAllocator.at(0).Get(), nullptr, IID_PPV_ARGS(&m_pD3DGraphicsCommandList));
	//UT_CHECK_HRESULT(Hr, "Cannot create command list!");

	// Command lists are created in "Recording" state. We do not want to record the command list yet, so we close it. 
	HRESULT Hr = m_pD3DGraphicsCommandList->Close();
	UT_CHECK_HRESULT(Hr, "Command List Close() failed!");

	LOG_DEBUG("Graphics command list created...");
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
	const ComPtr<ID3D12CommandAllocator> pCmdAllocator = m_pListD3DCommandAllocator.at(renderTargetID);

	UT_ASSERT_NULL(pCmdAllocator, ": Command Allocator");

	Hr = m_pD3DGraphicsCommandList->Reset(pCmdAllocator.Get(), m_pPSO.Get());
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
		ComPtr<ID3D12Fence> pFence;
		uint64_t uiFenceValue = 0;

		//HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
		//UT_CHECK_HRESULT(Hr, "Failed to create Fence!");

		m_pDXRenderDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, pFence);

		m_pListFences.emplace_back(pFence);
		m_pListFenceValue.emplace_back(uiFenceValue);	// Initialize with zero. 
	}

	// create a handle to a fence event...
	m_handleFenceEvent = CreateEvent(nullptr, false, false, nullptr);
	UT_CHECK_NULL(m_handleFenceEvent, "Fence event creation failed!");

	LOG_DEBUG("Fences & Fence event created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
uint32_t DXRenderer::WaitForPreviousFrame()
{
	HRESULT Hr = 0;

	// swap the current rtv buffer index, so we draw on the correct buffer
	const uint32_t uiCurrentFrameIndex = m_pDXRenderDevice->GetCurrentBackbufferIndex();

	// Acquire pointers for code readibility!
	const ComPtr<ID3D12Fence>	pFence	= m_pListFences.at(uiCurrentFrameIndex);
	const uint64_t	uiFenceValue		= m_pListFenceValue.at(uiCurrentFrameIndex);

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
