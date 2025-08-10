#include "UltimateEnginePCH.h"
#include "D3DCube.h"

//-------------------------------------------------------------------------------------------------------------------
// Static members
ID3D12Resource* D3DCube::m_pSharedVB;
ID3D12Resource* D3DCube::m_pSharedIB;
D3D12_VERTEX_BUFFER_VIEW D3DCube::m_sharedVBView = {};
D3D12_INDEX_BUFFER_VIEW  D3DCube::m_sharedIBView = {};
bool D3DCube::s_geometryCreated = false;

//-------------------------------------------------------------------------------------------------------------------
static const UT::D3D12::DAS::VertexPC kVertices[8] =
{
    { { -0.5f, -0.5f, -0.5f }, { 1,0,0,1 } },
    { {-0.5f, 0.5f,-0.5f},{0,1,0,1} },
    { { 0.5f, 0.5f,-0.5f},{0,0,1,1} },
    { { 0.5f,-0.5f,-0.5f},{1,1,0,1} },
    { {-0.5f,-0.5f, 0.5f},{1,0,1,1} },
    { {-0.5f, 0.5f, 0.5f},{0,1,1,1} },
    { { 0.5f, 0.5f, 0.5f},{1,1,1,1} },
    { { 0.5f,-0.5f, 0.5f},{0,0,0,1} }
};

static const uint16_t kIndices[36] =
{
    0,1,2, 0,2,3,
    4,6,5, 4,7,6,
    4,5,1, 4,1,0,
    3,2,6, 3,6,7,
    1,5,6, 1,6,2,
    4,0,3, 4,3,7
};

//-------------------------------------------------------------------------------------------------------------------
D3DCube::D3DCube()
{
   m_world = XMMatrixIdentity();

   CreateConstantBuffer();
}

//-------------------------------------------------------------------------------------------------------------------
D3DCube::~D3DCube()
{
    for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++)
    {
        if (m_listCB[i])
        {
            m_listCB[i]->Unmap(0, nullptr);
            m_listCBDataBegin[i] = nullptr;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::CreateStaticGeometry()
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
    ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

    if (s_geometryCreated) return;

    // --- Vertex buffer ---
    UINT vbSize = sizeof(kVertices);
    UT::D3D12::HELPER::CreateUploadBuffer(vbSize, &m_pSharedVB);

    void* vbData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    HRESULT Hr = m_pSharedVB->Map(0, &readRange, &vbData);// , "CreateStaticGeometry Map Failed!");
    memcpy(vbData, kVertices, vbSize);
    m_pSharedVB->Unmap(0, nullptr);

    m_sharedVBView.BufferLocation = m_pSharedVB->GetGPUVirtualAddress();
    m_sharedVBView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPC);
    m_sharedVBView.SizeInBytes = vbSize;

    // --- Index buffer ---
    UINT ibSize = sizeof(kIndices);
    UT::D3D12::HELPER::CreateUploadBuffer(vbSize, &m_pSharedIB);

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
void D3DCube::SetWorld(const XMMATRIX& world)
{
    m_world = world;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::SetColor(const XMFLOAT4& color)
{
    m_color = color;
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::UpdateConstantBuffer(const XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

    XMMATRIX wvp = XMMatrixTranspose(m_world * view * proj);

    UT::D3D12::DAS::CubesCB cb;
    XMStoreFloat4x4(&cb.WVP, wvp);
    cb.COLOR = m_color;

    memcpy(m_listCBDataBegin[frameIndex], &cb, sizeof(cb));
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::Update(double dt)
{
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::Render()
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
    ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->IASetVertexBuffers(0, 1, &m_sharedVBView);
    pCommandList->IASetIndexBuffer(&m_sharedIBView);
    pCommandList->SetGraphicsRootConstantBufferView(0, m_listCB[frameIndex]->GetGPUVirtualAddress());
    pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DCube::CreateConstantBuffer()
{
    // Create per-frame constant buffers
    constexpr UINT cbSize = (sizeof(UT::D3D12::DAS::CubesCB) + 255) & ~255;

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
