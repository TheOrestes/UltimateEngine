#pragma once

#include "Core/Core.h"
#include <dxgidebug.h>
#include <DirectXMath.h>
#include <GLFW/glfw3.h>
#include <d3d12.h>
#include <dxgi1_6.h>

using namespace Microsoft::WRL;

namespace UT
{
	namespace Globals
	{
		inline uint16_t GWindowWidth = 960;
		inline uint16_t GWindowHeight = 540;

		constexpr uint16_t GBackbufferCount = 3;
	}

	namespace DAS
	{
		struct VertexPC
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Color;
		};
	}
}
