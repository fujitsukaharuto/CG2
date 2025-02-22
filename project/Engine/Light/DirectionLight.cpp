#include "DirectionLight.h"
#include "DXCom.h"
#include "ImGuiManager/ImGuiManager.h"

void DirectionLight::Initialize() {
	drectionLightResource_ = DXCom::GetInstance()->CreateBufferResource(DXCom::GetInstance()->GetDevice(), sizeof(DirectionalLight));
	directionLightData_ = nullptr;
	drectionLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionLightData_));
	directionLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	directionLightData_->direction = { 0.0f,-1.0f,0.0f };
	directionLightData_->intensity = 0.3f;
}

void DirectionLight::SetLightCommand(ID3D12GraphicsCommandList* commandList) {
	commandList->SetGraphicsRootConstantBufferView(3, drectionLightResource_->GetGPUVirtualAddress());
}


void DirectionLight::Debug() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("directionLight")) {

		ImGui::ColorEdit4("color##direction", &directionLightData_->color.x);
		ImGui::DragFloat("intensity##direction", &directionLightData_->intensity, 0.01f);
		ImGui::DragFloat3("direction##direction", &directionLightData_->direction.x, 0.1f);
		directionLightData_->direction = directionLightData_->direction.Normalize();

	}
#endif // _DEBUG
}
