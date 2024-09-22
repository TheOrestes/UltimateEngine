#pragma once

#include "Core/EngineApplication.h"
#include "Core/Logger.h"

//---------------------------------------------------------------------------------------------------------------------
#define LOG_CRITICAL(...)	Logger::getInstance().GetLogger()->critical(__VA_ARGS__)
#define LOG_ERROR(...)		Logger::getInstance().GetLogger()->error(__VA_ARGS__)		
#define LOG_WARNING(...)	Logger::getInstance().GetLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...)		Logger::getInstance().GetLogger()->info(__VA_ARGS__)
#define LOG_DEBUG(...)		Logger::getInstance().GetLogger()->debug(__VA_ARGS__)

//---------------------------------------------------------------------------------------------------------------------
//#define UT_CHECK_NULL(x,...){if((x == nullptr)){LOG_ERROR("Null Ptr:{0}",__VA_ARGS__);return false;}}
//---------------------------------------------------------------------------------------------------------------------
//#define UT_CHECK_BOOL(x,...){if((!x)){LOG_ERROR("Bool False",__VA_ARGS__);return false;}}
//---------------------------------------------------------------------------------------------------------------------
//#define UT_CHECK_HRESULT(x,...){if(FAILED(x)){LOG_ERROR("HRESULT FAILED : {0}",__VA_ARGS__);return false;}}

////---------------------------------------------------------------------------------------------------------------------
//#define UT_ASSERT_NULL(x,...)															\
//{																						\
//	if (x == nullptr)																	\
//	{																					\
//		LOG_CRITICAL("Assertion Failed:{0}", __VA_ARGS__)								\
//		__debugbreak();																	\
//	}																					\
//}																						\

//---------------------------------------------------------------------------------------------------------------------
//#define UT_ASSERT_HRESULT(x,...)														\
//{																						\
//	if (FAILED(x))																		\
//	{																					\
//		LOG_CRITICAL("Assertion Failed:{0}", __VA_ARGS__);								\
//		__debugbreak();																	\
//	}																					\
//}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> bool UT_CHECK_NULL(T a, Types... args)
{
	if (a == nullptr)
	{
		LOG_ERROR("NULL_PTR:{0}", args...);
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> bool UT_CHECK_BOOL(T a, Types... args)
{
	if (!a)
	{
		LOG_ERROR("BOOL_FALSE:{0}", args...);
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> bool UT_CHECK_HRESULT(T a, Types... args)
{
	if(FAILED(a))
	{
		LOG_ERROR("HRESULT FAILED:{0}", args...);
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> void UT_ASSERT_NULL(T a, Types... args)
{
	if (a == nullptr)
	{
		LOG_CRITICAL("Assertion Failed:{0}", args...);
		__debugbreak();
	}
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> void UT_ASSERT_HRESULT(T a, Types... args)
{
	if(FAILED(a))
	{
		LOG_CRITICAL("Assertion Failed:{0}", args...);
		__debugbreak();
	}
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
template<typename T> void SAFE_RELEASE(T*& a)
{
	if (a)
	{
		(a)->Release();
		(a) = nullptr;

		LOG_DEBUG("{0} instance released!", typeid(T).name());
	}
}


								