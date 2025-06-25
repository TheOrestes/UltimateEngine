#pragma once

#include "Ray.h"
#include "Hitable.h"
#include "Helper.h"
#include "Material.h"

class Transparent : public Material
{
public:
	Transparent(float ri) : refr_index(ri) {}

	bool Scatter(const Ray& r_in, const HitRecord& rec, FastVector& attenuation, Ray& scattered) const override
	{
		FastVector outward_normal;
		const FastVector ray_direction = r_in.GetRayDirection();
		
		const FastVector reflected = Helper::Reflect(ray_direction, rec.N);
		float ni_over_nt;
		attenuation = FastVector(1, 1, 1);

		FastVector refracted;
		float reflect_prob;
		float cosine;

		if ((Dot(ray_direction, rec.N) > 0))
		{
			outward_normal = -1 * rec.N;  // because we want inverted image for refraction? 
			ni_over_nt = refr_index;
			cosine = refr_index * Dot(ray_direction, rec.N) / ray_direction.Length();
		}
		else
		{
			outward_normal = rec.N;
			ni_over_nt = 1 / refr_index;
			cosine = -Dot(ray_direction, rec.N) / ray_direction.Length();
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