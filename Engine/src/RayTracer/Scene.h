#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Hitable.h"
#include "Material.h"
#include "Helper.h"
#include "HitableList.h"
#include "Lambertian.h"
#include "Metal.h"
#include "Sphere.h"
#include "Transparent.h"

#include "D3D12Renderer/D3DGlobals.h"

//-------------------------------------------------------------------------------------------------------------------
enum class SIMD_LEVEL
{
	AVX_128,
	AVX_256,
	AVX_512
};

//-------------------------------------------------------------------------------------------------------------------
class Scene
{
public:
	Scene() : m_uiSamples(0), m_uiCurrentSampleIndex(0), m_pCamera(nullptr), m_pWorld(nullptr) {} 
	~Scene() {
		SAFE_DELETE(m_pCamera);
		SAFE_DELETE(m_pWorld);
	}

	void			Initialize(uint16_t nSamples);
	XMFLOAT3		Render(uint16_t xPixel, uint16_t yPixel);
	void			IncrementSampleIndex() { ++m_uiCurrentSampleIndex; }

	uint16_t		GetCurrentSampleIndex() const { return m_uiCurrentSampleIndex;}
	uint16_t		GetSampleCount()		const { return m_uiSamples; }

private:
	SIMD_LEVEL		DetectSIMD();
	Hitable*		RandomScene();
	Hitable*		BasicScene();
	XMVECTOR		Trace(const Ray& r, Hitable* world, int depth);

	uint16_t		m_uiSamples;
	uint16_t		m_uiCurrentSampleIndex;
	Camera*			m_pCamera;
	Hitable*		m_pWorld;
};

//-------------------------------------------------------------------------------------------------------------------
inline SIMD_LEVEL Scene::DetectSIMD()
{
	int cpuInfo[4];

	// Check CPU feature flags
	__cpuid(cpuInfo, 1);

	bool hasOSXSAVE = (cpuInfo[2] & (1 << 27)) != 0;  // OS Save/Restore support (needed for AVX)
	bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;  // AVX support
	bool hasAVX2 = false;
	bool hasAVX512 = false;

	if (hasAVX && hasOSXSAVE)
	{
		// Check for AVX2 support
		__cpuid(cpuInfo, 7);
		hasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;  // AVX2 present
		hasAVX512 = (cpuInfo[1] & (1 << 16)) != 0; // AVX-512 present
	}

	if (hasAVX512) return SIMD_LEVEL::AVX_512;
	if (hasAVX2) return SIMD_LEVEL::AVX_256;
	if (hasAVX) return SIMD_LEVEL::AVX_128;

	return SIMD_LEVEL::AVX_128; // Fallback

}

//-------------------------------------------------------------------------------------------------------------------
inline void Scene::Initialize(uint16_t nSamples)
{
	m_uiSamples = nSamples;
	m_pWorld = BasicScene();

	SIMD_LEVEL level = DetectSIMD();

	constexpr XMVECTOR lookFrom = { 0.0f, 1.5f, 6.0f };
	constexpr XMVECTOR lookAt = { 0, 0, 0 };
	constexpr XMVECTOR Up = { 0, 1, 0 };
	constexpr float dist_to_focus = 1.0f;	// set this to 1.0 & apertue to 0.0f to stop DOF effect!
	constexpr float aperture = 0.0f;
	const float aspect = static_cast<float>(UT::GLOBALS::GWindowWidth) / UT::GLOBALS::GWindowHeight;

	m_pCamera = new Camera(lookFrom, lookAt, Up, 25.0f, aspect, aperture, dist_to_focus);
}

//-------------------------------------------------------------------------------------------------------------------
inline XMFLOAT3 Scene::Render(uint16_t xPixel, uint16_t yPixel)
{
	XMFLOAT3 color(0, 0, 0);

	const float u = (xPixel + Helper::GetRandom01()) / UT::GLOBALS::GWindowWidth;
	const float v = (yPixel + Helper::GetRandom01()) / UT::GLOBALS::GWindowHeight;

	const Ray r = m_pCamera->get_ray(u, v);

	// Convert XMFLOAT3 to XMVECTOR for computation
	XMVECTOR colorVec = XMLoadFloat3(&color);
	const XMVECTOR tracedColor = Trace(r, m_pWorld, 0);

	// Perform vector addition
	colorVec = XMVectorAdd(colorVec, tracedColor);

	// Convert back to XMFLOAT3 for storage
	XMStoreFloat3(&color, colorVec);

	float ir = (255.0 * color.x);
	float ig = (255.0 * color.y);
	float ib = (255.0 * color.z);

	return XMFLOAT3(ir, ig, ib);
}

