#pragma once

#include "../Core/Core.h"
#include "../EngineHeader.h"
#include "D3D12Renderer/D3DGlobals.h"

class D3DRenderer;
class EngineApplication;

class UT_API DirectXApplication : public EngineApplication
{
public:
	DirectXApplication();
	virtual ~DirectXApplication() override;

	virtual void		Cleanup() override;
	virtual bool		Initialize(const GLFWwindow* pWindow);
	virtual void		Update(double dt);
	virtual void		Render();

	void				OnKeyPressed(UT::GLOBALS::InputAction action);
	void				OnKeyReleased(UT::GLOBALS::InputAction action);
	void				OnMouseMove(float x, float y, bool bMouseClicked);

	void				HandleWindowResizeCallback(const GLFWwindow* pWindow);

private:
	DirectXApplication(const DirectXApplication&);
	DirectXApplication& operator =(const DirectXApplication&);

	void				CleanupOnWindowResize();
	void				RecreateOnWindowResize(const GLFWwindow* pWindow);

	uint16_t			m_uiAppWidth;
	uint16_t			m_uiAppHeight;

	D3DRenderer*		m_pD3DRenderer;
};

