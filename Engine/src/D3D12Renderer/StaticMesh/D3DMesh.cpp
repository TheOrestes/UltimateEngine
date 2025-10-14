#include "UltimateEnginePCH.h"
#include "D3DMesh.h"
#include "ModelLoader.h"
#include "D3D12Renderer/World/Camera.h"

//-------------------------------------------------------------------------------------------------------------------
D3DMesh::D3DMesh()
{
	m_pModelLoader = nullptr;

	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_uiIndexCount = 0;

	m_pBaseTexture = nullptr;

	for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
	{
		m_listCB[i] = nullptr;
		m_listCBDataBegin[i] = nullptr;
	}

	m_HandleBaseTextureSRV.ptr = 0;
}

//-------------------------------------------------------------------------------------------------------------------
D3DMesh::~D3DMesh()
{
	for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++)
	{
		if (m_listCB[i])
		{
			m_listCB[i]->Unmap(0, nullptr);
			SAFE_RELEASE(m_listCB[i]);

			m_listCBDataBegin[i] = nullptr;
		}
	}

	SAFE_RELEASE(m_pBaseTexture);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBuffer);

	SAFE_DELETE(m_pModelLoader);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::SetMesh(const std::string& filePath)
{
	std::vector<UT::D3D12::DAS::VertexPNBT> vertices;
	std::vector<uint16_t> indices;

	m_pModelLoader = new ModelLoader();
	m_pModelLoader->LoadModel(filePath, vertices, indices);

	CreateMesh(vertices, indices);

	CreateConstantBuffer();
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::CreateMesh(const std::vector<UT::D3D12::DAS::VertexPNBT>& vertices, const std::vector<uint16_t>& indices)
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

	// --- Vertex buffer ---
	const UINT vbSize = sizeof(UT::D3D12::DAS::VertexPNBT) * vertices.size();
	UT::D3D12::HELPER::CreateUploadBuffer(vbSize, &m_pVertexBuffer);

	void* vbData = nullptr;
	constexpr D3D12_RANGE readRange = { 0, 0 };
	HRESULT Hr = m_pVertexBuffer->Map(0, &readRange, &vbData);
	memcpy(vbData, vertices.data(), vbSize);
	m_pVertexBuffer->Unmap(0, nullptr);

	m_VBView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VBView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPNBT);
	m_VBView.SizeInBytes = vbSize;

	// --- Index buffer ---
	m_uiIndexCount = indices.size();
	const UINT ibSize = sizeof(uint16_t) * m_uiIndexCount;
	UT::D3D12::HELPER::CreateUploadBuffer(ibSize, &m_pIndexBuffer);

	void* ibData = nullptr;
	m_pIndexBuffer->Map(0, &readRange, &ibData);
	memcpy(ibData, indices.data(), ibSize);
	m_pIndexBuffer->Unmap(0, nullptr);

	m_IBView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_IBView.SizeInBytes = ibSize;
	m_IBView.Format = DXGI_FORMAT_R16_UINT;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::SetTexture(const std::string& fileName)
{
	UT::D3D12::HELPER::CreateTexture(fileName, &m_pBaseTexture);

	// Create the Shader Resource View for texture!
	CreateTextureSRV();
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::UpdateConstantBuffer()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	UT::D3D12::DAS::GeomsCB cb;

	cb.World = m_World;
	XMStoreFloat4x4(&cb.View, Camera::GetInstance().GetViewMatrix());
	XMStoreFloat4x4(&cb.Proj, Camera::GetInstance().GetProjectionMatrix());

	memcpy(m_listCBDataBegin[frameIndex], &cb, sizeof(cb));
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::Render()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	ID3D12DescriptorHeap* descriptorHeaps[] = { UT::D3D12::CORE::GetGlobalDescriptorHeap() };

	pCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	pCommandList->SetGraphicsRootConstantBufferView(0, m_listCB[frameIndex]->GetGPUVirtualAddress());
	pCommandList->SetGraphicsRootDescriptorTable(1, m_HandleBaseTextureSRV);

	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VBView);
	pCommandList->IASetIndexBuffer(&m_IBView);
	pCommandList->DrawIndexedInstanced(m_uiIndexCount, 1, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::CreateTextureSRV()
{
	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();
	ID3D12DescriptorHeap* pDescriptorHeap = UT::D3D12::CORE::GetGlobalDescriptorHeap();

	const UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	const D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart = pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = cpuHeapStart;
	cpuHandle.ptr += UT::GLOBALS::GCurrentDescriptorIndex * descriptorSize;

	pDevice->CreateShaderResourceView(m_pBaseTexture, &srvDesc, cpuHandle);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = gpuHeapStart;
	gpuHandle.ptr += UT::GLOBALS::GCurrentDescriptorIndex * descriptorSize;
	m_HandleBaseTextureSRV = gpuHandle;

	// Increment counter for the Global descriptor counter!
	++UT::GLOBALS::GCurrentDescriptorIndex;
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DMesh::CreateConstantBuffer()
{
	// Create per-frame constant buffers
	constexpr UINT cbSize = (sizeof(UT::D3D12::DAS::GeomsCB) + 255) & ~255;

	// Map Constant buffer memory once for write access
	constexpr D3D12_RANGE readRange = { 0, 0 };	// We do not intent to read this resource on the CPU!

	for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++)
	{
		UT::D3D12::HELPER::CreateUploadBuffer(cbSize, &m_listCB[i]);

		UT_CHECK_HRESULT(m_listCB[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_listCBDataBegin[i])), "Failed to Map Constant Buffer!");

		// Zero initialize the mapped Constant Buffer!
		memset(m_listCBDataBegin[i], 0, cbSize);
	}

	return true;
}
