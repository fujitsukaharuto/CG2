#include "ImGuiManager.h"

#include "DXCom.h"
#include "MyWindow.h"
#include "SRVManager.h"
#ifdef _DEBUG
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#endif // _DEBUG

ImGuiManager* ImGuiManager::GetInstance()
{
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Init(
	[[maybe_unused]] MyWin* myWin, [[maybe_unused]] DXCom* dxComon)
{

	dxCommon_ = dxComon;

#ifdef _DEBUG

	SRVManager* srvManager = SRVManager::GetInstance();
	ID3D12DescriptorHeap* srv = srvManager->GetSRVHeap();
	srvIndex = srvManager->Allocate();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(myWin->GetHwnd());
	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(), static_cast<int>(dxCommon_->GetBackBufferCount()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srv,
		srvManager->GetCPUDescriptorHandle(srvIndex),
		srvManager->GetGPUDescriptorHandle(srvIndex));

	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io;
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Dockingを有効化


#endif // _DEBUG

}

void ImGuiManager::Fin()
{
#ifdef _DEBUG
	
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#endif // _DEBUG

}

void ImGuiManager::Begin()
{
#ifdef _DEBUG
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // _DEBUG

}

void ImGuiManager::End()
{
#ifdef _DEBUG
	ImGui::Render();
#endif // _DEBUG

}

void ImGuiManager::Draw()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#ifdef _DEBUG
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // _DEBUG

}
