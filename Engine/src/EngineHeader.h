#pragma once

#include "Core/EngineApplication.h"
#include "Core/Logger.h"

//---------------------------------------------------------------------------------------------------------------------
#define LOG_CRITICAL(...)	Logger::getInstance().GetLogger()->critical(__VA_ARGS__);
#define LOG_ERROR(...)		Logger::getInstance().GetLogger()->error(__VA_ARGS__);		
#define LOG_WARNING(...)	Logger::getInstance().GetLogger()->warn(__VA_ARGS__);
#define LOG_INFO(...)		Logger::getInstance().GetLogger()->info(__VA_ARGS__);
#define LOG_DEBUG(...)		Logger::getInstance().GetLogger()->debug(__VA_ARGS__);

//---------------------------------------------------------------------------------------------------------------------
const uint16_t gWindowWidht = 1920;
const uint16_t gWindowHeight = 1080;

//---------------------------------------------------------------------------------------------------------------------
template<typename T> void SAFE_DELETE(T*& a)
{
	delete a;
	a = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
#define CHECK(x)						\
{										\
	if (!x)								\
	{									\
		return false;					\
	}									\
}										\

//---------------------------------------------------------------------------------------------------------------------
#define VK_CHECK(x)																		\
{																						\
	if (x != VK_SUCCESS)																\
	{																					\
		LOG_ERROR("Detected Vulkan Error at {0}:{1}", __FILE__, __LINE__);				\
		return false;																	\
	}																					\
}										