#pragma once
#include <d3d12.h>
#include "D3DGlobals.h"

class RT_Scene;

class D3DRenderer
{
public:
	D3DRenderer();
	~D3DRenderer();

	bool	Initialize();
	void	RecordCommands();

	void	StartRayTracerAccumulationThread();
	void	REBDER_COMPUTE();
	void	RENDER_CUDA();

private:
	bool	CreateRTV();
	bool	CreateUploadBuffer();
	void	AccumulatePixels();

private:
	ID3D12DescriptorHeap*													m_pHeapRTV;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, UT::GLOBALS::GFramesInFlight>	m_handlesRTV;
	std::array<ID3D12Resource*, UT::GLOBALS::GFramesInFlight>				m_ResourceRT;

	ID3D12Resource*															m_ResourceUploadBuffer;

	UINT8*																	m_PersistentData;
	RT_Scene*																m_pRTScene;
	XMFLOAT3																m_RTColor;
	std::thread																m_threadAccumulation;
	std::atomic<bool>														m_bAppRunning;

	std::mutex																m_mutexAccumulation;
	std::vector<float>														m_ListAccumulatedBuffer;		// stores float values for RGBA channel
	std::vector<uint32_t>													m_ListSampleCount;				// tracks how many frames each pixel has accumulated.
};

