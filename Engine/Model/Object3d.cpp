#include "Object3d.h"
#include "ModelManager.h"
#include "DXCom.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "PointLightManager.h"
#include "CameraManager.h"

Object3d::~Object3d() {
	delete model_;
}

void Object3d::Create(const std::string& fileName) {

	this->camera_ = CameraManager::GetInstance()->GetCamera();
	ModelManager::GetInstance()->LoadOBJ(fileName);
	SetModel(fileName);
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	CreateWVP();

}

void Object3d::CreateSphere() {

	this->camera_ = CameraManager::GetInstance()->GetCamera();
	ModelManager::GetInstance()->CreateSphere();
	SetModel("Sphere");
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	CreateWVP();
}

void Object3d::Draw() {

	SetWVP();

	ID3D12GraphicsCommandList* cList = DXCom::GetInstance()->GetCommandList();
	cList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	cList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	cList->SetGraphicsRootConstantBufferView(4, cameraPosResource_->GetGPUVirtualAddress());
	PointLightManager::GetInstance()->SetLightCommand(cList);

	if (model_) {
		model_->Draw(cList);
	}

}

void Object3d::SetColor(const Vector4& color) {
	model_->SetColor(color);
}

void Object3d::SetModel(const std::string& fileName) {
	model_ = new Model(*(ModelManager::FindModel(fileName)));
}

void Object3d::CreateWVP() {

	wvpResource_ = DXCom::GetInstance()->CreateBufferResource(DXCom::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	wvpDate_ = nullptr;
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpDate_));
	wvpDate_->WVP = MakeIdentity4x4();
	wvpDate_->World = MakeIdentity4x4();
	wvpDate_->WorldInverseTransPose = Transpose(Inverse(wvpDate_->World));


	directionalLightResource_ = DXCom::GetInstance()->CreateBufferResource(DXCom::GetInstance()->GetDevice(), sizeof(DirectionalLight));
	directionalLightData_ = nullptr;
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData_->direction = { 1.0f,0.0f,0.0f };
	directionalLightData_->intensity = 1.0f;


	cameraPosResource_ = DXCom::GetInstance()->CreateBufferResource(DXCom::GetInstance()->GetDevice(), sizeof(DirectionalLight));
	cameraPosData_ = nullptr;
	cameraPosResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraPosData_));
	cameraPosData_->worldPosition = camera_->transform.translate;


}

void Object3d::SetWVP() {

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix= Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}

	wvpDate_->World = worldMatrix;
	wvpDate_->WVP = worldViewProjectionMatrix;
	wvpDate_->WorldInverseTransPose = Transpose(Inverse(wvpDate_->World));

	cameraPosData_->worldPosition = camera_->transform.translate;

}