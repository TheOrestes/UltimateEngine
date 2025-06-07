#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "Material.h"
#include "Helper.h"

class Metal : public Material
{
public:
	Metal (const XMFLOAT3& _albedo, float f) : Albedo(_albedo) 
	{
		if (f < 1)
			fuzz = f;
		else
			fuzz = 1;
	}

	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, XMFLOAT3& attenuation, Ray& scatterd) const
	{
		const XMVECTOR target = Helper::Reflect(XMVector3Normalize(r_in.GetRayDirection()), rec.N);
		const XMVECTOR direction = XMVectorAdd(target, XMVectorScale(Helper::RandomInUnitSphere(), fuzz));
		scatterd = Ray(rec.P, direction);
		attenuation = Albedo;
		return (XMVectorGetX(XMVector3Dot(scatterd.GetRayDirection(), rec.N)) > 0);
	}

private:
	XMFLOAT3 Albedo;
	float fuzz;
};
