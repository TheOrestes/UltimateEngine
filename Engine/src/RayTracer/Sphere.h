#pragma once

#include "Hitable.h"

class Material;

class Sphere : public Hitable
{
public:
	Sphere() {}
	Sphere(XMVECTOR _center, float _r, Material* ptr_mat) :
		center(_center),
		radius(_r),
		mat_ptr(ptr_mat) {};

	virtual bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const;

private:
	XMVECTOR	center;
	float		radius;
	Material*	mat_ptr;
};

/////////////////////////////////////////////////////////////////////////////////////////
bool Sphere::hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const
{
	const XMVECTOR oc = XMVectorSubtract(r.GetRayOrigin(), center);
	const float a = XMVectorGetX(XMVector3Dot(r.GetRayDirection(), r.GetRayDirection()));
	const float b = 2.0f * XMVectorGetX(XMVector3Dot(oc, r.GetRayDirection()));
	const float c = XMVectorGetX(XMVector3Dot(oc, oc)) - radius * radius;
	const float discriminant = b * b - 4 * a* c;

	float t;

	if (discriminant > 0)
	{
		t = (-b - sqrt(discriminant)) / (2.0 * a);
		if (t < tmax && t > tmin)
		{
			rec.t = t;
			rec.P = r.GetPointAt(t);
			rec.N = XMVectorScale(XMVectorSubtract(rec.P, center), 1.0f / radius);
			rec.mat_ptr = mat_ptr;
			return true;
		}

		t = (-b + sqrt(discriminant)) / (2.0 * a);
		if (t < tmax && t > tmin)
		{
			rec.t = t;
			rec.P = r.GetPointAt(t);
			rec.N = XMVectorScale(XMVectorSubtract(rec.P, center), 1.0f / radius);
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}

	return false;
}