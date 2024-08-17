#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include "EngineHeader.h"

struct GLFWwindow;
class DXRenderDevice;

class UIRenderer
{
public:
	UIRenderer() {};
	~UIRenderer() { Cleanup(); };

	bool Initialize(const GLFWwindow* pWindow, const DXRenderDevice* pDXRenderDevice);
	void Begin();
	void Render(const DXRenderDevice* pDXRenderDevice, ComPtr<ID3D12GraphicsCommandList> pGraphicsCommandList, DirectX::XMFLOAT4& clearColor);
	void End(ComPtr<ID3D12GraphicsCommandList> pGraphicsCommandList);
	void Cleanup();
	void CleanupOnWindowResize();
	void RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight);

private:

};

