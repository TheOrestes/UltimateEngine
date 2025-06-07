#include "UltimateEnginePCH.h"
#include "D3DRenderer.h"
#include "RayTracer/RT_Scene.h"

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::D3DRenderer()
{
	m_PersistentData = nullptr;
}

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::~D3DRenderer()
{
	m_bAppRunning = false;
	m_dTotalRenderTime = 0;

	if(m_threadAccumulation.joinable())
	{
		m_threadAccumulation.join();
	}

	if(m_PersistentData)
	{
		m_ResourceUploadBuffer->Unmap(0, nullptr);
		m_PersistentData = nullptr;
	}

	SAFE_RELEASE(m_ResourceUploadBuffer);
	SAFE_DELETE(m_pRTScene);
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::Initialize()
{
	UT_CHECK_BOOL(CreateRTV());
	UT_CHECK_BOOL(CreateUploadBuffer());

	m_pRTScene = new RT_Scene();
	m_pRTScene->Initialize(10);

	m_bAppRunning = true;
	m_bAccumulationDone = false;

	m_RTColor = XMFLOAT3(0, 0, 0);

	m_ListAccumulatedBuffer.resize(UT::GLOBALS::GWindowWidth * UT::GLOBALS::GWindowHeight * 4);
	std::fill(m_ListAccumulatedBuffer.begin(), m_ListAccumulatedBuffer.end(), 0);

	m_ListSampleCount.resize(UT::GLOBALS::GWindowWidth * UT::GLOBALS::GWindowHeight);
	std::fill(m_ListSampleCount.begin(), m_ListSampleCount.end(), 0);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::RecordCommands()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	//-- Transition to COPY_DEST before copying data!
	D3D12_RESOURCE_BARRIER copyBarrier = {};
	copyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	copyBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	copyBarrier.Transition.pResource = m_ResourceRT[frameIndex];
	copyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	copyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	copyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &copyBarrier);

	D3D12_RESOURCE_DESC backBufferDesc = m_ResourceRT[frameIndex]->GetDesc();

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = m_ResourceUploadBuffer; // Source
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	pDevice->GetCopyableFootprints(&backBufferDesc, 0, 1, 0, &srcLocation.PlacedFootprint, nullptr, nullptr, nullptr);

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = m_ResourceRT[frameIndex]; // Destination
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	pCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	//-- Transition to RENDER_TARGET before rendering
	D3D12_RESOURCE_BARRIER rtBarrier = {};
	rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rtBarrier.Transition.pResource = m_ResourceRT[frameIndex];
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &rtBarrier);

	//-- RENDER!

	// Set Render Target
	pCommandList->OMSetRenderTargets(1, &m_handlesRTV[frameIndex], false, nullptr);
	//pCommandList->ClearRenderTargetView(m_handlesRTV[frameIndex], clearColor, 0, nullptr);

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
void D3DRenderer::StartRayTracerAccumulationThread()
{
	// Ensure the previous thread has finished before starting a new one
	if (m_threadAccumulation.joinable())
	{
		m_threadAccumulation.join();
	}

	m_bAppRunning = true;
	m_bAccumulationDone = false;

	m_threadAccumulation = std::thread([this]()
		{
			while (m_bAppRunning && !m_bAccumulationDone)
			{
				AccumulatePixels();
				std::this_thread::sleep_for(std::chrono::milliseconds(16));
			}
		});
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::AccumulatePixels()
{
	std::lock_guard<std::mutex> lock(m_mutexAccumulation); // Ensure thread safety

	if (m_bAccumulationDone) return;	// Stop execution if accumulation is complete

	const clock_t begin_time = clock();
	double counter = 0;

	for (UINT y = 0; y < UT::GLOBALS::GWindowHeight; ++y)
	{
		for (UINT x = 0; x < UT::GLOBALS::GWindowWidth; ++x)
		{
			RenderPixel(x, y);
		}
	}

	const clock_t end_time = clock();
	m_dTotalRenderTime = (end_time - begin_time) / (double)CLOCKS_PER_SEC;

	LOG_INFO("Total Ray tracing time = {0}", m_dTotalRenderTime);

	m_bAccumulationDone = true;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::RenderPixel(UINT x, UINT y)
{
	const UINT numSamples = m_pRTScene->GetSampleCount();  // Adjust the number of samples per pixel
	XMVECTOR accumulatedColor = XMVectorZero();

	for (UINT s = 0; s < numSamples; ++s)
	{
		XMFLOAT3 renderColor = m_pRTScene->Render(x, y);
		accumulatedColor = XMVectorAdd(accumulatedColor, XMLoadFloat3(&renderColor));
	}

	accumulatedColor = XMVectorScale(accumulatedColor, 1.0f / static_cast<float>(numSamples));

	// Store final computed color back to XMFLOAT3
	XMFLOAT3 finalColor;
	XMStoreFloat3(&finalColor, accumulatedColor);

	// Write to mapped GPU buffer
	const UINT pixelIndex = (y * UT::GLOBALS::GWindowWidth + x) * 4;
	m_PersistentData[pixelIndex + 0] = static_cast<UINT8>(finalColor.x);
	m_PersistentData[pixelIndex + 1] = static_cast<UINT8>(finalColor.y);
	m_PersistentData[pixelIndex + 2] = static_cast<UINT8>(finalColor.z);
	m_PersistentData[pixelIndex + 3] = 255;  // Alpha remains fixed
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
bool D3DRenderer::CreateUploadBuffer()
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

	// **Create Upload Buffer (Persistent)**
	D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = UT::GLOBALS::GWindowWidth * UT::GLOBALS::GWindowHeight * 4; // RGBA format
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	UT_CHECK_HRESULT(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ResourceUploadBuffer)));
	UT_NAME_D3D_OBJECT(m_ResourceUploadBuffer, "Upload Buffer");

	// Map Upload Buffer to store modified Pixels data...
	UINT8* mappedData;
	UT_ASSERT_HRESULT(m_ResourceUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));
	m_PersistentData = mappedData;

	return true;
}



