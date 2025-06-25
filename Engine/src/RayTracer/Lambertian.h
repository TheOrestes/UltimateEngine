#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "Material.h"
#include "Helper.h"
#include "FastVector.h"

class Lambertian : public Material
{
public:
	Lambertian(const FastVector& _albedo) : Albedo(_albedo) {}

	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, FastVector& attenuation, Ray& scattered) const
	{
        const FastVector randomDir = Helper::RandomInUnitSphere();

        // Compute scattering target using SIMD-friendly operations
        const FastVector target = (rec.P + rec.N) + randomDir;
        scattered = Ray(rec.P, target - rec.P);

        // Store albedo as attenuation
        attenuation = Albedo;

        return true;
	}

private:
	FastVector Albedo;
};