#pragma once

#include <DirectXMath.h>
#include <immintrin.h>

#include "D3D12Renderer/D3DGlobals.h"
using namespace DirectX;
using namespace UT::GLOBALS;

//---------------------------------------------------------------------------------------------------------------------
struct FastVector
{
	FastVector() { data.xm = XMVectorZero(); }
	FastVector(XMVECTOR v)	{ data.xm = v; }
	FastVector(__m256 v)	{ data.avx2 = v; }
	FastVector(__m512 v)	{ data.avx512 = v; }
	FastVector(float x, float y, float z);

	union Data
	{
		XMVECTOR	xm;
		__m256		avx2;
		__m512		avx512;

		Data() : xm(XMVectorZero())	{}
		~Data()	{}
	} data;

	float operator[](int index) const;
	friend FastVector operator+(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator-(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator-(const FastVector& vec);
	friend FastVector operator*(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator/(const FastVector& lhs, const FastVector& rhs);
	friend FastVector operator*(const FastVector& vec, const float value);
	friend FastVector operator*(const float value, const FastVector& vec);
	friend FastVector operator/(const FastVector& vec, const float value);

	friend float Dot(const FastVector& lhs, const FastVector& rhs);
	friend FastVector Cross(const FastVector& lhs, const FastVector& rhs);
	
	FastVector& operator+=(const FastVector& v2);
	FastVector& operator-=(const FastVector& v2);
	FastVector& operator*=(const FastVector& v2);
	FastVector& operator/=(const FastVector& v2);
	FastVector& operator*=(const float value);
	FastVector& operator/=(const float value);

	[[nodiscard]] float Length() const;
	[[nodiscard]] float LengthSquared() const;

	FastVector UnitVector() const;
	[[nodiscard]] float GetX() const;
	[[nodiscard]] float GetY() const;
	[[nodiscard]] float GetZ() const;
};
