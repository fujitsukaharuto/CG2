#pragma once
#include "Game/OriginGameObject.h"
#include "ImGuiManager/ImGuiManager.h"
#include <numbers>
#include "Game/Collider/BaseCollider.h"
#include "Game/Collider/AABBCollider.h"
#include "Model/Line3dDrawer.h"

class TestBaseObj : public OriginGameObject {
public:
	TestBaseObj();
	~TestBaseObj()override;

	void Initialize()override;
	void Update()override;
	void Draw(Material* mate = nullptr)override;
	void DebugGUI()override;

	void OnCollisionEnter(const ColliderInfo& other);
	void OnCollisionStay(const ColliderInfo& other);
	void OnCollisionExit(const ColliderInfo& other);


	BaseCollider* GetCollider() { return collider_.get(); }

	std::string name_;

#ifdef _DEBUG
	void Debug()override;
	void DrawCollider();
#endif // _DEBUG

private:

	float time_;
	float omega_;
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	std::unique_ptr<AABBCollider> collider_ = nullptr;
	bool isCollider_ = false;

};


inline TestBaseObj::TestBaseObj() {
}

inline TestBaseObj::~TestBaseObj() {
}

inline void TestBaseObj::Initialize() {
	OriginGameObject::Initialize();
	model_->Create("suzanne.obj");
	omega_ = 2.0f * std::numbers::pi_v<float> / 300.0f;
	collider_ = std::make_unique<AABBCollider>();
	collider_->SetCollisionEnterCallback([this](const ColliderInfo& other) {OnCollisionEnter(other); });
	collider_->SetCollisionStayCallback([this](const ColliderInfo& other) {OnCollisionStay(other); });
	collider_->SetCollisionExitCallback([this](const ColliderInfo& other) {OnCollisionExit(other); });
}

inline void TestBaseObj::Update() {
	isCollider_ = false;
	
	collider_->SetPos(model_->GetWorldPos());
}

inline void TestBaseObj::Draw([[maybe_unused]] Material* mate) {
	OriginGameObject::Draw(mate);
}

inline void TestBaseObj::DebugGUI() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader(name_.c_str())) {
		OriginGameObject::DebugGUI();
		collider_->SetPos(model_->GetWorldPos());

		collider_->DebugGUI();
	}
#endif // _DEBUG
}

inline void TestBaseObj::OnCollisionEnter([[maybe_unused]]const ColliderInfo& other) {

	isCollider_ = true;
	color_ = { 0.0f,0.0f,0.0f,1.0f };

}

inline void TestBaseObj::OnCollisionStay([[maybe_unused]] const ColliderInfo& other) {
	isCollider_ = true;
	color_ = { 0.0f,1.0f,1.0f,1.0f };
}

inline void TestBaseObj::OnCollisionExit([[maybe_unused]] const ColliderInfo& other) {
	color_ = { 1.0f,0.0f,1.0f,1.0f };
}

#ifdef _DEBUG
inline void TestBaseObj::Debug() {

	if (ImGui::CollapsingHeader(name_.c_str())) {
		ImGui::DragFloat3("position", &model_->transform.translate.x, 0.1f);
		collider_->SetPos(model_->GetWorldPos());

		float w = collider_->GetWidth();
		float h = collider_->GetHeight();
		float d = collider_->GetDepth();
		ImGui::DragFloat("width", &w, 0.1f);
		ImGui::DragFloat("height", &h, 0.1f);
		ImGui::DragFloat("depth", &d, 0.1f);
		collider_->SetWidth(w);
		collider_->SetHeight(h);
		collider_->SetDepth(d);
		bool isColl = collider_->GetIsCollisonCheck();
		ImGui::Checkbox("isCollision", &isColl);
		collider_->SetIsCollisonCheck(isColl);
	}
}
inline void TestBaseObj::DrawCollider() {

	// 半サイズを計算
	float halfWidth = collider_->GetWidth() / 2.0f;
	float halfHeight = collider_->GetHeight() / 2.0f;
	float halfDepth = collider_->GetDepth() / 2.0f;

	// 8つの頂点を計算
	Vector3 v1 = { collider_->GetPos().x - halfWidth, collider_->GetPos().y - halfHeight, collider_->GetPos().z - halfDepth};
	Vector3 v2 = { collider_->GetPos().x + halfWidth, collider_->GetPos().y - halfHeight, collider_->GetPos().z - halfDepth };
	Vector3 v3 = { collider_->GetPos().x + halfWidth, collider_->GetPos().y + halfHeight, collider_->GetPos().z - halfDepth };
	Vector3 v4 = { collider_->GetPos().x - halfWidth, collider_->GetPos().y + halfHeight, collider_->GetPos().z - halfDepth };
	Vector3 v5 = { collider_->GetPos().x - halfWidth, collider_->GetPos().y - halfHeight, collider_->GetPos().z + halfDepth };
	Vector3 v6 = { collider_->GetPos().x + halfWidth, collider_->GetPos().y - halfHeight, collider_->GetPos().z + halfDepth };
	Vector3 v7 = { collider_->GetPos().x + halfWidth, collider_->GetPos().y + halfHeight, collider_->GetPos().z + halfDepth };
	Vector3 v8 = { collider_->GetPos().x - halfWidth, collider_->GetPos().y + halfHeight, collider_->GetPos().z + halfDepth };

	// 線を描画
	Line3dDrawer::GetInstance()->DrawLine3d(v1, v2, color_); // 底面の線
	Line3dDrawer::GetInstance()->DrawLine3d(v2, v3, color_);
	Line3dDrawer::GetInstance()->DrawLine3d(v3, v4, color_);
	Line3dDrawer::GetInstance()->DrawLine3d(v4, v1, color_);

	Line3dDrawer::GetInstance()->DrawLine3d(v5, v6, color_); // 上面の線
	Line3dDrawer::GetInstance()->DrawLine3d(v6, v7, color_);
	Line3dDrawer::GetInstance()->DrawLine3d(v7, v8, color_);
	Line3dDrawer::GetInstance()->DrawLine3d(v8, v5, color_);

	Line3dDrawer::GetInstance()->DrawLine3d(v1, v5, color_); // 側面の線
	Line3dDrawer::GetInstance()->DrawLine3d(v2, v6, color_);
	Line3dDrawer::GetInstance()->DrawLine3d(v3, v7, color_);
	Line3dDrawer::GetInstance()->DrawLine3d(v4, v8, color_);

}
#endif // _DEBUG
