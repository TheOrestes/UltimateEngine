#include "UltimateEnginePCH.h"
#include "D3DRenderer.h"
#include "D3DGlobals.h"

#include "RayTracer/RT_Scene.h"

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::D3DRenderer()
{
	
}

//-------------------------------------------------------------------------------------------------------------------
D3DRenderer::~D3DRenderer()
{
	SAFE_DELETE(m_pRTScene);
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DRenderer::Initialize()
{
	UT_CHECK_BOOL(CreateRTV());
	UT_CHECK_BOOL(CreateUploadBuffer());

	m_pRTScene = new RT_Scene();
	m_pRTScene->Initialize(1);

	m_RTColor = Vector3(0, 0, 0);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DRenderer::RecordCommands()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	MODIFY_PIXELS_CPU();

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
void D3DRenderer::MODIFY_PIXELS_CPU()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	// Map Upload Buffer and Modify Pixels
	UINT8* mappedData;
	m_ResourceUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));


	for (UINT y = 0; y < UT::GLOBALS::GWindowHeight; ++y)
	{
		for (UINT x = 0; x < UT::GLOBALS::GWindowWidth; ++x)
		{
			const UINT pixelIndex = (y * UT::GLOBALS::GWindowWidth + x) * 4;

			const Vector3 color = m_pRTScene->Render(x, y, 25);

			// Dynamic pixel modification each frame
			mappedData[pixelIndex + 0] = color.x;  // Red
			mappedData[pixelIndex + 1] = color.y;  // Green
			mappedData[pixelIndex + 2] = color.z; // Blue
			mappedData[pixelIndex + 3] = 255; // Alpha
		}
	}

	m_ResourceUploadBuffer->Unmap(0, nullptr);
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

	return true;
}

