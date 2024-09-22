#pragma once

#if _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <dxgi1_6.h>

#include "../Core/Core.h"
#include "../EngineHeader.h"

class DXRenderer;
class EngineApplication;
enum class CameraAction;

class DirectXApplication : public EngineApplication
{
public:
	DirectXApplication();
	virtual ~DirectXApplication() override;

	virtual void							Cleanup() override;
	virtual bool							Initialize(const GLFWwindow* pWindow);
	virtual void							Update(double dt);
	virtual void							Render();

	void									EnableDebug();
	void									DisableDebug();

	void									HandleSceneInput(const GLFWwindow* pWindow, CameraAction direction, float mousePosX = 0.0f, float mousePosY = 0.0f, bool isMouseClicked = false) const;
	void									HandleWindowResizeCallback(const GLFWwindow* pWindow);

private:
	DirectXApplication(const DirectXApplication&);
	DirectXApplication& operator =(const DirectXApplication&);

	void									CleanupOnWindowResize();
	void									RecreateOnWindowResize(const GLFWwindow* pWindow);

	uint16_t								m_uiAppWidth;
	uint16_t								m_uiAppHeight;

	HWND									m_hwnd;
	std::string								m_strGPUName;

	ComPtr<IDXGIFactory6>					m_pDXGIFactory;
	ComPtr<ID3D12Debug1>					m_pD3DDebug;
	ComPtr<IDXGIDebug1>						m_pDXGIDebug;

	DXRenderer*								m_pDXRenderer;
};

