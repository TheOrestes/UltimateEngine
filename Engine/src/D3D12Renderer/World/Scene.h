#pragma once

#include "D3D12Renderer/D3DGlobals.h"

class GameObject;
class D3DCube;
class D3DMesh;

class Scene
{
public:
	static Scene& getInstance()
	{
		static Scene instance;
		return instance;
	}

	// Delete copy-constructor & assignment operator
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	bool	Initialize();
	void	Update(double dt);
	void	Render();
	void	Cleanup();

private:
	Scene();
	~Scene();

private:
	GameObject* m_pCubeRed;
	GameObject* m_pCubeGreen;
	GameObject* m_pCubeBlue;

	GameObject* m_pMesh;

	uint32_t	m_uiNextTransformID = 0;
};

