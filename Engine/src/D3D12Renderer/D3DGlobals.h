#pragma once

#include "Core/Core.h"
#include <dxgidebug.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <GLFW/glfw3.h>
#include "GLFW/glfw3native.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>

#include "EngineHeader.h"

using namespace Microsoft::WRL;
using namespace DirectX;

//#define ENABLE_AVX128
#define ENABLE_AVX256

namespace UT
{
	namespace D3D12
	{
		namespace CORE
		{
			extern constexpr ID3D12Device*				const		GetDevice();
			extern constexpr IDXGIFactory6*				const		GetFactory();
			extern constexpr ID3D12CommandQueue*		const		GetCommandQueue();
			extern constexpr IDXGISwapChain4*			const		GetSwapchain();
			extern constexpr ID3D12CommandAllocator*	const		GetCommandAllocator(uint16_t index);
			extern constexpr ID3D12GraphicsCommandList* const		GetCommandList(uint16_t index);

			bool			Initialize();
			void			BeginFrame();
			void			EndFrame();
			void			Cleanup();
		}
	}

	namespace GLOBALS
	{
		enum class SIMDType
		{
			XMVECTOR,
			AVX2,
			AVX512
		};

		inline uint16_t GWindowWidth = 960;
		inline uint16_t GWindowHeight = 540;
		inline uint16_t GCurrentFrameId = 0;
		inline HWND		GWindowHandle = nullptr;

		inline constexpr uint16_t GFramesInFlight = 3;

		inline SIMDType	SIMD_TYPE = SIMDType::XMVECTOR;
	}
}