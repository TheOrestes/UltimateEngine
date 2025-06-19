#include "UltimateEnginePCH.h"
#include "FastVector.h"

//---------------------------------------------------------------------------------------------------------------------
float FastVector<XMVECTOR>::operator[](int i) const
{
	assert(i >= 0 && i < 4);
	return XMVectorGetByIndex(data, i);
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR>& FastVector<XMVECTOR>::operator+=(const FastVector<XMVECTOR>& v2)
{
	data = XMVectorAdd(data, v2.data);
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR>& FastVector<XMVECTOR>::operator-=(const FastVector<XMVECTOR>& v2)
{
	data = XMVectorSubtract(data, v2.data);
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR>& FastVector<XMVECTOR>::operator*=(const FastVector<XMVECTOR>& v2)
{
	data = XMVectorMultiply(data, v2.data);
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR>& FastVector<XMVECTOR>::operator/=(const FastVector<XMVECTOR>& v2)
{
	data = XMVectorDivide(data, v2.data);
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR>& FastVector<XMVECTOR>::operator*=(const float value)
{
	data = XMVectorScale(data, value);
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR>& FastVector<XMVECTOR>::operator/=(const float value)
{
	data = XMVectorScale(data, 1.0f / value);
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector<XMVECTOR>::Length() const
{
	return XMVectorGetX(XMVector3Length(data));
}

//---------------------------------------------------------------------------------------------------------------------
float FastVector<XMVECTOR>::LengthSquared() const
{
	return XMVectorGetX(XMVector3LengthSq(data));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator+(const FastVector<XMVECTOR>& lhs, const FastVector<XMVECTOR>& rhs)
{
	return FastVector<XMVECTOR>(XMVectorAdd(lhs.data, rhs.data));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator-(const FastVector<XMVECTOR>& lhs, const FastVector<XMVECTOR>& rhs)
{
	return FastVector<XMVECTOR>(XMVectorSubtract(lhs.data, rhs.data));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator-(const FastVector<XMVECTOR>& vec)
{
	return FastVector<XMVECTOR>(XMVectorNegate(vec.data));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator*(const FastVector<XMVECTOR>& lhs, const FastVector<XMVECTOR>& rhs)
{
	return FastVector<XMVECTOR>(XMVectorMultiply(lhs.data, rhs.data));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator/(const FastVector<XMVECTOR>& lhs, const FastVector<XMVECTOR>& rhs)
{
	return FastVector<XMVECTOR>(XMVectorDivide(lhs.data, rhs.data));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator*(const FastVector<XMVECTOR>& vec, const float value)
{
	return FastVector<XMVECTOR>(XMVectorScale(vec.data, value));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator*(const float value, const FastVector<XMVECTOR>& vec)
{
	return FastVector<XMVECTOR>(XMVectorScale(vec.data, value));
}

//---------------------------------------------------------------------------------------------------------------------
FastVector<XMVECTOR> operator/(const FastVector<XMVECTOR>& vec, const float value)
{
	return FastVector<XMVECTOR>(XMVectorScale(vec.data, 1.0f/value));
}
