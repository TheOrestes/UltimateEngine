#pragma once

class Ray
{
public:
	Ray() {}
	Ray(const XMVECTOR& A, const XMVECTOR& B) 
	{ 
		origin = A;
		direction = B; 
	}

	XMVECTOR GetRayOrigin()			const { return origin; }
	XMVECTOR GetRayDirection()		const { return direction; }
	XMVECTOR GetPointAt(float t)	const { return XMVectorAdd(origin, XMVectorScale(direction, t)); }

private:
	XMVECTOR origin;
	XMVECTOR direction;
};