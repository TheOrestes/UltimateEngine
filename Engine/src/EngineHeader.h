#pragma once

#include "Core/EngineApplication.h"
#include "Core/Logger.h"

#include <typeinfo>

//---------------------------------------------------------------------------------------------------------------------
#define LOG_CRITICAL(...)	Logger::getInstance().GetLogger()->critical(__VA_ARGS__);
#define LOG_ERROR(...)		Logger::getInstance().GetLogger()->error(__VA_ARGS__);		
#define LOG_WARNING(...)	Logger::getInstance().GetLogger()->warn(__VA_ARGS__);
#define LOG_INFO(...)		Logger::getInstance().GetLogger()->info(__VA_ARGS__);
#define LOG_DEBUG(...)		Logger::getInstance().GetLogger()->debug(__VA_ARGS__); 			
	
//---------------------------------------------------------------------------------------------------------------------
#define UT_ASSERT(x,...){if((x==nullptr)){LOG_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
#define UT_ASSERT_VK(x,...){if((x==nullptr)){LOG_CRITICAL("VK Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}

//---------------------------------------------------------------------------------------------------------------------
const uint16_t gWindowWidht = 1920;
const uint16_t gWindowHeight = 1080;

//---------------------------------------------------------------------------------------------------------------------
template<typename T> void SAFE_DELETE(T*& a)
{
	if (a != nullptr)
	{
		delete a;
		a = nullptr;

		LOG_DEBUG("{0} instance deleted!", typeid(T).name());
	}
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