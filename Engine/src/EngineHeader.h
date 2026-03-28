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
template <typename T> void UT_NAME_D3D_OBJECT(T type, std::string name)
{
	std::wstring wStr(name.begin(), name.end());
	type->SetName(wStr.c_str());
	LOG_INFO("D3D Object Created => {0}", name);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void UT_NAME_D3D_OBJECT_INDEXED(T type, int n, std::string name)
{
	name += std::to_string(n);
	std::wstring wStr(name.begin(), name.end());
	type->SetName(wStr.c_str());
	LOG_INFO("D3D Object Created => {0}", name);
}

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
	bool status = false;

	if (FAILED(a))
	{
		LOG_ERROR("FAILED => {0} | {1}", args...);
		status = false;
	}
	else
	{
		//LOG_DEBUG("SUCCESS => {0} | {1}", args...);
		status = true;
	}

	return status;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> void UT_ASSERT_NULL(T a, Types... args)
{
	if (a == nullptr)
	{
		LOG_CRITICAL("!! ASSERT !! => {0}", args...);
		__debugbreak();
	}
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Types> void UT_ASSERT_HRESULT(T a, Types... args)
{
	if (FAILED(a))
	{
		// Cast HRESULT to UINT32 for safe formatting
		LOG_CRITICAL("!! ASSERT !! HRESULT={0:x} | {1}", static_cast<UINT32>(a), std::forward<Types>(args)...);
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

		LOG_INFO("{0} object deleted!", typeid(T).name());
	}
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T> void SAFE_RELEASE(T*& a)
{
	if (a)
	{
		(a)->Release();
		(a) = nullptr;

		LOG_WARNING("{0} object released!", typeid(T).name());
	}
}

