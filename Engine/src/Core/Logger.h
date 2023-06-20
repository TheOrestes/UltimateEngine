#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"

class UT_API Logger
{
public:
	static Logger& getInstance()
	{
		static Logger instance;
		return instance;
	}

	inline spdlog::logger* GetLogger()	{ return m_pLogger;  }

private:
	Logger();
	Logger(const Logger&);

	spdlog::logger* m_pLogger;
};

