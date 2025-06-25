#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "FastVector.h"

class Material
{
public:
	virtual ~Material() = default;
	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, FastVector& attenuation, Ray& scattered) const = 0;
};
