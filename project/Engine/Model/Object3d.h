#pragma once
#include <wrl/client.h>
#include <string>
#include "Model.h"
#include "Camera.h"
#include "Object3dCommon.h"


class PointLight;
class SpotLight;

class Object3d {
public:
	Object3d() = default;
	~Object3d();

public:

	void Create(const std::string& fileName);

	void CreateSphere();

	void Draw(Material* mate = nullptr);

	void AnimeDraw();

	Matrix4x4 GetWorldMat() const;

	Vector3 GetWorldPos()const;

	void SetColor(const Vector4& color);

	void SetRightDir(const Vector3& right) { directionalLightData_->direction = right; }

	void SetCamera(Camera* camera) { this->camera_ = camera; }

	void SetParent(Object3d* parent) { parent_ = parent; }

	void SetCameraParent(bool is) { isCameraParent_ = is; }

	void SetTexture(const std::string& name);

	void SetBillboardMat(const Matrix4x4& mat) { billboardMatrix_ = mat; }

	/*void SetPointLight(PointLight* light) { pointLight_ = light; }
	void SetSpotLight(SpotLight* light) { spotLight_ = light; }*/

	Trans transform{};

private:

	void SetModel(const std::string& fileName);

	void CreateWVP();

	void SetWVP();

	void SetBillboardWVP();

private:
	Object3dCommon* common_;
	std::unique_ptr<Model> model_ = nullptr;
	PointLight* pointLight_;
	SpotLight* spotLight_;
	Camera* camera_;
	Object3d* parent_ = nullptr;

	bool isCameraParent_ = false;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	TransformationMatrix* wvpDate_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraPosResource_ = nullptr;
	CameraForGPU* cameraPosData_ = nullptr;

	Matrix4x4 billboardMatrix_;
	std::string nowTextureName;
};
