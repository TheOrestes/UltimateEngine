#pragma once

#include "D3D12Renderer/D3DGlobals.h"
#include "D3D12Renderer/World/GameObject.h"

class ModelLoader;
class GameObject;

class D3DMesh : public GameObject
{
public:
    D3DMesh();
    ~D3DMesh();

    void    SetMesh(const std::string& filePath) override;
    void    SetTexture(const std::string& fileName);
   

    void    Render() override;

private:
    void    CreateMesh(const std::vector<UT::D3D12::DAS::VertexPNBT>& vertices, const std::vector<uint16_t>& indices);
    void    CreateTextureSRV();
    bool    CreateConstantBuffer();
    void    UpdateConstantBuffer() override;

    // D3D resources for vertices & indices
    ID3D12Resource*                                             m_pVertexBuffer;
    ID3D12Resource*                                             m_pIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                                    m_VBView;
    D3D12_INDEX_BUFFER_VIEW                                     m_IBView;

    uint32_t                                                    m_uiIndexCount;

    // === Per-instance data ===
    ID3D12Resource*                                             m_pBaseTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE                                 m_HandleBaseTextureSRV;

    std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>   m_listCB;
    std::array<UINT8*, UT::GLOBALS::GFramesInFlight>            m_listCBDataBegin;

    ModelLoader*                                                m_pModelLoader;
};

