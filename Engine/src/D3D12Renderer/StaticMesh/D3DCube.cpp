#include "UltimateEnginePCH.h"
#include "D3DCube.h"
#include "D3D12Renderer/World/Camera.h"

//-------------------------------------------------------------------------------------------------------------------
// Static members
ID3D12Resource* D3DCube::m_pSharedVB;
ID3D12Resource* D3DCube::m_pSharedIB;
D3D12_VERTEX_BUFFER_VIEW D3DCube::m_sharedVBView = {};
D3D12_INDEX_BUFFER_VIEW  D3DCube::m_sharedIBView = {};
bool D3DCube::s_geometryCreated = false;

//-------------------------------------------------------------------------------------------------------------------
static const UT::D3D12::DAS::VertexPNBT kVertices[24] =
{
  //    // Position               // Normal       // BiNormal    // UV
  //  { { -0.5f, -0.5f, -0.5f },  { 0, 0, -1 },   { 1, 0, 0 },   { 0.0f, 1.0f } },  // 0: Front-bottom-left
  //  { { -0.5f,  0.5f, -0.5f },  { 0, 0, -1 },   { 1, 0, 0 },   { 1.0f, 1.0f } },  // 1: Front-bottom-right
  //  { {  0.5f,  0.5f, -0.5f },  { 0, 0, -1 },   { 1, 0, 0 },   { 1.0f, 0.0f } },  // 2: Front-top-right
  //  { {  0.5f, -0.5f, -0.5f },  { 0, 0, -1 },   { 1, 0, 0 },   { 0.0f, 0.0f } },  // 3: Front-top-left
  //                              
  //  { { -0.5f, -0.5f,  0.5f },  { 0, 0,  1 },   { -1, 0, 0 },  { 0.0f, 1.0f } },  // 4: Back-bottom-left
  //  { { -0.5f,  0.5f,  0.5f },  { 0, 0,  1 },   { -1, 0, 0 },  { 1.0f, 1.0f } },  // 5: Back-bottom-right
  //  { {  0.5f,  0.5f,  0.5f },  { 0, 0,  1 },   { -1, 0, 0 },  { 1.0f, 0.0f } },  // 6: Back-top-right
  //  { {  0.5f, -0.5f,  0.5f },  { 0, 0,  1 },   { -1, 0, 0 },  { 0.0f, 0.0f } }   // 7: Back-top-left

    // Front face (+Z)
    { { -0.5f, -0.5f, -0.5f }, { 0, 0, -1 },  { 1, 0, 0 },  { 0.0f, 1.0f } }, // 0
    { { -0.5f,  0.5f, -0.5f }, { 0, 0, -1 },  { 1, 0, 0 },  { 0.0f, 0.0f } }, // 1
    { {  0.5f,  0.5f, -0.5f }, { 0, 0, -1 },  { 1, 0, 0 },  { 1.0f, 0.0f } }, // 2
    { {  0.5f, -0.5f, -0.5f }, { 0, 0, -1 },  { 1, 0, 0 },  { 1.0f, 1.0f } }, // 3

    // Back face (-Z)
    { { -0.5f, -0.5f,  0.5f }, { 0, 0,  1 },  { -1, 0, 0 }, { 1.0f, 1.0f } }, // 4
    { {  0.5f, -0.5f,  0.5f }, { 0, 0,  1 },  { -1, 0, 0 }, { 0.0f, 1.0f } }, // 5
    { {  0.5f,  0.5f,  0.5f }, { 0, 0,  1 },  { -1, 0, 0 }, { 0.0f, 0.0f } }, // 6
    { { -0.5f,  0.5f,  0.5f }, { 0, 0,  1 },  { -1, 0, 0 }, { 1.0f, 0.0f } }, // 7

    // Left face (-X)
    { { -0.5f, -0.5f,  0.5f }, { -1, 0, 0 },  { 0, 0, -1 }, { 0.0f, 1.0f } }, // 8
    { { -0.5f,  0.5f,  0.5f }, { -1, 0, 0 },  { 0, 0, -1 }, { 0.0f, 0.0f } }, // 9
    { { -0.5f,  0.5f, -0.5f }, { -1, 0, 0 },  { 0, 0, -1 }, { 1.0f, 0.0f } }, // 10
    { { -0.5f, -0.5f, -0.5f }, { -1, 0, 0 },  { 0, 0, -1 }, { 1.0f, 1.0f } }, // 11

    // Right face (+X)
    { { 0.5f, -0.5f, -0.5f },  { 1, 0, 0 },   { 0, 0, 1 },  { 0.0f, 1.0f } }, // 12
    { { 0.5f,  0.5f, -0.5f },  { 1, 0, 0 },   { 0, 0, 1 },  { 0.0f, 0.0f } }, // 13
    { { 0.5f,  0.5f,  0.5f },  { 1, 0, 0 },   { 0, 0, 1 },  { 1.0f, 0.0f } }, // 14
    { { 0.5f, -0.5f,  0.5f },  { 1, 0, 0 },   { 0, 0, 1 },  { 1.0f, 1.0f } }, // 15

    // Top face (+Y)
    { { -0.5f,  0.5f, -0.5f }, { 0, 1, 0 },   { 1, 0, 0 },  { 0.0f, 1.0f } }, // 16
    { { -0.5f,  0.5f,  0.5f }, { 0, 1, 0 },   { 1, 0, 0 },  { 0.0f, 0.0f } }, // 17
    { {  0.5f,  0.5f,  0.5f }, { 0, 1, 0 },   { 1, 0, 0 },  { 1.0f, 0.0f } }, // 18
    { {  0.5f,  0.5f, -0.5f }, { 0, 1, 0 },   { 1, 0, 0 },  { 1.0f, 1.0f } }, // 19

    // Bottom face (-Y)
    { { -0.5f, -0.5f,  0.5f }, { 0, -1, 0 },  { 1, 0, 0 },  { 0.0f, 1.0f } }, // 20
    { { -0.5f, -0.5f, -0.5f }, { 0, -1, 0 },  { 1, 0, 0 },  { 0.0f, 0.0f } }, // 21
    { {  0.5f, -0.5f, -0.5f }, { 0, -1, 0 },  { 1, 0, 0 },  { 1.0f, 0.0f } }, // 22
    { {  0.5f, -0.5f,  0.5f }, { 0, -1, 0 },  { 1, 0, 0 },  { 1.0f, 1.0f } }, // 23
};

