#pragma once

#include "../Core/Core.h"

class UT_API IObject
{
public:
	virtual ~IObject() = default;
	virtual bool Initialize(const void*) = 0;
	virtual void Cleanup(void*) = 0;
};
