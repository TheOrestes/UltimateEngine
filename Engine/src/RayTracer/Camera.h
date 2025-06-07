#pragma once

#include "Ray.h"
#include "Helper.h"

class Camera
{
public:
	Camera(FXMVECTOR lookFrom, FXMVECTOR lookAt, FXMVECTOR Up, float vfov, float aspect, float aperture, float focus_dist)	// vofv is vertical fov
	{
		lens_radius = aperture / 2.0f;

		float theta = XMConvertToRadians(vfov);
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;

		origin = lookFrom;
		w = XMVector3Normalize(XMVectorSubtract(lookFrom, lookAt));
		u = XMVector3Normalize(XMVector3Cross(Up, w));
		v = XMVector3Cross(w, u);

		lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
		horizontal = 2 * half_width * focus_dist * u;
		vertical = 2 * half_height * focus_dist * v;
	}

	Ray get_ray(float s, float t)
	{
		const XMVECTOR rd = XMVectorScale(Helper::GetRandomInUnitDisk(), lens_radius);

		const XMVECTOR term1 = XMVectorScale(u, XMVectorGetX(rd));
		const XMVECTOR term2 = XMVectorScale(v, XMVectorGetY(rd));
		const XMVECTOR offset = XMVectorAdd(term1, term2);

		return Ray(XMVectorAdd(origin, offset), lower_left_corner + s * horizontal + (1.0f-t) * vertical - origin - offset);
	}

private:
	XMVECTOR origin;
	XMVECTOR lower_left_corner;
	XMVECTOR horizontal;
	XMVECTOR vertical;
	XMVECTOR u, v, w;
	float lens_radius;
};
