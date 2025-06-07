#pragma once

const float PI = 3.14159265358f;

namespace Helper
{
	XMVECTOR LerpVector(FXMVECTOR vec1, FXMVECTOR vec2, float t)
	{
		return XMVectorLerp(vec1, vec2, t);
	}

	float GetRandom01()
	{
		return static_cast<float>(rand()) / RAND_MAX;
	}

	XMVECTOR GetRandomInUnitDisk()
	{
		XMVECTOR p;
		do
		{
			const XMVECTOR randomVec = XMVectorSet(GetRandom01(), GetRandom01(), 0.0f, 0.0f);
			const XMVECTOR unitVec = XMVectorSet(1, 1, 0, 0);

			p = XMVectorSubtract(XMVectorScale(randomVec, 2.0f), unitVec);

		} while (XMVectorGetX(XMVector3Dot(p, p)) >= 1.0f);
		
		return p;
	}

	XMVECTOR RandomInUnitSphere()
	{
		XMVECTOR P;

		do
		{
			const XMVECTOR randomVec = XMVectorSet(GetRandom01(), GetRandom01(), GetRandom01(), 0.0f);
			const XMVECTOR unitVec = XMVectorSet(1, 1, 1, 0);

			P = XMVectorSubtract(XMVectorScale(randomVec, 2.0f), unitVec);

		} while (XMVectorGetX(XMVector3LengthSq(P)) >= 1.0f);

		return P;
	}

	XMVECTOR Reflect(FXMVECTOR v, FXMVECTOR n)
	{
		return XMVector3Reflect(v, n);
	}

	bool Refract(FXMVECTOR v, FXMVECTOR n, float ni_over_nt, XMVECTOR& refracted)
	{
		const XMVECTOR unit_v = XMVector3Normalize(v);
		const float NdotV = XMVectorGetX(XMVector3Dot(unit_v, n));
		const float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1 - NdotV * NdotV);

		if (discriminant > 0)
		{
			const XMVECTOR scaledN = XMVectorScale(n, NdotV);
			const XMVECTOR term1 = XMVectorScale(XMVectorSubtract(unit_v, scaledN), ni_over_nt);
			const XMVECTOR term2 = XMVectorScale(n, -sqrtf(discriminant));

			refracted = XMVectorAdd(term1, term2);
			return true;
		}
		else
			return false;
	}

	XMFLOAT3 AddFloat3(const XMFLOAT3& a, const XMFLOAT3& b)
	{
		XMVECTOR va = XMLoadFloat3(&a);
		XMVECTOR vb = XMLoadFloat3(&b);

		XMVECTOR resultVec = XMVectorAdd(va, vb);

		XMFLOAT3 result;
		XMStoreFloat3(&result, resultVec);

		return result;
	}

	XMFLOAT3 DivideFloat3(const XMFLOAT3& a, float scalar)
	{
		XMVECTOR va = XMLoadFloat3(&a);
		XMVECTOR resultVec = XMVectorScale(va, 1.0f / scalar); // Multiply by reciprocal

		XMFLOAT3 result;
		XMStoreFloat3(&result, resultVec);

		return result;
	}

	float schlick(float cosine, float ref_idx)
	{
		float r0 = pow((1.0f - ref_idx) / (1.0f + ref_idx), 2);
		r0 = r0 * r0;
		return r0 + (1.0f - r0) * pow((1.0f - cosine), 5);
	}
}
