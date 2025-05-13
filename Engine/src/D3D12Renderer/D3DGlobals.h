#pragma once

#include "Core/Core.h"
#include <dxgidebug.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <GLFW/glfw3.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#include "EngineHeader.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace UT
{
	namespace D3D12
	{
		namespace CORE
		{
			extern constexpr ID3D12Device* const GetDevice();
			extern constexpr IDXGIFactory6* const GetFactory();

			bool Initialize();
			void Cleanup();
		}

		namespace Globals
		{
			inline uint16_t GWindowWidth = 960;
			inline uint16_t GWindowHeight = 540;

			inline constexpr uint16_t GBackbufferCount = 3;
		}
	}
}