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
	m_world = XMMatrixIdentity();
    m_pTexture = nullptr;

    for (UINT i = 0; i < UT::GLOBALS::GFramesInFlight; ++i)
    {
        m_listCB[i] = nullptr;
        m_listCBDataBegin[i] = nullptr;

        m_HandleTextureSRV.ptr = 0;
    }

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
void D3DCube::SetWorldPosition(float x, float y, float z)
{
    m_world = XMMatrixTranslation(x, y, z);
}

//-------------------------------------------------------------------------------------------------------------------
void D3DCube::UpdateConstantBuffer(const XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;

    UT::D3D12::DAS::GeomsCB cb;
    XMStoreFloat4x4(&cb.World, m_world);
    XMStoreFloat4x4(&cb.View, view);
    XMStoreFloat4x4(&cb.Proj, proj);

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
    ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

    const uint16_t frameIndex = UT::GLOBALS::GCurrentFrameId;
    ID3D12CommandAllocator* const pCmdAlloc = UT::D3D12::CORE::GetCommandAllocator(frameIndex);
    ID3D12GraphicsCommandList* const pCommandList = UT::D3D12::CORE::GetCommandList(frameIndex);

    int width, height, channels = 0;
    void* imgData = nullptr;

    // Load image data from the file!
    UT::D3D12::HELPER::LoadImageData(fileName, &width, &height, &channels, &imgData);

    // Create Texture resource!
    if(imgData)
    {
        // Describe Texture resource
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        // Create default heap resource
        HRESULT Hr = pDevice->CreateCommittedResource(&heapProps,
                                                            D3D12_HEAP_FLAG_NONE,
                                                            &texDesc,
                                                            D3D12_RESOURCE_STATE_COPY_DEST,
                                                            nullptr,
                                                            IID_PPV_ARGS(&m_pTexture));

        UT_ASSERT_HRESULT(Hr, "CreateResource => Texture Buffer");
        UT_NAME_D3D_OBJECT(m_pTexture, "TextureResource: " + UT::GLOBALS::GetFileNameWithoutExtension(fileName));

        //-- Create upload buffer for the texture!
        UINT64 uploadBufferSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        UINT numRows = 0; 
        UINT64 rowSizeInBytes = 0; UINT64 totalBytes = 0;

        pDevice->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

        ID3D12Resource* pUploadHeap = nullptr;
        UT::D3D12::HELPER::CreateUploadBuffer(totalBytes, &pUploadHeap);

        //-- Write image to the upload heap!
        UINT8* pUploadData = nullptr;

        Hr = pUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&pUploadData));
        UT_CHECK_HRESULT(Hr, "Map Texture buffer failed!");

        for(UINT row = 0 ; row < numRows ; ++row)
        {
            memcpy(pUploadData + footprint.Offset + row * footprint.Footprint.RowPitch,
                static_cast<const UINT8*>(imgData) + row * rowSizeInBytes,
                rowSizeInBytes);
        }

        pUploadHeap->Unmap(0, nullptr);

        //-- Copy to the Default Heap (GPU)!
        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = m_pTexture;
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = pUploadHeap;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint = footprint;

        UT::D3D12::CORE::ResetCommandList();

        pCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_pTexture;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        pCommandList->ResourceBarrier(1, &barrier);

        UT::D3D12::CORE::CloseAndExecuteCommandList();
    }

    // Create the Shader Resource View for texture!
    CreateTextureSRV();
}
