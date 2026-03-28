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
    void    Cleanup() override;

private:
    void    CreateMesh(const std::vector<UT::D3D12::DAS::VertexPNBT>& vertices, const std::vector<uint16_t>& indices);
    void    UpdateConstantBuffer() override;

    // D3D resources for vertices & indices
    ID3D12Resource*                                             m_pVertexBuffer;
    ID3D12Resource*                                             m_pIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                                    m_VBView;
    D3D12_INDEX_BUFFER_VIEW                                     m_IBView;

    uint32_t                                                    m_uiIndexCount;

    // === Per-instance data ===
    ID3D12Resource*                                             m_pBaseTexture;
    ModelLoader*                                                m_pModelLoader;
};

