#pragma once

#include "D3D12Renderer/D3DGlobals.h"


class D3DCube
{
public:
    D3DCube();
    ~D3DCube();

    // Static geometry must be created once before creating Cubes
    static void CreateStaticGeometry();

    // Per-instance transform
    void    SetWorldPosition(float x, float y, float z);
    void    SetTexture(const std::string& fileName);

    void    UpdateConstantBuffer(const XMMATRIX& view, const DirectX::XMMATRIX& proj);

    void    Update(double dt);
    void    Render();

private:
    void    CreateTextureSRV();
    bool    CreateConstantBuffer();
    

    // === Shared static geometry ===
    static ID3D12Resource*                                      m_pSharedVB;
    static ID3D12Resource*                                      m_pSharedIB;
    static D3D12_VERTEX_BUFFER_VIEW                             m_sharedVBView;
    static D3D12_INDEX_BUFFER_VIEW                              m_sharedIBView;
    static bool                                                 s_geometryCreated;

    // === Per-instance data ===
    ID3D12Resource*                                             m_pTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE                                 m_HandleTextureSRV;

    XMMATRIX m_world;

    std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>   m_listCB;
    std::array<UINT8*, UT::GLOBALS::GFramesInFlight>            m_listCBDataBegin;
};

