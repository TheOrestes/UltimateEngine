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


	}
	return false;
}
