#pragma once

#include "Core/Core.h"
#include <DirectXMath.h>

namespace UT
{
	namespace D3DGlobals
	{
		inline uint16_t GWindowWidth = 960;
		inline uint16_t GWindowHeight = 540;

		constexpr uint16_t GBackbufferCount = 3;
	}

	namespace D3DStructs
	{
		struct VertexPC
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Color;
		};
	}
}
