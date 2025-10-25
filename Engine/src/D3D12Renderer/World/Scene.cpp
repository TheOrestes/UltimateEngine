#include "UltimateEnginePCH.h"
#include "Scene.h"

#include "D3D12Renderer/StaticMesh/D3DCube.h"
#include "D3D12Renderer/StaticMesh/D3DMesh.h"
#include "D3D12Renderer/World/GameObject.h"
#include "Camera.h"

//-------------------------------------------------------------------------------------------------------------------
Scene::Scene()
{
	m_pCubeRed = nullptr;
	m_pCubeBlue = nullptr;
	m_pCubeGreen = nullptr;
	m_pMesh = nullptr;
}

//-------------------------------------------------------------------------------------------------------------------
Scene::~Scene()
{
	SAFE_DELETE(m_pMesh);
	SAFE_DELETE(m_pCubeRed);
	SAFE_DELETE(m_pCubeGreen);
	SAFE_DELETE(m_pCubeBlue);
}

//-------------------------------------------------------------------------------------------------------------------
bool Scene::Initialize()
{
	D3DCube::CreateStaticGeometry();

	m_pCubeRed = new D3DCube();
	m_pCubeRed->SetName("RedCube");
	m_pCubeRed->SetPosition(-2, 0, 0);
	m_pCubeRed->SetTexture("Assets/Textures/Red/texture_10.png");

	m_pCubeGreen = new D3DCube();
	m_pCubeGreen->SetName("GreenCube");
	m_pCubeGreen->SetPosition(0, 0, 0);
	m_pCubeGreen->SetTexture("Assets/Textures/Green/texture_09.png");

	m_pCubeBlue = new D3DCube();
	m_pCubeBlue->SetName("BlueCube");
	m_pCubeBlue->SetPosition(2, 0, 0);
	m_pCubeBlue->SetTexture("Assets/Textures/Purple/texture_05.png");

	m_pMesh = new D3DMesh();
	m_pMesh->SetName("Barbarian");
	m_pMesh->SetMesh("Assets/Models/Barbarian/BarbNew2.fbx");
	m_pMesh->SetTexture("Assets/Models/Barbarian/Body_Color.jpg");
	m_pMesh->SetPosition(0, 0.5f, 0);
	m_pMesh->SetRotation(0, 1, 0, XM_PI);
	m_pMesh->SetScale(0.1f, 0.1f, 0.1f);

	Camera::GetInstance().SetPosition(0.0f, 1.0f, -3.0f);
	Camera::GetInstance().SetRotation(0, 0, 0);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
void Scene::Update(double dt)
{
	Camera::GetInstance().Update(dt);

	m_pCubeRed->Update(dt);
	m_pCubeGreen->Update(dt);
	m_pCubeBlue->Update(dt);
	m_pMesh->Update(dt);
}

//-------------------------------------------------------------------------------------------------------------------
void Scene::Render()
{
	m_pCubeRed->Render();
	m_pCubeGreen->Render();
	m_pCubeBlue->Render();
	m_pMesh->Render();
}

//-------------------------------------------------------------------------------------------------------------------
void Scene::Cleanup()
{
}
