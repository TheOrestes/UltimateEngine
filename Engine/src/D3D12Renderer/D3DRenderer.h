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

	void	Render();

private:
	bool	CreateRTV();
	bool	CreateTriangle();

private:
	ID3D12DescriptorHeap* m_pHeapRTV;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, UT::GLOBALS::GFramesInFlight>	m_handlesRTV;
	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_ResourceRT;

	// Triangle
	ID3D12RootSignature*						m_pRootSignature;
	ID3D12PipelineState*						m_pPSO;
	ID3D12Resource*								m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW					m_VertexBufferView;

	D3D12_VIEWPORT								m_Viewport;
	D3D12_RECT									m_ScissorRect;
};
