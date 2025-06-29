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
class Scene
{
public:
	Scene() : m_uiSamples(0), m_uiCurrentSampleIndex(0), m_pCamera(nullptr), m_pWorld(nullptr)
	{
	}

	~Scene() {
		SAFE_DELETE(m_pCamera);
		SAFE_DELETE(m_pWorld);
	}

	void			Initialize(uint16_t nSamples);
	FastVector		Render(uint16_t xPixel, uint16_t yPixel);
	void			IncrementSampleIndex() { ++m_uiCurrentSampleIndex; }

	uint16_t		GetCurrentSampleIndex() const { return m_uiCurrentSampleIndex;}
	uint16_t		GetSampleCount()		const { return m_uiSamples; }

private:
	void			DetectSIMD();
	Hitable*		RandomScene();
	Hitable*		BasicScene();
	FastVector		Trace(const Ray& r, Hitable* world, int depth);

	uint16_t		m_uiSamples;
	uint16_t		m_uiCurrentSampleIndex;
	Camera*			m_pCamera;
	Hitable*		m_pWorld;
};

//-------------------------------------------------------------------------------------------------------------------
inline void Scene::DetectSIMD()
{
	int cpuInfo[4];

	// Check CPU feature flags
	__cpuid(cpuInfo, 1);

	const bool hasOSXSAVE = (cpuInfo[2] & (1 << 27)) != 0;  // OS Save/Restore support (needed for AVX)
	const bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;  // AVX support

	bool hasAVX512 = false;
	bool hasAVX2 = false;

	if (hasAVX && hasOSXSAVE)
	{
		// Check for AVX2 support
		__cpuid(cpuInfo, 7);
		hasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;  // AVX2 present
		hasAVX512 = (cpuInfo[1] & (1 << 16)) != 0; // AVX-512 present
	}

#if defined ENABLE_AVX512
	if (hasAVX512)
	{
		UT::GLOBALS::SIMD_TYPE = UT::GLOBALS::SIMDType::AVX512;
		LOG_WARNING("Using AVX512 Instruction Set...!");
	}
	else
		LOG_ERROR("AVX512 not supported!");
	
#elif defined ENABLE_AVX256
	if (hasAVX2)
	{
		UT::GLOBALS::SIMD_TYPE = UT::GLOBALS::SIMDType::AVX2;
		LOG_WARNING("Using AVX256 Instruction Set...!");
	}
	else
		LOG_ERROR("AVX256 not supported!");

#elif defined ENABLE_AVX128
	if (hasAVX)
	{
		UT::GLOBALS::SIMD_TYPE = UT::GLOBALS::SIMDType::XMVECTOR;
		LOG_WARNING("Using AVX128 Instruction Set...!");
	}

#endif
}

//-------------------------------------------------------------------------------------------------------------------
inline void Scene::Initialize(uint16_t nSamples)
{
	LOG_WARNING("Using {0} samples for Ray Tracing!", nSamples);

	m_uiSamples = nSamples;
	m_pWorld = BasicScene();

	DetectSIMD();

	FastVector lookFrom(0.0f, 1.5f, 6.0f);
	FastVector lookAt(0, 0, 0);
	FastVector Up(0, 1, 0);
	constexpr float dist_to_focus = 1.0f;	// set this to 1.0 & apertue to 0.0f to stop DOF effect!
	constexpr float aperture = 0.0f;
	const float aspect = static_cast<float>(UT::GLOBALS::GWindowWidth) / UT::GLOBALS::GWindowHeight;

	m_pCamera = new Camera(lookFrom, lookAt, Up, 25.0f, aspect, aperture, dist_to_focus);
}

//-------------------------------------------------------------------------------------------------------------------
inline FastVector Scene::Render(uint16_t xPixel, uint16_t yPixel)
{
	const float u = (xPixel + Helper::GetRandom01()) / UT::GLOBALS::GWindowWidth;
	const float v = (yPixel + Helper::GetRandom01()) / UT::GLOBALS::GWindowHeight;

	const Ray r = m_pCamera->get_ray(u, v);

	// Convert XMFLOAT3 to XMVECTOR for computation
	FastVector colorVec(0, 0, 0);
	const FastVector tracedColor = Trace(r, m_pWorld, 0);

	// Perform vector addition
	colorVec += tracedColor;

	float ir = 255.0f * colorVec.GetX();
	float ig = 255.0f * colorVec.GetY();
	float ib = 255.0f * colorVec.GetZ();

	return { ir, ig, ib };
}

//-------------------------------------------------------------------------------------------------------------------
inline Hitable* Scene::BasicScene()
{
	Hitable** list = new Hitable * [5];
	list[0] = new Sphere(FastVector(1.05f, 0, 0), 0.5, new Metal(FastVector(0.5f, 0.2f, 0.1f), 0.5));
	list[1] = new Sphere(FastVector(0, -100.5f, 0), 100, new Lambertian(FastVector(0.2f, 0.2f, 0.2f)));
	list[2] = new Sphere(FastVector(0, 0, 0.1f), 0.5, new Transparent(1.5f));
	list[3] = new Sphere(FastVector(-1.05f, 0, 0), 0.5, new Metal(FastVector(1.0, 0.2f, 0.0), 0));
	list[4] = new Sphere(FastVector(0.0f, 0, -3), 0.5, new Lambertian(FastVector(1.0f, 1.0f, 0.0f)));

	return new HitableList(list, 5);
}

//-------------------------------------------------------------------------------------------------------------------
inline Hitable* Scene::RandomScene()
{
	const int n = 500;
	Hitable** list = new Hitable * [n + 1];
	list[0] = new Sphere(XMVectorSet(0, -1000, 0, 0), 1000, new Lambertian(FastVector(0.5, 0.5, 0.5)));
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
					list[i++] = new Sphere(center, 0.2f, new Lambertian(FastVector(Helper::GetRandom01() * Helper::GetRandom01(), Helper::GetRandom01() * Helper::GetRandom01(), Helper::GetRandom01() * Helper::GetRandom01())));
				}
				else if (choose_mat < 0.95)
				{
					// Metal
					list[i++] = new Sphere(center, 0.2f, new Metal(FastVector(0.5f * (1 + Helper::GetRandom01()), 0.5f * (1 + Helper::GetRandom01()), 0.5f * (1 + Helper::GetRandom01())), Helper::GetRandom01()));
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
	list[i++] = new Sphere(XMVectorSet(-4, 1, 0, 0), 1.0f, new Lambertian(FastVector(0.4f, 0.2f, 0.1f)));
	list[i++] = new Sphere(XMVectorSet(4, 1, 0, 0), 1.0f, new Metal(FastVector(0.7f, 0.6f, 0.5f), 0.0f));

	return new HitableList(list, i);
}

//-------------------------------------------------------------------------------------------------------------------
inline FastVector Scene::Trace(const Ray& r, Hitable* world, int depth)
{
	HitRecord rec;

	if (m_pWorld->hit(r, 0.001f, FLT_MAX, rec))
	{
		Ray scatteredRay;
		FastVector attenuation;

		if (depth < 50 && rec.mat_ptr->Scatter(r, rec, attenuation, scatteredRay))
		{
			return attenuation * Trace(scatteredRay, world, depth + 1);
		}
		else
		{
			return FastVector{ 0, 0, 0 };
		}
	}
	else
	{
		const FastVector unit_direction = r.GetRayDirection().UnitVector();
		const float t = 0.5f * unit_direction.GetY() + 1.0f;

		return Helper::LerpVector(FastVector(1, 1, 1), FastVector(0.5f, 0.7f, 1.0f), t);
	}
}

