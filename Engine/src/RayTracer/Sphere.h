#pragma once

#include "Hitable.h"

class Material;

class Sphere : public Hitable
{
public:
	Sphere(): radius(0), mat_ptr(nullptr){}
	Sphere(FastVector _center, float _r, Material* ptr_mat) :
		center(std::move(_center)),
		radius(_r),
		mat_ptr(ptr_mat) {};

	bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const override;

private:
	FastVector	center;
	float		radius;
	Material*	mat_ptr;
};

/////////////////////////////////////////////////////////////////////////////////////////
inline bool Sphere::hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const
{
	const FastVector oc = r.GetRayOrigin() - center;
	const float a = Dot(r.GetRayDirection(), r.GetRayDirection());
	const float b = 2.0f * Dot(oc, r.GetRayDirection());
	const float c = Dot(oc, oc) - radius * radius;
	const float discriminant = b * b - 4 * a* c;
	const float root_discr = sqrt(discriminant);
	const float invDenom = 0.5f * a;

	float t = 0.0f;

	if (discriminant > 0)
	{
		t = (-b - root_discr) / (2.0 * a);
		if (t < tmax && t > tmin)
		{
			rec.t = t;
			rec.P = r.GetPointAt(t);
			rec.N = (rec.P - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}

		t = (-b + root_discr) / (2.0 * a);
		if (t < tmax && t > tmin)
		{
			rec.t = t;
			rec.P = r.GetPointAt(t);
			rec.N = (rec.P - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}

	return false;
}