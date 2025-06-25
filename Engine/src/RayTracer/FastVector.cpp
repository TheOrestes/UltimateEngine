#include "UltimateEnginePCH.h"
#include "FastVector.h"

//---------------------------------------------------------------------------------------------------------------------
FastVector::FastVector(float x, float y, float z)
{
#if defined ENABLE_AVX512
	const float temp[16] = { x, y, z, 0.0f };
	data.avx512 = _mm512_loadu_ps(temp);

#elif defined ENABLE_AVX256
	const float temp[8] = { x, y, z, 0.0f, 0, 0, 0, 0 };
	data.avx2 = _mm256_loadu_ps(temp);

#elif defined ENABLE_AVX128
	data.xm = XMVectorSet(x, y, z, 0.0f);

#endif
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector::operator[](int index) const
{
	assert(index >= 0 && index < 4);

	float value = 0.0f;

#if defined ENABLE_AVX512
	

#elif defined ENABLE_AVX256
	

#elif defined ENABLE_AVX128
	value = XMVectorGetByIndex(this->data.xm, index);

#endif

	return value;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator+(const FastVector& lhs, const FastVector& rhs) 
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorAdd(lhs.data.xm, rhs.data.xm);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator-(const FastVector& lhs, const FastVector& rhs)
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorSubtract(lhs.data.xm, rhs.data.xm);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator-(const FastVector& vec)
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorNegate(vec.data.xm);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator*(const FastVector& lhs, const FastVector& rhs) 
{
	FastVector result;
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorMultiply(lhs.data.xm, rhs.data.xm);

#endif
	
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator/(const FastVector& lhs, const FastVector& rhs) 
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorDivide(lhs.data.xm, rhs.data.xm);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator*(const FastVector& vec, const float value)
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorScale(vec.data.xm, value);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator*(const float value, const FastVector& vec) 
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorScale(vec.data.xm, value);

#endif
	
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector operator/(const FastVector& vec, const float value) 
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVectorScale(vec.data.xm, 1.0f / value);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
float Dot(const FastVector& lhs, const FastVector& rhs) 
{
	float dot = 0.0f;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	dot = XMVectorGetX(XMVector3Dot(lhs.data.xm, rhs.data.xm));

#endif

	return dot;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector Cross(const FastVector& lhs, const FastVector& rhs) 
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVector3Cross(lhs.data.xm, rhs.data.xm);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector& FastVector::operator+=(const FastVector& v2)
{
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	this->data.xm = XMVectorAdd(this->data.xm, v2.data.xm);

#endif

	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector& FastVector::operator-=(const FastVector& v2)
{
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	this->data.xm = XMVectorSubtract(this->data.xm, v2.data.xm);

#endif
	
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector& FastVector::operator*=(const FastVector& v2)
{
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	this->data.xm = XMVectorMultiply(this->data.xm, v2.data.xm);

#endif

	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector& FastVector::operator/=(const FastVector& v2)
{
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	this->data.xm = XMVectorDivide(this->data.xm, v2.data.xm);

#endif
	
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector& FastVector::operator*=(const float value)
{
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	this->data.xm = XMVectorScale(this->data.xm, value);

#endif
	
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector& FastVector::operator/=(const float value)
{
#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	this->data.xm = XMVectorScale(this->data.xm, 1.0f / value);

#endif
	
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector::Length() const
{
	float length = 0.0f;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	length = XMVectorGetX(XMVector3Length(this->data.xm));

#endif

	return length;
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector::LengthSquared() const
{
	float lengthSq = 0.0f;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	lengthSq = XMVectorGetX(XMVector3LengthSq(this->data.xm));

#endif

	return lengthSq;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector FastVector::UnitVector() const
{
	FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	result.data.xm = XMVector3Normalize(this->data.xm);

#endif

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector::GetX() const
{
	float value = 0.0f;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	value = XMVectorGetX(this->data.xm);

#endif

	return value;
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector::GetY() const
{
	float value = 0.0f;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	value = XMVectorGetY(this->data.xm);

#endif

	return value;
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector::GetZ() const
{
	float value = 0.0f;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256


#elif defined ENABLE_AVX128
	value = XMVectorGetZ(this->data.xm);

#endif

	return value;
}


