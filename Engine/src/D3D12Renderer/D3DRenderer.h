#pragma once
#include <d3d12.h>
#include "D3DGlobals.h"

class D3DRenderer
{
public:
	D3DRenderer();
	~D3DRenderer();

	bool	Initialize();
	void	RecordCommands();

private:
	bool	CreateRTV();

private:
	ID3D12DescriptorHeap*													m_pHeapRTV;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, UT::GLOBALS::GFramesInFlight>	m_handlesRTV;
	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_ResourceRT;
};

