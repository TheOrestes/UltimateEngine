#pragma once

#include <DirectXMath.h>
using namespace DirectX;

template <typename T> class FastVector;

// Specialization for XMVECTOR

template<> 
class FastVector<XMVECTOR>
{
public:
	FastVector() { data = XMVectorSet(0, 0, 0, 0); }
	FastVector(float x, float y, float z, float w = 0.0f) { data = XMVectorSet(x, y, z, w); }
	FastVector(XMVECTOR v) : data(v) {}

	inline float operator[](int i) const;

	friend FastVector operator+(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator-(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator-(const FastVector& vec);
	friend FastVector operator*(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator/(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator*(const FastVector& vec, const float value);
	friend FastVector operator*(const float value, const FastVector& vec);
	friend FastVector operator/(const FastVector& vec, const float value);

	FastVector& operator+=(const FastVector& v2);
	FastVector& operator-=(const FastVector& v2);
	FastVector& operator*=(const FastVector& v2);
	FastVector& operator/=(const FastVector& v2);
	FastVector& operator*=(const float value);
	FastVector& operator/=(const float value);

	[[nodiscard]] float Length() const;
	[[nodiscard]] float LengthSquared() const;

private:
	XMVECTOR data;
};

