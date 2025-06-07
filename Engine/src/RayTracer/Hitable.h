#pragma once

#include "Ray.h"

class Material;

struct HitRecord
{
	float t;
	XMVECTOR P;
	XMVECTOR N;
	Material* mat_ptr;
};

class Hitable
{
public:
	virtual ~Hitable() {}
	virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const = 0;
};