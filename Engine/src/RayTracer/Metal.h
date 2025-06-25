#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "Material.h"
#include "Helper.h"

class Metal : public Material
{
public:
	Metal (const FastVector& _albedo, float f) : Albedo(_albedo) 
	{
		if (f < 1)
			fuzz = f;
		else
			fuzz = 1;
	}

	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, FastVector& attenuation, Ray& scatterd) const
	{
		const FastVector target = Helper::Reflect((r_in.GetRayDirection().UnitVector()), rec.N);
		const FastVector direction = target + Helper::RandomInUnitSphere() * fuzz;
		scatterd = Ray(rec.P, direction);
		attenuation = Albedo;
		return (Dot(scatterd.GetRayDirection(), rec.N) > 0);
	}

private:
	FastVector Albedo;
	float fuzz;
};
