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

    for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
    {
        m_listCB[i] = nullptr;
        m_listCBDataBegin[i] = nullptr;
    }

    m_HandleTextureSRV.ptr = 0;
	CreateConstantBuffer();
}

//-------------------------------------------------------------------------------------------------------------------
D3DCube::~D3DCube()
{
    SAFE_RELEASE(m_pTexture);

    for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; i++)
    {
        if (m_listCB[i])
        {
            m_listCB[i]->Unmap(0, nullptr);
            SAFE_RELEASE(m_listCB[i]);
            
            m_listCBDataBegin[i] = nullptr;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::CreateStaticGeometry()
{
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
    m_sharedVBView.StrideInBytes = sizeof(UT::D3D12::DAS::VertexPNBT);
    m_sharedVBView.SizeInBytes = vbSize;

    // --- Index buffer ---
    UINT ibSize = sizeof(kIndices);
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
void D3DCube::UpdateConstantBuffer()
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

    UT::D3D12::DAS::GeomsCB cb;

    cb.World = m_World;
    XMStoreFloat4x4(&cb.View, Camera::GetInstance().GetViewMatrix());
    XMStoreFloat4x4(&cb.Proj, Camera::GetInstance().GetProjectionMatrix());

    memcpy(m_listCBDataBegin[frameIndex], &cb, sizeof(cb));
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::Render()
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
    ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

    ID3D12DescriptorHeap* descriptorHeaps[] = { UT::D3D12::CORE::GetGlobalDescriptorHeap() };

    pCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    pCommandList->SetGraphicsRootConstantBufferView(0, m_listCB[frameIndex]->GetGPUVirtualAddress());
    pCommandList->SetGraphicsRootDescriptorTable(1, m_HandleTextureSRV);

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_sharedVBView);
    pCommandList->IASetIndexBuffer(&m_sharedIBView);
    pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::CreateTextureSRV()
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

    pDevice->CreateShaderResourceView(m_pTexture, &srvDesc, cpuHandle);
    
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = gpuHeapStart;
    gpuHandle.ptr += UT::GLOBALS::GCurrentDescriptorIndex * descriptorSize;
    m_HandleTextureSRV = gpuHandle;

    // Increment counter for the Global descriptor counter!
    ++UT::GLOBALS::GCurrentDescriptorIndex;
}

//-------------------------------------------------------------------------------------------------------------------
bool D3DCube::CreateConstantBuffer()
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

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::SetTexture(const std::string& fileName)
{
    UT::D3D12::HELPER::CreateTexture(fileName, &m_pTexture);

    // Create the Shader Resource View for texture!
    CreateTextureSRV();
}
