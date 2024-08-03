#pragma once

#ifdef UT_PLATFORM_WINDOWS
	#ifdef UT_BUILD_DLL
		#define UT_API __declspec(dllexport)
	#else
		#define UT_API __declspec(dllimport)
	#endif
using namespace Microsoft::WRL;
#else
	#error Ultimate Engine supports only Windows Platform!
#endif


