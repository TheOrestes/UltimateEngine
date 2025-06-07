#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "Helper.h"
#include "Material.h"

class Transparent : public Material
{
public:
	Transparent(float ri) : refr_index(ri) {}

	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, XMFLOAT3& attenuation, Ray& scattered) const
	{
		XMVECTOR outward_normal;
		const XMVECTOR ray_direction = r_in.GetRayDirection();
		
		const XMVECTOR reflected = Helper::Reflect(ray_direction, rec.N);
		float ni_over_nt;
		attenuation = XMFLOAT3(1, 1, 1);

		XMVECTOR refracted;
		float reflect_prob;
		float cosine;

		if (XMVectorGetX(XMVector3Dot(ray_direction, rec.N)) > 0)
		{
			outward_normal = -1 * rec.N;  // because we want inverted image for refraction? 
			ni_over_nt = refr_index;
			cosine = refr_index * XMVectorGetX(XMVector3Dot(ray_direction, rec.N)) / XMVectorGetX(XMVector3Length(ray_direction));
		}
		else
		{
			outward_normal = rec.N;
			ni_over_nt = 1 / refr_index;
			cosine = -XMVectorGetX(XMVector3Dot(ray_direction, rec.N)) / XMVectorGetX(XMVector3Length(ray_direction));
		}

		if (Helper::Refract(ray_direction, outward_normal, ni_over_nt, refracted))
		{
			reflect_prob = Helper::schlick(cosine, refr_index);
		}
		else
		{
			reflect_prob = 1.0f;
		}

		// this logic is not clear? 
		if (Helper::GetRandom01() < reflect_prob)
		{
			scattered = Ray(rec.P, reflected);
		}
		else
		{
			scattered = Ray(rec.P, refracted);
		}

		return true;
	}

private:
	float refr_index;
};