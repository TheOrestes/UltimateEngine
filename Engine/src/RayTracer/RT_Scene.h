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
class RT_Scene
{
public:
	RT_Scene() : m_uiSamples(0), m_uiCurrentSampleIndex(0), m_pCamera(nullptr), m_pWorld(nullptr) {} 
	~RT_Scene() {
		SAFE_DELETE(m_pCamera);
		SAFE_DELETE(m_pWorld);
	}

	void			Initialize(uint16_t nSamples);
	Vector3			Render(uint16_t xPixel, uint16_t yPixel);
	void			IncrementSampleIndex() { ++m_uiCurrentSampleIndex; }

	uint16_t		GetCurrentSampleIndex() const { return m_uiCurrentSampleIndex;}
	uint16_t		GetSampleCount()		const { return m_uiSamples; }

private:
	Hitable*		RandomScene();
	Hitable*		BasicScene();
	Vector3			Trace(const Ray& r, Hitable* world, int depth);

	uint16_t		m_uiSamples;
	uint16_t		m_uiCurrentSampleIndex;
	Camera*			m_pCamera;
	Hitable*		m_pWorld;
};

//-------------------------------------------------------------------------------------------------------------------
inline void RT_Scene::Initialize(uint16_t nSamples)
{
	m_uiSamples = nSamples;
	m_pWorld = BasicScene();

	const Vector3 lookFrom(0, 1.5, 6);
	const Vector3 lookAt(0, 0, 0);
	constexpr float dist_to_focus = 1.0f;	// set this to 1.0 & apertue to 0.0f to stop DOF effect!
	constexpr float aperture = 0.0f;

	m_pCamera = new Camera(lookFrom, lookAt, Vector3(0, 1, 0), 20, UT::GLOBALS::GWindowWidth / UT::GLOBALS::GWindowHeight, aperture, dist_to_focus);
}

//-------------------------------------------------------------------------------------------------------------------
inline Vector3 RT_Scene::Render(uint16_t xPixel, uint16_t yPixel)
{
	Vector3 color(0, 0, 0);

	float u = float(xPixel + Helper::GetRandom01()) / UT::GLOBALS::GWindowWidth;
	float v = float(yPixel + Helper::GetRandom01()) / UT::GLOBALS::GWindowHeight;

	Ray r = m_pCamera->get_ray(u, v);

	color = color + Trace(r, m_pWorld, 0);

	float ir = (255.99 * color.x);
	float ig = (255.99 * color.y);
	float ib = (255.99 * color.z);

	return Vector3(ir, ig, ib);
}

//-------------------------------------------------------------------------------------------------------------------
inline Hitable* RT_Scene::BasicScene()
{
	Hitable** list = new Hitable * [5];
	list[0] = new Sphere(Vector3(1.05f, 0, 0), 0.5, new Metal(Vector3(0.5f, 0.2f, 0.1f), 0.5));
	list[1] = new Sphere(Vector3(0, -100.5f, 0), 100, new Lambertian(Vector3(0.2f, 0.2f, 0.2f)));
	list[2] = new Sphere(Vector3(0, 0, 0.1f), 0.5, new Transparent(1.5f));
	list[3] = new Sphere(Vector3(-1.05f, 0, 0), 0.5, new Metal(Vector3(1.0, 0.2f, 0.0), 0));
	list[4] = new Sphere(Vector3(0.0f, 0, -3), 0.5, new Lambertian(Vector3(1.0f, 1.0f, 0.0f)));

	return new HitableList(list, 5);
}

//-------------------------------------------------------------------------------------------------------------------
inline Hitable* RT_Scene::RandomScene()
{
	const int n = 500;
	Hitable** list = new Hitable * [n + 1];
	list[0] = new Sphere(Vector3(0, -1000, 0), 1000, new Lambertian(Vector3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			const float choose_mat = Helper::GetRandom01();
			Vector3 center(a + 0.9f * Helper::GetRandom01(), 0.2, b + 0.9 * Helper::GetRandom01());
			if ((center - Vector3(4, 0.2, 0)).length() > 0.9f)
			{
				if (choose_mat < 0.8f)
				{
					// diffuse
					list[i++] = new Sphere(center, 0.2f, new Lambertian(Vector3(Helper::GetRandom01() * Helper::GetRandom01(), Helper::GetRandom01() * Helper::GetRandom01(), Helper::GetRandom01() * Helper::GetRandom01())));
				}
				else if (choose_mat < 0.95)
				{
					// Metal
					list[i++] = new Sphere(center, 0.2f, new Metal(Vector3(0.5f * (1 + Helper::GetRandom01()), 0.5f * (1 + Helper::GetRandom01()), 0.5f * (1 + Helper::GetRandom01())), Helper::GetRandom01()));
				}
				else
				{
					// glass
					list[i++] = new Sphere(center, 0.2f, new Transparent(1.5f));
				}
			}
		}
	}

	list[i++] = new Sphere(Vector3(0, 1, 0), 1.0f, new Transparent(1.5f));
	list[i++] = new Sphere(Vector3(-4, 1, 0), 1.0f, new Lambertian(Vector3(0.4f, 0.2f, 0.1f)));
	list[i++] = new Sphere(Vector3(4, 1, 0), 1.0f, new Metal(Vector3(0.7f, 0.6f, 0.5f), 0.0f));

	return new HitableList(list, i);
}

//-------------------------------------------------------------------------------------------------------------------
inline Vector3 RT_Scene::Trace(const Ray& r, Hitable* world, int depth)
{
	HitRecord rec;

	if (m_pWorld->hit(r, 0.001f, FLT_MAX, rec))
	{
		Ray scatteredRay;
		Vector3 attenuation;

		if (depth < 50 && rec.mat_ptr->Scatter(r, rec, attenuation, scatteredRay))
		{
			return attenuation * Trace(scatteredRay, world, depth + 1);
		}
		else
		{
			return Vector3(0, 0, 0);
		}
	}
	else
	{
		const Vector3 unit_direction = unit_vector(r.GetRayDirection());
		const float t = 0.5f * (unit_direction.y + 1.0f);
		return Helper::LerpVector(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.5f, 0.7f, 1.0f), t);
	}
}

