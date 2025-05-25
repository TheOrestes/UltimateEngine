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

	void	MODIFY_PIXELS_CPU();
	void	REBDER_COMPUTE();
	void	RENDER_CUDA();

private:
	bool	CreateRTV();
	bool	CreateUploadBuffer();

private:
	ID3D12DescriptorHeap*													m_pHeapRTV;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, UT::GLOBALS::GFramesInFlight>	m_handlesRTV;
	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_ResourceRT;

	ID3D12Resource*															m_ResourceUploadBuffer;
};