//-------------------------------------------------------------------------------------------------------------------
inline Hitable* Scene::BasicScene()
{
	Hitable** list = new Hitable * [5];
	list[0] = new Sphere(XMVectorSet(1.05f, 0, 0, 0), 0.5, new Metal(XMFLOAT3(0.5f, 0.2f, 0.1f), 0.5));
	list[1] = new Sphere(XMVectorSet(0, -100.5f, 0, 0), 100, new Lambertian(XMFLOAT3(0.2f, 0.2f, 0.2f)));
	list[2] = new Sphere(XMVectorSet(0, 0, 0.1f, 0), 0.5, new Transparent(1.5f));
	list[3] = new Sphere(XMVectorSet(-1.05f, 0, 0, 0), 0.5, new Metal(XMFLOAT3(1.0, 0.2f, 0.0), 0));
	list[4] = new Sphere(XMVectorSet(0.0f, 0, -3, 0), 0.5, new Lambertian(XMFLOAT3(1.0f, 1.0f, 0.0f)));

	return new HitableList(list, 5);
}

//-------------------------------------------------------------------------------------------------------------------
inline Hitable* Scene::RandomScene()
{
	const int n = 500;
	Hitable** list = new Hitable * [n + 1];
	list[0] = new Sphere(XMVectorSet(0, -1000, 0, 0), 1000, new Lambertian(XMFLOAT3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			const float choose_mat = Helper::GetRandom01();
			const XMVECTOR center = { a + 0.9f * Helper::GetRandom01(), 0.2f, b + 0.9f * Helper::GetRandom01() };
			
			if (XMVectorGetX(XMVector3Length(XMVectorSubtract(center, XMVectorSet(4.0f, 0.2f, 0.0f, 0.0f)))) > 0.9f)
			{
				if (choose_mat < 0.8f)
				{
					// diffuse
					list[i++] = new Sphere(center, 0.2f, new Lambertian(XMFLOAT3(Helper::GetRandom01() * Helper::GetRandom01(), Helper::GetRandom01() * Helper::GetRandom01(), Helper::GetRandom01() * Helper::GetRandom01())));
				}
				else if (choose_mat < 0.95)
				{
					// Metal
					list[i++] = new Sphere(center, 0.2f, new Metal(XMFLOAT3(0.5f * (1 + Helper::GetRandom01()), 0.5f * (1 + Helper::GetRandom01()), 0.5f * (1 + Helper::GetRandom01())), Helper::GetRandom01()));
				}
				else
				{
					// glass
					list[i++] = new Sphere(center, 0.2f, new Transparent(1.5f));
				}
			}
		}
	}

	list[i++] = new Sphere(XMVectorSet(0, 1, 0, 0), 1.0f, new Transparent(1.5f));
	list[i++] = new Sphere(XMVectorSet(-4, 1, 0, 0), 1.0f, new Lambertian(XMFLOAT3(0.4f, 0.2f, 0.1f)));
	list[i++] = new Sphere(XMVectorSet(4, 1, 0, 0), 1.0f, new Metal(XMFLOAT3(0.7f, 0.6f, 0.5f), 0.0f));

	return new HitableList(list, i);
}

//-------------------------------------------------------------------------------------------------------------------
inline XMVECTOR Scene::Trace(const Ray& r, Hitable* world, int depth)
{
	HitRecord rec;

	if (m_pWorld->hit(r, 0.001f, FLT_MAX, rec))
	{
		Ray scatteredRay;
		XMFLOAT3 attenuation;

		if (depth < 50 && rec.mat_ptr->Scatter(r, rec, attenuation, scatteredRay))
		{
			const XMVECTOR atten = XMLoadFloat3(&attenuation);
			return XMVectorMultiply(atten, Trace(scatteredRay, world, depth + 1));
		}
		else
		{
			return XMVECTOR{ 0, 0, 0, 0 };
		}
	}
	else
	{
		const XMVECTOR unit_direction = XMVector3Normalize(r.GetRayDirection());
		const float t = 0.5f * (XMVectorGetY(unit_direction) + 1.0f);
		return XMVectorLerp(XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f), XMVectorSet(0.5f, 0.7f, 1.0f, 0.0f), t);
	}
}

