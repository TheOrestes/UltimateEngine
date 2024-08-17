#include "UltimateEnginePCH.h"
#include "DXRenderer.h"

#include <wrl/client.h>

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
	//for (uint16_t i = 0; i < UT::D3DGlobals::GBackbufferCount; ++i)
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
	for (uint16_t i = 0; i < UT::D3DGlobals::GBackbufferCount; ++i)
	{
		ID3D12CommandAllocator* pCmdAllocator;
		HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCmdAllocator));
		UT_CHECK_HRESULT(Hr, "Command Allocator creation failed!");

		m_pListD3DCommandAllocator.emplace_back(pCmdAllocator);
	}

	LOG_DEBUG("Command Allocator created...");
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::CreateCommandList()
{
	// create the command list with the first allocator
	HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pListD3DCommandAllocator.at(0).Get(), nullptr, IID_PPV_ARGS(&m_pD3DGraphicsCommandList));
	UT_CHECK_HRESULT(Hr, "Cannot create command list!");

	// Command lists are created in "Recording" state. We do not want to record the command list yet, so we close it. 
	Hr = m_pD3DGraphicsCommandList->Close();
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

	Hr = m_pD3DGraphicsCommandList->Reset(pCmdAllocator.Get(), nullptr);
	UT_ASSERT_HRESULT(Hr, "CommandList Reset FAILED!");
}

//---------------------------------------------------------------------------------------------------------------------
bool DXRenderer::CreateFences()
{
	m_pListFences.reserve(UT::D3DGlobals::GBackbufferCount);
	m_pListFenceValue.reserve(UT::D3DGlobals::GBackbufferCount);

	// Create the fences...
	for (uint16_t i = 0; i < UT::D3DGlobals::GBackbufferCount; ++i)
	{
		ComPtr<ID3D12Fence> pFence;
		uint64_t uiFenceValue = 0;

		HRESULT Hr = m_pDXRenderDevice->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
		UT_CHECK_HRESULT(Hr, "Failed to create Fence!");

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
