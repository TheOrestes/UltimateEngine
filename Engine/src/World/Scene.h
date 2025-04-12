#pragma once

#include "D3D12Renderer/D3DGlobals.h"

class FreeCamera;

class Scene
{
public:
	Scene() {}
	~Scene() { Cleanup(); }

	void LoadScene();
	void Cleanup();

	void Update(double dt);
	void HandleSceneInput(UT::Globals::InputAction action, float mousePosX, float mousePosY);
};

