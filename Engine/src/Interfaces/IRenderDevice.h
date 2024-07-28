#pragma once

#include "Core/Core.h"

class UT_API IRenderDevice
{
public:
	virtual  ~IRenderDevice() = default;

	virtual const char* GetAPIName() = 0;
	virtual const char* GetGPUName() = 0;
};

