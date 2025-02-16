#pragma once

#include "D3DGlobals.h"

class DXDescriptorHeap
{
public:
	explicit DXDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_HeapType(type) {}

	bool	Initialize(uint32_t capacity, bool isShaderVisible);

	constexpr  bool							IsValid()			const { return m_hCPUStartAddr.ptr != 0; }
	constexpr  bool							IsShaderVisible()	const { return m_hGPUStartAddr.ptr != 0; }

	constexpr  D3D12_DESCRIPTOR_HEAP_TYPE	GetType()			const { return m_HeapType; }
	constexpr  D3D12_CPU_DESCRIPTOR_HANDLE	GetCPUStartAddr()	const { return m_hCPUStartAddr; }
	constexpr  D3D12_GPU_DESCRIPTOR_HANDLE	GetGPUStartAddr()	const { return m_hGPUStartAddr; }
	ComPtr<ID3D12DescriptorHeap>			GetHeapComPtr()		const { return m_pHeap.Get(); }

	constexpr uint32_t						GetCapacity()		const { return m_uiCapacity; }
	constexpr uint32_t						GetNumDescriptors() const { return m_uiSize; }
	constexpr uint32_t						GetDescriptorSize() const { return m_uiDescriptorSize; }

private:
	ComPtr<ID3D12DescriptorHeap>			m_pHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE				m_hCPUStartAddr;
	D3D12_GPU_DESCRIPTOR_HANDLE				m_hGPUStartAddr;

	std::vector<std::unique_ptr<uint32_t>>	m_listFreeHandles;

	uint32_t								m_uiCapacity;
	uint32_t								m_uiSize;
	uint32_t								m_uiDescriptorSize;

	const D3D12_DESCRIPTOR_HEAP_TYPE		m_HeapType;
};
