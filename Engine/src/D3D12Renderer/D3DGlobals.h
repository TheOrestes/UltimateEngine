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
		inline void CreateVertexShader(const std::string& vsFile, ID3DBlob** vertexShaderBlob)
		{
			//UT_ASSERT_NULL(vertexShaderBlob);

			ID3DBlob* errorBlob;

			const std::wstring sTemp = std::wstring(vsFile.begin(), vsFile.end());
			const LPCWSTR ws = sTemp.c_str();

			HRESULT Hr = D3DCompileFromFile(ws, nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vertexShaderBlob, &errorBlob);

			if (FAILED(Hr) && errorBlob != nullptr)
			{
				const char* ErrorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
				UT_ASSERT_HRESULT(Hr, "CreateVertexShader", ErrorMsg);
			}

			// Extract the shader name getting compiled...
			std::size_t pos = vsFile.find_last_of("/");
			std::string shaderName = vsFile.substr(++pos);
			LOG_DEBUG("{0} => Vertex Shader Compiled", shaderName);
		}

		//-------------------------------------------------------------------------------------------------------------------
		inline void CreateFragmentShader(const std::string& fsFile, ID3DBlob** fragmentShaderBlob)
		{
			//UT_ASSERT_NULL(fragmentShaderBlob);

			ID3DBlob* errorBlob;

			const std::wstring sTemp = std::wstring(fsFile.begin(), fsFile.end());
			const LPCWSTR ws = sTemp.c_str();

			HRESULT Hr = D3DCompileFromFile(ws, nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, fragmentShaderBlob, &errorBlob);

			if (FAILED(Hr) && errorBlob != nullptr)
			{
				const char* ErrorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
				UT_ASSERT_HRESULT(Hr, "CreateFragmentShader", ErrorMsg);
			}

			// Extract the shader name getting compiled...
			std::size_t pos = fsFile.find_last_of("/");
			std::string shaderName = fsFile.substr(++pos);
			LOG_DEBUG("{0} => Fragment Shader Compiled", shaderName);
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
