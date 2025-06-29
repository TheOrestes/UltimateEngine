#pragma once

#include "RayTracer/FastVector.h"
#include "D3DGlobals.h"

const float PI = 3.14159265358f;

namespace Helper
{
	//-------------------------------------------------------------------------------------------------------------------
	inline FastVector LerpVector(const FastVector& vec1, const FastVector& vec2, float t)
	{
		FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256
		result = vec1 + (vec2 - vec1) * t;

#elif defined ENABLE_AVX128
		result = XMVectorLerp(vec1.data.xm, vec2.data.xm, t);

#endif

		return result;
	}

	//-------------------------------------------------------------------------------------------------------------------
	inline float GetRandom01()
	{
		return static_cast<float>(rand()) / RAND_MAX;
	}

	//-------------------------------------------------------------------------------------------------------------------
	inline FastVector GetRandomInUnitDisk()
	{
		FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256
		FastVector p;
		do
		{
			const FastVector randomVec = FastVector(GetRandom01(), GetRandom01(), 0.0f);
			const FastVector unitVec = FastVector(1, 1, 0);

			p = (randomVec * 2.0f) - unitVec;
		} while ((Dot(p, p)) >= 1.0f);

		result.data.avx2 = p.data.avx2;

#elif defined ENABLE_AVX128
		XMVECTOR p;
		do
		{
			const XMVECTOR randomVec = XMVectorSet(GetRandom01(), GetRandom01(), 0.0f, 0.0f);
			const XMVECTOR unitVec = XMVectorSet(1, 1, 0, 0);

			p = XMVectorSubtract(XMVectorScale(randomVec, 2.0f), unitVec);

		} while (XMVectorGetX(XMVector3Dot(p, p)) >= 1.0f);

		result.data.xm = p;

#endif
	
		return result;
	}

	//-------------------------------------------------------------------------------------------------------------------
	inline FastVector RandomInUnitSphere()
	{
		FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256
		FastVector p;
		do
		{
			const FastVector randomVec = FastVector(GetRandom01(), GetRandom01(), GetRandom01());
			const FastVector unitVec = FastVector(1, 1, 1);

			p = (randomVec * 2.0f) - unitVec;
		} while ((p.LengthSquared()) >= 1.0f);

		result.data.avx2 = p.data.avx2;

#elif defined ENABLE_AVX128
		XMVECTOR p;
		do
		{
			const XMVECTOR randomVec = XMVectorSet(GetRandom01(), GetRandom01(), GetRandom01(), 0.0f);
			const XMVECTOR unitVec = XMVectorSet(1, 1, 1, 0);

			p = XMVectorSubtract(XMVectorScale(randomVec, 2.0f), unitVec);

		} while (XMVectorGetX(XMVector3LengthSq(p)) >= 1.0f);

		result.data.xm = p;

#endif

		return result;
	}

	//-------------------------------------------------------------------------------------------------------------------
	inline FastVector Reflect(const FastVector& v, const FastVector& n)
	{
		FastVector result;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256
		const float dot = Dot(v, n);
		result = v - n * (2.0f * dot);

#elif defined ENABLE_AVX128
		result.data.xm = XMVector3Reflect(v.data.xm, n.data.xm);

#endif

		return result;
	}

	//-------------------------------------------------------------------------------------------------------------------
	inline bool Refract(const FastVector& v, const FastVector& n, float ni_over_nt, FastVector& refracted)
	{
		bool refract = false;

#if defined ENABLE_AVX512


#elif defined ENABLE_AVX256
		const float NdotV = Dot(v.UnitVector(), n);
		float k = 1.0f - ni_over_nt * ni_over_nt * (1.0f - NdotV * NdotV);

		if (k > 0.0f)
		{
			refracted = v * ni_over_nt - n * (ni_over_nt * NdotV + sqrtf(k));
			refract = true;
		}
		else
			refract = false;

#elif defined ENABLE_AVX128
		const XMVECTOR unit_v = XMVector3Normalize(v.data.xm);
		const float NdotV = XMVectorGetX(XMVector3Dot(unit_v, n.data.xm));
		const float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1 - NdotV * NdotV);

		if (discriminant > 0)
		{
			const XMVECTOR scaledN = XMVectorScale(n.data.xm, NdotV);
			const XMVECTOR term1 = XMVectorScale(XMVectorSubtract(unit_v, scaledN), ni_over_nt);
			const XMVECTOR term2 = XMVectorScale(n.data.xm, -sqrtf(discriminant));

			refracted = XMVectorAdd(term1, term2);
			refract = true;
		}
		else
			refract = false;

#endif

		return refract;
	}

	//-------------------------------------------------------------------------------------------------------------------
	inline float schlick(float cosine, float ref_idx)
	{
		float r0 = pow((1.0f - ref_idx) / (1.0f + ref_idx), 2);
		r0 = r0 * r0;
		return r0 + (1.0f - r0) * pow((1.0f - cosine), 5);
	}
}
