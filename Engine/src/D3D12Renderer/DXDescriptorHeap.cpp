#include "UltimateEnginePCH.h"
#include "DXDescriptorHeap.h"

//---------------------------------------------------------------------------------------------------------------------
bool DXDescriptorHeap::Initialize(uint32_t capacity, bool isShaderVisible)
{
	if(capacity != 0 && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2)
	{
		if(m_HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			isShaderVisible = false;
		}

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.Type = m_HeapType;
		desc.NodeMask = 0;
		desc.NumDescriptors = capacity;

		ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

		HRESULT Hr = pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeap));
		UT_CHECK_HRESULT(Hr, "CreateDescriptorHeap", magic_enum::enum_name(desc.Type));

	}
	return false;
}
