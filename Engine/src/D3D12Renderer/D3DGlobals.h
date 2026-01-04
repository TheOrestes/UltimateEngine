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

			extern constexpr ID3D12DescriptorHeap*		const		GetGlobalDescriptorHeap();

			extern constexpr ID3D12Fence*				const		GetFence(uint16_t index);
			extern constexpr HANDLE						const		GetFenceEvent(uint16_t index); 
			extern constexpr UINT64						const 		GetFenceValue(uint16_t index);

			
			bool			Initialize();
			void			BeginFrame();
			void			EndFrame();
			void			WaitToFinishCurrentFrame();
			void			Cleanup();
			void			FenceIncrement();
			void			ResetCommandList();
			void			CloseAndExecuteCommandList();
		}

		namespace HELPER
		{
			void CreateGPUBuffer(UINT64 byteSize, ID3D12Resource** outGPUBuffer);
			void CreateUploadBuffer(UINT64 byteSize, ID3D12Resource** outUploadBuffer);
			void CreateReadbackBuffer(UINT64 byteSize, ID3D12Resource** outReadbackBuffer);
			void CopyDataFromUploadBufferToGPU(UINT64 byteSize, const void* data, ID3D12Resource* uploadBuffer, ID3D12Resource* gpuBuffer);

			void CompileShader(const std::string& srcFile, const std::string& entryPoint, const std::string& target, const D3D_SHADER_MACRO* pDefines, D3D12_SHADER_BYTECODE& outByteCode);
			void CreateRootSignatue(UINT numRootParams, const D3D12_ROOT_PARAMETER* pRootParams, UINT numStaticSamplers, const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers, D3D12_ROOT_SIGNATURE_FLAGS flags, ID3D12RootSignature** pOutRootSignature);
			void CreatePSO(ID3D12RootSignature* pSignature, const D3D12_SHADER_BYTECODE& vsBytecode, const D3D12_SHADER_BYTECODE& psBytecode, const D3D12_INPUT_LAYOUT_DESC& inputLayout, ID3D12PipelineState** pOutPSO);

			void LoadImageData(const std::string& filePath, int* width, int* height, int* channels, void** outImagaData);
			void CreateTexture(const std::string& filePath, ID3D12Resource** outTexture);
		}

		namespace DAS
		{
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

			struct VertexPNBT
			{
				VertexPNBT() = default;

				VertexPNBT(XMFLOAT3 const& iPosition, XMFLOAT3 const& iNormal, XMFLOAT3 const& iBiNormal, XMFLOAT2 const& iTex) noexcept
					:	Position(iPosition),
						Normal(iNormal),
						BiNormal(iBiNormal),
						TexCoord(iTex)
				{
				}

				VertexPNBT(const VertexPNBT&) = default;
				VertexPNBT& operator=(const VertexPNBT&) = default;

				VertexPNBT(VertexPNBT&&) = default;
				VertexPNBT& operator=(VertexPNBT&&) = default;

				XMFLOAT3 Position;
				XMFLOAT3 Normal;
				XMFLOAT3 BiNormal;
				XMFLOAT2 TexCoord;
			};

			struct GeomsCB
			{
				XMFLOAT4X4 World;
				XMFLOAT4X4 View;
				XMFLOAT4X4 Proj;
			};
		}
	}

	namespace GLOBALS
	{
		inline uint16_t GWindowWidth = 960;
		inline uint16_t GWindowHeight = 540;
		inline uint16_t GCurrentFrameId = 0;
		inline HWND		GWindowHandle = nullptr;

		inline constexpr uint16_t GFramesInFlight = 3;
		inline UINT GCurrentDescriptorIndex = 0;

		std::string GetFileNameWithoutExtension(const std::string& fileName);
		std::string GetExecutableFolderPath();
		std::wstring ToWString(const std::string& utf8Str);

		enum class InputAction
		{
			NONE,
			FORWARD,
			BACK,
			LEFT,
			RIGHT,
			UP,
			DOWN,
			MOUSE_MOVE
		};
	}
}