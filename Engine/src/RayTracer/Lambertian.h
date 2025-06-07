#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "Material.h"
#include "Helper.h"

class Lambertian : public Material
{
public:
	Lambertian(const XMFLOAT3& _albedo) : Albedo(_albedo) {}

	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, XMFLOAT3& attenuation, Ray& scattered) const
	{
        const XMVECTOR randomDir = Helper::RandomInUnitSphere();

        // Compute scattering target using SIMD-friendly operations
        const XMVECTOR target = XMVectorAdd(XMVectorAdd(rec.P, rec.N), randomDir);
        scattered = Ray(rec.P, XMVectorSubtract(target, rec.P));

        // Store albedo as attenuation
        attenuation = Albedo;

        return true;
	}

private:
	XMFLOAT3 Albedo;
};