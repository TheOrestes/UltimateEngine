#include "UltimateEnginePCH.h"
#include "UIRenderer.h"
#include "D3D12Renderer/D3DGlobals.h"

#include "D3D12Renderer/DXRenderDevice.h"

#include "UI/imgui.h"
#include "UI/imgui_impl_glfw.h"
#include "UI/imgui_impl_dx12.h"
#include "UI/imgui_internal.h"

//---------------------------------------------------------------------------------------------------------------------
bool UIRenderer::Initialize(const GLFWwindow* pWindow, const DXRenderDevice* pDXRenderDevice)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ID3D12Device* const pDevice = UT::D3D12::CORE::GetDevice();

	UT_CHECK_BOOL(ImGui_ImplGlfw_InitForOther(const_cast<GLFWwindow*>(pWindow), true), "ImGui_ImplGlfw_InitForOther() failed!");
	UT_CHECK_BOOL(ImGui_ImplDX12_Init(	pDevice,
										UT::Globals::GBackbufferCount,
										DXGI_FORMAT_R8G8B8A8_UNORM,
										pDXRenderDevice->GetDescriptorHeapUI(),
										pDXRenderDevice->GetCPUDescriptorHandleUI(),
										pDXRenderDevice->GetGPUDescriptorHandleUI()),
		"ImGui_ImplDX12_Init() FAILED!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::Begin()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::Render(const DXRenderDevice* pDXRenderDevice, ID3D12GraphicsCommandList* pGraphicsCommandList, DirectX::XMFLOAT4& clearColor )
{
	bool show = true;
	//ImGui::ShowDemoWindow(&show);

	ImVec4 ClearColor = ImVec4(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

	if (ImGui::Begin("Menu"));
	
	if(ImGui::CollapsingHeader("Scene Settings"))
	{
		if (ImGui::ColorEdit3("clear color", (float*)&ClearColor))
		{
			clearColor.x = ClearColor.x;
			clearColor.y = ClearColor.y;
			clearColor.z = ClearColor.z;
			clearColor.w = ClearColor.w;
		}
	}

	if(ImGui::CollapsingHeader("Debug Info"))
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		static float fps_values[30] = {};
		static int fps_values_offset = 0;

		static double refresh_time = 0.0;
		float MAX_FPS_CAP = 1000.0f;
		if (refresh_time == 0.0)
			refresh_time = ImGui::GetTime();

		while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
		{
			fps_values[fps_values_offset] = io.Framerate / MAX_FPS_CAP;
			fps_values_offset = (fps_values_offset + 1) % IM_ARRAYSIZE(fps_values);

			refresh_time += 1.0f / 30.0f;
		}

		// Plots can display overlay texts
		// (in this example, we will display an average value)
		{
			float average_fps = 0.0f;
			for (int n = 0; n < IM_ARRAYSIZE(fps_values); n++)
				average_fps += fps_values[n] * MAX_FPS_CAP;

			average_fps /= (float)IM_ARRAYSIZE(fps_values);


			char overlay[32];
			sprintf(overlay, "Avg FPS = %f", average_fps);
			ImGui::PlotLines("FPS", fps_values, IM_ARRAYSIZE(fps_values), fps_values_offset, overlay, 0.0f, 1.0f, ImVec2(0, 120.0f));
			ImGui::PlotHistogram("FPS", fps_values, IM_ARRAYSIZE(fps_values), fps_values_offset, overlay, 0.0f, 1.0f, ImVec2(0, 120.0f));
		}
	}

	ImGui::End();
	
	ImGui::Render();

	const std::array<ID3D12DescriptorHeap*, 1> arrDescriptorHeaps = { pDXRenderDevice->GetDescriptorHeapUI() };
	pGraphicsCommandList->SetDescriptorHeaps(static_cast<uint32_t>(arrDescriptorHeaps.size()), arrDescriptorHeaps.data());

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pGraphicsCommandList);
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::End(ID3D12GraphicsCommandList* pGraphicsCommandList)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault(nullptr, (void*)pGraphicsCommandList);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::Cleanup()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::CleanupOnWindowResize()
{
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::RecreateOnWindowResize(uint32_t newWidth, uint32_t newHeight)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize = ImVec2(static_cast<float>(newWidth), static_cast<float>(newHeight));
}

