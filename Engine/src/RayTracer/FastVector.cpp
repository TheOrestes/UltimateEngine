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
	float temp[8];
	_mm256_store_ps(temp, data.avx2);
	return temp[index];

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
	result.data.avx2 = _mm256_add_ps(lhs.data.avx2, rhs.data.avx2);

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
	result = _mm256_sub_ps(lhs.data.avx2, rhs.data.avx2);

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
	const __m256 signMask = _mm256_set1_ps(-0.0f);
	result = _mm256_xor_ps(vec.data.avx2, signMask);

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
	result = _mm256_mul_ps(lhs.data.avx2, rhs.data.avx2);

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
	result.data.avx2 = _mm256_div_ps(lhs.data.avx2, rhs.data.avx2);

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
	const __m256 scale = _mm256_set1_ps(value);
	result = _mm256_mul_ps(vec.data.avx2, scale);

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
	const __m256 scale = _mm256_set1_ps(value);
	result = _mm256_mul_ps(vec.data.avx2, scale);

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
	const __m256 scale = _mm256_set1_ps(1.0f / value);
	result = _mm256_mul_ps(vec.data.avx2, scale);

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
	// Square of each element
	const __m256 mul = _mm256_mul_ps(lhs.data.avx2, rhs.data.avx2);

	// Split into two 128-bit halves
	const __m128 low = _mm256_castps256_ps128(mul);		// 0-3
	const __m128 high = _mm256_extractf128_ps(mul, 1);	// 4-7

	// Add the two halves
	__m128 sum = _mm_add_ps(low, high);

	// Horizontal add to reduce to scalar
	sum = _mm_hadd_ps(sum, sum);	// [a+b, c+d, a+b, c+d]
	sum = _mm_hadd_ps(sum, sum);	// [a+b+c+d, a+b+c+d, a+b+c+d, a+b+c+d]

	// Extract scalar & take the square root
	dot = _mm_cvtss_f32(sum);

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
	const __m256 a = lhs.data.avx2;
	const __m256 b = rhs.data.avx2;

	// Shuffle to get yzx order
	const __m256 a_yzx = _mm256_permute_ps(a, _MM_SHUFFLE(3, 0, 2, 1));
	const __m256 b_yzx = _mm256_permute_ps(b, _MM_SHUFFLE(3, 0, 2, 1));

	// Multiply components
	const __m256 mul1 = _mm256_mul_ps(a, b_yzx);
	const __m256 mul2 = _mm256_mul_ps(a_yzx, b);

	// Subtract to get cross product
	const __m256 cross = _mm256_sub_ps(mul1, mul2);

	// Shuffle back to xyz order
	result.data.avx2 = _mm256_permute_ps(cross, _MM_SHUFFLE(3, 0, 2, 1));

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
	this->data.avx2 = _mm256_add_ps(this->data.avx2, v2.data.avx2);


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
	this->data.avx2 = _mm256_sub_ps(this->data.avx2, v2.data.avx2);

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
	this->data.avx2 = _mm256_mul_ps(this->data.avx2, v2.data.avx2);

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
	__m256 reciprocal = _mm256_rcp_ps(v2.data.avx2);
	this->data.avx2 = _mm256_mul_ps(this->data.avx2, v2.data.avx2);

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
	const __m256 scale = _mm256_set1_ps(value);
	this->data.avx2 = _mm256_mul_ps(this->data.avx2, scale);

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
	const __m256 scale = _mm256_set1_ps(value);
	this->data.avx2 = _mm256_div_ps(this->data.avx2, scale);

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
	// Square of each element
	const __m256 squared = _mm256_mul_ps(this->data.avx2, this->data.avx2);

	// Split into two 128-bit halves
	const __m128 low = _mm256_castps256_ps128(squared);		// 0-3
	const __m128 high = _mm256_extractf128_ps(squared, 1);	// 4-7

	// Add the two halves
	__m128 sum = _mm_add_ps(low, high);

	// Horizontal add to reduce to scalar
	sum = _mm_hadd_ps(sum, sum);	// [a+b, c+d, a+b, c+d]
	sum = _mm_hadd_ps(sum, sum);	// [a+b+c+d, a+b+c+d, a+b+c+d, a+b+c+d]

	// Extract scalar & take the square root
	const float dot = _mm_cvtss_f32(sum);
	length = sqrtf(dot);

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
	// Square of each element
	const __m256 squared = _mm256_mul_ps(this->data.avx2, this->data.avx2);

	// Split into two 128-bit halves
	const __m128 low = _mm256_castps256_ps128(squared);		// 0-3
	const __m128 high = _mm256_extractf128_ps(squared, 1);	// 4-7

	// Add the two halves
	__m128 sum = _mm_add_ps(low, high);

	// Horizontal add to reduce to scalar
	sum = _mm_hadd_ps(sum, sum);	// [a+b, c+d, a+b, c+d]
	sum = _mm_hadd_ps(sum, sum);	// [a+b+c+d, a+b+c+d, a+b+c+d, a+b+c+d]

	// Extract scalar & take the square root
	lengthSq = _mm_cvtss_f32(sum);

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
	const float length = this->Length();
	if(length > 0.0f)
	result = this->data.avx2 / length;

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
	alignas(32) float temp[8];
	_mm256_store_ps(temp, this->data.avx2);
	value = temp[0];

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
	alignas(32) float temp[8];
	_mm256_store_ps(temp, this->data.avx2);
	value = temp[1];

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
	alignas(32) float temp[8];
	_mm256_store_ps(temp, this->data.avx2);
	value = temp[2];

#elif defined ENABLE_AVX128
	value = XMVectorGetZ(this->data.xm);

#endif

	return value;
}


