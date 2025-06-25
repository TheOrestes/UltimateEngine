#pragma once

#include "Ray.h"
#include "Helper.h"
#include "FastVector.h"

class Camera
{
public:
	Camera(const FastVector& lookFrom, const FastVector& lookAt, const FastVector& Up, float vfov, float aspect, float aperture, float focus_dist)	// vofv is vertical fov
	{
		lens_radius = aperture / 2.0f;

		constexpr float DegToRad = 0.017453292519943295f; 
		const float theta = vfov * DegToRad;

		const float half_height = tan(theta / 2);
		const float half_width = aspect * half_height;

		origin = lookFrom;
		w = (lookFrom - lookAt).UnitVector();
		u = Cross(Up, w).UnitVector(); 
		v = Cross(w, u); 

		lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
		horizontal = 2 * half_width * focus_dist * u;
		vertical = 2 * half_height * focus_dist * v;
	}

	Ray get_ray(float s, float t)
	{
		const FastVector rd = Helper::GetRandomInUnitDisk() * lens_radius;

		const FastVector term1 = u * rd.GetX(); 
		const FastVector term2 = v * rd.GetY();
		const FastVector offset = term1 + term2;

		return Ray(origin + offset, lower_left_corner + s * horizontal + (1.0f-t) * vertical - origin - offset);
	}

private:
	FastVector origin;
	FastVector lower_left_corner;
	FastVector horizontal;
	FastVector vertical;
	FastVector u, v, w;
	float lens_radius;
};
