#pragma once
#include <d3d12.h>
#include "D3DGlobals.h"

class D3DCube;

class D3DRenderer
{
public:
	D3DRenderer();
	~D3DRenderer();

	bool	Initialize();
	void	RecordCommands();
	void	Update(double dt);
	void	Render();

private:
	bool	CreateRTV();
	bool	CreateDSV();
	bool	CreateTriangle();
	bool	CreateCube();
	bool	CreatePSO();
	bool	CreateConstantBuffer();
	void	UpdateConstantBuffer(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj);
	

private:
	ID3D12DescriptorHeap* m_pHeapRTV;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, UT::GLOBALS::GFramesInFlight>	m_handlesRTV;
	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_listRTBuffers;

	ID3D12DescriptorHeap* m_pHeapDSV;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, UT::GLOBALS::GFramesInFlight>	m_handlesDSV;
	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_listDSBuffers;

	// Triangle
	ID3D12RootSignature*													m_pRootSignature;
	ID3D12PipelineState*													m_pPSO;

	ID3D12Resource*															m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW												m_VertexBufferView;

	ID3D12Resource*															m_pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW													m_IndexBufferView;

	D3D12_VIEWPORT															m_Viewport;
	D3D12_RECT																m_ScissorRect;

	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_listConstantBuffers;
	std::array<UINT8*, UT::GLOBALS::GFramesInFlight>						m_pCBDataBegin;
	UT::D3D12::DAS::TransformCB*											m_pTransformData;

	std::array<D3DCube*, 5>													m_listCubes;
};
