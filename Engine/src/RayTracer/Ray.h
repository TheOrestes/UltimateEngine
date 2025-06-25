#pragma once

#include "FastVector.h"

class Ray
{
public:
	Ray() {}
	Ray(const FastVector& A, const FastVector& B) 
	{ 
		origin = A;
		direction = B; 
	}

	inline FastVector GetRayOrigin()		const { return origin; }
	inline FastVector GetRayDirection()		const { return direction; }
	inline FastVector GetPointAt(float t)	const { return (origin + (direction*t)); }

private:
	FastVector origin;
	FastVector direction;
};