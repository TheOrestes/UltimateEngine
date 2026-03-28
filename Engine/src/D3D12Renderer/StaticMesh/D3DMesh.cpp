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
}

//-------------------------------------------------------------------------------------------------------------------
D3DMesh::~D3DMesh()
{
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

	// Register SRV into global descriptor heap; get bindless index
	m_uiAlbedoID = UT::D3D12::HELPER::RegisterTextureSRV(m_pBaseTexture);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::UpdateConstantBuffer()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

	UT::D3D12::DAS::TransformData* gTransformPtr = UT::D3D12::DAS::GetGlobalTransformDataPtr();
	UT_ASSERT_NULL(gTransformPtr);

	gTransformPtr[m_uiTransformID].World = m_World;
	XMStoreFloat4x4(&gTransformPtr[m_uiTransformID].View, Camera::GetInstance().GetViewMatrix());
	XMStoreFloat4x4(&gTransformPtr[m_uiTransformID].Proj, Camera::GetInstance().GetProjectionMatrix());
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::Render()
{
	const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
	ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

	UT::D3D12::DAS::DrawConstants dc = {};
	dc.albedoID = m_uiAlbedoID;
	dc.transformID = m_uiTransformID;
	dc.pad0 = dc.pad1 = 0;

	pCommandList->SetGraphicsRoot32BitConstants(0, sizeof(UT::D3D12::DAS::DrawConstants) / 4, &dc, 0);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VBView);
	pCommandList->IASetIndexBuffer(&m_IBView);
	pCommandList->DrawIndexedInstanced(m_uiIndexCount, 1, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DMesh::Cleanup()
{
	SAFE_RELEASE(m_pBaseTexture);        // ID3D12Resource* texture
	SAFE_RELEASE(m_pVertexBuffer);   // per-mesh VB
	SAFE_RELEASE(m_pIndexBuffer);    // per-mesh IB
}


