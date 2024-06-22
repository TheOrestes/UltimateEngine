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
#define UT_ASSERT_NULL(x,...){if((!x)){LOG_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
//---------------------------------------------------------------------------------------------------------------------
#define UT_ASSERT_BOOL(x,...){if((!x)){LOG_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
//---------------------------------------------------------------------------------------------------------------------
#define UT_ASSERT_VK(x,...)																\
{																						\
	if (x != vk::Result::eSuccess)														\
	{																					\
		LOG_CRITICAL("Assertion Failed:{0}", __VA_ARGS__);								\
		__debugbreak();																	\
	}																					\
}	

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
#define SAFE_RELEASE(x)																	\
{																						\
	if(x)																				\
	{																					\
		(x)->Release();																	\
		(x) = nullptr;																	\
	}																					\
}																						\

//---------------------------------------------------------------------------------------------------------------------
#define SAFE_ADDREF(x)																	\
{																						\
	if(x)																				\
	{																					\
		(x)->AddRef();																	\
	}																					\
}																						\

//---------------------------------------------------------------------------------------------------------------------
#define CHECK(x)																		\
{																						\
	if (!(x))																			\
	{																					\
		return false;																	\
	}																					\
}																						\

//---------------------------------------------------------------------------------------------------------------------
#define CHECK_LOG(x,...)																\
{																						\
	if (!(x))																			\
	{																					\
		LOG_ERROR("Returned FALSE:{0}", __VA_ARGS__);									\
		return false;																	\
	}																					\
}																						\

//---------------------------------------------------------------------------------------------------------------------
#define CHECK_VK_RESULT(x)																		\
{																						\
	if ((x) != vk::Result::eSuccess)													\
	{																					\
		LOG_ERROR("Detected Vulkan Error at {0}:{1}", __FILE__, __LINE__);				\
		return false;																	\
	}																					\
}								