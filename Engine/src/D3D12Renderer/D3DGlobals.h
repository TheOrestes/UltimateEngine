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
	//-------------------------------------------------------------------------------------------------------------------
	namespace Globals
	{
		inline uint16_t GWindowWidth = 960;
		inline uint16_t GWindowHeight = 540;

		constexpr uint16_t GBackbufferCount = 3;
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
	}

	//-------------------------------------------------------------------------------------------------------------------
	namespace HelperFunc
	{
		inline ComPtr<ID3DBlob> CreateVertexShader(const std::string& vsFile)
		{
			ComPtr<ID3DBlob> vsBlob;
			ComPtr<ID3DBlob> errorBlob;

			const std::wstring sTemp = std::wstring(vsFile.begin(), vsFile.end());
			const LPCWSTR ws = sTemp.c_str();

			UT_CHECK_HRESULT(D3DCompileFromFile(ws,
												nullptr,
												nullptr,
												"main",
												"vs_5_0",
												D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
												0,
												&vsBlob,
												&errorBlob),
							"{0} shader compilation failed", vsFile.c_str());

			return vsBlob;

			// Fill out shader bytecode
			//outVS_ByteCode->BytecodeLength = vsBlob->GetBufferSize();
			//outVS_ByteCode->pShaderBytecode = vsBlob->GetBufferPointer();
		}

		//-------------------------------------------------------------------------------------------------------------------
		inline ComPtr<ID3DBlob> CreateFragmentShader(const std::string& fsFile)
		{
			ComPtr<ID3DBlob> fsBlob;
			ComPtr<ID3DBlob> errorBlob;

			const std::wstring sTemp = std::wstring(fsFile.begin(), fsFile.end());
			const LPCWSTR ws = sTemp.c_str();

			UT_CHECK_HRESULT(D3DCompileFromFile(ws,
												nullptr,
												nullptr,
												"main",
												"ps_5_0",
												D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
												0,
												&fsBlob,
												&errorBlob),
												"{0} shader compilation failed", fsFile.c_str());

			return fsBlob;

			// Fill out shader bytecode
			//outFS_ByteCode->BytecodeLength = fsBlob->GetBufferSize();
			//outFS_ByteCode->pShaderBytecode = fsBlob->GetBufferPointer();
		}

		//-------------------------------------------------------------------------------------------------------------------
		inline void CreateVertexInputLayoutDesc(D3D12_INPUT_LAYOUT_DESC& outLayoutDesc)
		{
			D3D12_INPUT_ELEMENT_DESC inputLayout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
			};

			// fill out an input layout description structure
			outLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
			outLayoutDesc.pInputElementDescs = inputLayout;
		}
	}
}
