#pragma once

#include "D3D12Renderer/D3DGlobals.h"
#include "D3D12Renderer/World/GameObject.h"

class GameObject;

class D3DCube : public GameObject
{
public:
    D3DCube();
    virtual ~D3DCube();

    // Static geometry must be created once before creating Cubes
    static void CreateStaticGeometry();

    void    SetTexture(const std::string& fileName);
    void    Render() override;

private:
    void    CreateTextureSRV();
    bool    CreateConstantBuffer();

    void    UpdateConstantBuffer() override;

    // === Shared static geometry ===
    static ID3D12Resource*                                      m_pSharedVB;
    static ID3D12Resource*                                      m_pSharedIB;
    static D3D12_VERTEX_BUFFER_VIEW                             m_sharedVBView;
    static D3D12_INDEX_BUFFER_VIEW                              m_sharedIBView;
    static bool                                                 s_geometryCreated;

    // === Per-instance data ===
    ID3D12Resource*                                             m_pTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE                                 m_HandleTextureSRV;

    std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>   m_listCB;
    std::array<UINT8*, UT::GLOBALS::GFramesInFlight>            m_listCBDataBegin;
};

