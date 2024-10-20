#include "UltimateEnginePCH.h"
#include "UIRenderer.h"

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

	UT_CHECK_BOOL(ImGui_ImplGlfw_InitForOther(const_cast<GLFWwindow*>(pWindow), true), "ImGui_ImplGlfw_InitForOther() failed!");
	UT_CHECK_BOOL(ImGui_ImplDX12_Init(	pDXRenderDevice->GetD3DDevice().Get(),
										UT::Globals::GBackbufferCount,
										DXGI_FORMAT_R8G8B8A8_UNORM,
										pDXRenderDevice->GetDescriptorHeapShaderResourceView().Get(),
										pDXRenderDevice->GetCPUDescriptorHandleShaderResourceView(),
										pDXRenderDevice->GetGPUDescriptorHandleShaderResourceView()),
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
void UIRenderer::Render(const DXRenderDevice* pDXRenderDevice, ComPtr<ID3D12GraphicsCommandList> pGraphicsCommandList, DirectX::XMFLOAT4& clearColor )
{
	bool show = true;
	//ImGui::ShowDemoWindow(&show);

	ImVec4 ClearColor = ImVec4(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

	ImGui::Begin("Globals");

	if (ImGui::ColorEdit3("clear color", (float*)&ClearColor))
	{
		clearColor.x = ClearColor.x;
		clearColor.y = ClearColor.y;
		clearColor.z = ClearColor.z;
		clearColor.w = ClearColor.w;
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	ImGui::End();

	ImGui::Render();

	pGraphicsCommandList->SetDescriptorHeaps(1, pDXRenderDevice->GetDescriptorHeapShaderResourceView().GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pGraphicsCommandList.Get());
}

//---------------------------------------------------------------------------------------------------------------------
void UIRenderer::End(ComPtr<ID3D12GraphicsCommandList> pGraphicsCommandList)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault(nullptr, (void*)pGraphicsCommandList.Get());
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

