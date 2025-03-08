#pragma once

#include "Core/Core.h"
#include <dxgidebug.h>
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
		//-------------------------------------------------------------------------------------------------------------------
		namespace CORE
		{
			extern constexpr ID3D12Device* const GetDevice();
			extern constexpr IDXGIFactory6* const GetFactory();

			bool Initialize();
			void Cleanup();
		}

		//-------------------------------------------------------------------------------------------------------------------
		namespace DAS
		{
			struct VertexP
			{
				VertexP() = default;

				VertexP(XMFLOAT3 const& iposition) noexcept
					: Position(iposition)
				{
				}

				VertexP(const VertexP&) = default;
				VertexP& operator=(const VertexP&) = default;

				VertexP(VertexP&&) = default;
				VertexP& operator=(VertexP&&) = default;

				XMFLOAT3 Position;
			};

			struct VertexPC
			{
				VertexPC() = default;

				VertexPC(XMFLOAT3 const& iposition, XMFLOAT4 const& icolor) noexcept
					: Position(iposition),
					Color(icolor)
				{
				}

				VertexPC(const VertexPC&) = default;
				VertexPC& operator=(const VertexPC&) = default;

				VertexPC(VertexPC&&) = default;
				VertexPC& operator=(VertexPC&&) = default;

				XMFLOAT3 Position;
				XMFLOAT4 Color;
			};

			struct ConstantBuffer
			{
				XMFLOAT4 offset;
			};
		}

		//-------------------------------------------------------------------------------------------------------------------
		namespace HelperFunc
		{
			void CreateVertexShader(const std::string& vsFile, ID3DBlob** vertexShaderBlob);
			void CreateFragmentShader(const std::string& fsFile, ID3DBlob** fragmentShaderBlob);
			void CreateVertexInputLayoutDesc(D3D12_INPUT_LAYOUT_DESC& outLayoutDesc);
		}
	}

	//-------------------------------------------------------------------------------------------------------------------
	namespace Globals
	{
		inline uint16_t GWindowWidth = 0;
		inline uint16_t GWindowHeight = 0;
		inline double	GDeltaTime = 0.0f;

		inline constexpr uint16_t GBackbufferCount = 3;
	}
}
