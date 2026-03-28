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
    static void DestroyStaticGeometry();

    void    SetTexture(const std::string& fileName);
    void    Render() override;
    void    Cleanup() override;

    void UpdateConstantBuffer() override;

private:

    // === Shared static geometry ===
    static ID3D12Resource*                                      m_pSharedVB;
    static ID3D12Resource*                                      m_pSharedIB;
    static D3D12_VERTEX_BUFFER_VIEW                             m_sharedVBView;
    static D3D12_INDEX_BUFFER_VIEW                              m_sharedIBView;
    static bool                                                 s_geometryCreated;

    // === Per-instance data ===
    ID3D12Resource*                                             m_pTexture;
};