static const uint16_t kIndices[36] =
{
    //0,1,2, 0,2,3,
    //4,6,5, 4,7,6,
    //4,5,1, 4,1,0,
    //3,2,6, 3,6,7,
    //1,5,6, 1,6,2,
    //4,0,3, 4,3,7

     // Front face
    0, 1, 2,  0, 2, 3,
    // Back face
    4, 5, 6,  4, 6, 7,
    // Left face
    8, 9, 10, 8, 10,11,
    // Right face
    12,13,14, 12,14,15,
    // Top face
    16,17,18, 16,18,19,
    // Bottom face
    20,21,22, 20,22,23
};

//-------------------------------------------------------------------------------------------------------------------
D3DCube::D3DCube()
{
    m_pTexture = nullptr;
}

//-------------------------------------------------------------------------------------------------------------------
D3DCube::~D3DCube()
{
    SAFE_RELEASE(m_pTexture);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::CreateStaticGeometry()
{
    if (s_geometryCreated) return;

    // --- Vertex buffer ---
    constexpr UINT vbSize = sizeof(kVertices);
    UT::D3D12::HELPER::CreateUploadBuffer(vbSize, &m_pSharedVB);

    void* vbData = nullptr;
    constexpr D3D12_RANGE readRange = { 0, 0 };
    HRESULT Hr = m_pSharedVB->Map(0, &readRange, &vbData);// , "CreateStaticGeometry Map Failed!");
    memcpy(vbData, kVertices, vbSize);
    m_pSharedVB->Unmap(0, nullptr);

    m_sharedVBView.BufferLocation = m_pSharedVB->GetGPUVirtualAddress();
    m_sharedVBView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPNBT);
    m_sharedVBView.SizeInBytes = vbSize;

    // --- Index buffer ---
    constexpr UINT ibSize = sizeof(kIndices);
    UT::D3D12::HELPER::CreateUploadBuffer(ibSize, &m_pSharedIB);

    void* ibData = nullptr;
    m_pSharedIB->Map(0, &readRange, &ibData);
    memcpy(ibData, kIndices, ibSize);
    m_pSharedIB->Unmap(0, nullptr);

    m_sharedIBView.BufferLocation = m_pSharedIB->GetGPUVirtualAddress();
    m_sharedIBView.SizeInBytes = ibSize;
    m_sharedIBView.Format = DXGI_FORMAT_R16_UINT;

    s_geometryCreated = true;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::DestroyStaticGeometry()
{
    SAFE_RELEASE(m_pSharedVB);      // static shared VB
    SAFE_RELEASE(m_pSharedIB);      // static shared IB
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::UpdateConstantBuffer()
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

    UT::D3D12::DAS::TransformData* gTransformPtr = UT::D3D12::DAS::GetGlobalTransformDataPtr();
    UT_ASSERT_NULL(gTransformPtr);

    gTransformPtr[m_uiTransformID].World = m_World;
    XMStoreFloat4x4(&gTransformPtr[m_uiTransformID].View, Camera::GetInstance().GetViewMatrix());
    XMStoreFloat4x4(&gTransformPtr[m_uiTransformID].Proj, Camera::GetInstance().GetProjectionMatrix());
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::Render()
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
    ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

    UT::D3D12::DAS::DrawConstants dc;
    dc.albedoID = m_uiAlbedoID;
    dc.transformID = m_uiTransformID;
    dc.pad0 = dc.pad1 = 0;

    pCommandList->SetGraphicsRoot32BitConstants(0, sizeof(UT::D3D12::DAS::DrawConstants) / 4, &dc, 0);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_sharedVBView);
    pCommandList->IASetIndexBuffer(&m_sharedIBView);
    pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::Cleanup()
{
    SAFE_RELEASE(m_pTexture);        // ID3D12Resource* texture
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::SetTexture(const std::string& fileName)
{
    UT::D3D12::HELPER::CreateTexture(fileName, &m_pTexture);

    m_uiAlbedoID = UT::D3D12::HELPER::RegisterTextureSRV(m_pTexture);
}
