
#include "Logger.h"

Logger::Logger()
{
	spdlog::set_pattern("%^[%H:%M:%S] %n: [%l] %v%$");

	m_pLogger= spdlog::default_logger_raw();
	m_pLogger->set_level(spdlog::level::trace);
}
