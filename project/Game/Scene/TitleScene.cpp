#include "TitleScene.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "GlobalVariables.h"
#include "CameraManager.h"
#include "FPSKeeper.h"
#include "Math/Random/Random.h"
#include "Model/Line3dDrawer.h"
#include "Particle/ParticleManager.h"
#include "Scene/SceneManager.h"
#include "Engine/Editor/CommandManager.h"


TitleScene::TitleScene() {}

TitleScene::~TitleScene() {
}

void TitleScene::Initialize() {

	obj3dCommon.reset(new Object3dCommon());
	obj3dCommon->Initialize();

	CameraManager::GetInstance()->GetCamera()->transform.rotate = { -1.02f,0.0f,0.0f };
	CameraManager::GetInstance()->GetCamera()->transform.translate = { 0.0f, 3.5f, -20.0f };

#pragma region シーン遷移用
	black_ = std::make_unique<Sprite>();
	black_->Load("white2x2.png");
	black_->SetColor({ 0.0f,0.0f,0.0f,1.0f });
	black_->SetSize({ 1280.0f,720.0f });
	black_->SetAnchor({ 0.0f,0.0f });
#pragma endregion

	skybox_ = std::make_unique<SkyBox>();
	skybox_->SetCommonResources(dxcommon_, SRVManager::GetInstance(), CameraManager::GetInstance()->GetCamera());
	skybox_->Initialize();

	terrain_ = std::make_unique<Object3d>();
	terrain_->Create("terrain.obj");
	terrain_->LoadTransformFromJson("default_terrainTrnasform.json");
	terrain_->SetUVScale({ 20.0f,20.0f }, { 0.0f,0.0f });

	titlePlane_ = std::make_unique<Object3d>();
	titlePlane_->Create("plane.obj");
	titlePlane_->SetTexture("title_beta.png");
	titlePlane_->transform.translate.y = 13.0f;
	titlePlane_->transform.translate.z = -15.0f;
	titlePlane_->transform.rotate.x = -2.291f;
	titlePlane_->transform.rotate.z = 3.142f;

	space_ = std::make_unique<Sprite>();
	space_->Load("spaceKey.png");
	space_->SetPos({ 640.0f,400.0f,0.0f });
	space_->SetSize({ 256.0f,128.0f });

	star_ = std::make_unique<Object3d>();
	star_->Create("Star.obj");
	star_->SetColor({ 0.9f,0.7f,0.0f,1.0f });
	star_->LoadTransformFromJson("titleStar_transform.json");

	cube_ = std::make_unique<AnimationModel>();
	cube_->Create("T_boss.gltf");
	cube_->LoadAnimationFile("T_boss.gltf");
	cube_->transform.translate.y = 3.0f;

	animParentObj_ = std::make_unique<Object3d>();
	animParentObj_->Create("boss.obj");
	animParentObj_->SetAnimParent(cube_->GetJointTrans("mixamorig:LeftHandIndex1"));
	animParentObj_->SetNoneScaleParent(true);
	animParentObj_->LoadTransformFromJson("AnimParent_transform.json");

	cMane_ = std::make_unique<CollisionManager>();

	ParticleManager::Load(emit, "sphere");

}

void TitleScene::Update() {

	cMane_->Reset();

#ifdef _DEBUG



#endif // _DEBUG

	BlackFade();
	skybox_->Update();

	cube_->AnimationUpdate();

	if (input_->TriggerKey(DIK_5)) {
		emit.Emit();
	}
	emit.Emit();

	cMane_->CheckAllCollision();

	ParticleManager::GetInstance()->Update();
}

void TitleScene::Draw() {

	cube_->CSDispatch();

#pragma region 背景描画


	dxcommon_->ClearDepthBuffer();
#pragma endregion


#pragma region 3Dオブジェクト
	skybox_->Draw();

	obj3dCommon->PreDraw();
//	terrain_->Draw();

	cube_->Draw();
	animParentObj_->Draw();

	star_->Draw();

	space_->Draw();

	titlePlane_->Draw();

#ifdef _DEBUG
	CommandManager::GetInstance()->Draw();
#endif // _DEBUG

	ParticleManager::GetInstance()->Draw();

#ifdef _DEBUG
	emit.DrawSize();
#endif // _DEBUG
	cube_->SkeletonDraw();
	Line3dDrawer::GetInstance()->Render();

#pragma endregion

	//test
#pragma region 前景スプライト

	dxcommon_->PreSpriteDraw();
	if (blackTime != 0.0f) {
		black_->Draw();
	}

#pragma endregion
	ModelManager::GetInstance()->PickingDataCopy();
}

void TitleScene::DebugGUI() {
#ifdef _DEBUG
	ImGui::Indent();
	if (ImGui::CollapsingHeader("titlePlane")) {
		titlePlane_->DebugGUI();
	}
	if (ImGui::CollapsingHeader("cube")) {
		cube_->DebugGUI();
	}
	if (ImGui::CollapsingHeader("animParent")) {
		animParentObj_->DebugGUI();
	}

	ImGui::Unindent();
#endif // _DEBUG
}

void TitleScene::ParticleDebugGUI() {
#ifdef _DEBUG
	ImGui::Indent();
	emit.DebugGUI();
	ImGui::Unindent();
#endif // _DEBUG
}

void TitleScene::BlackFade() {
	if (isChangeFase) {
		if (blackTime < blackLimmite) {
			blackTime += FPSKeeper::DeltaTime();
			if (blackTime >= blackLimmite) {
				blackTime = blackLimmite;
			}
		} else {
			if (!isParticleDebugScene_) {
				ChangeScene("GAME", 40.0f);
			} else {
				ChangeScene("PARTICLEDEBUG", 40.0f);
			}
		}
	} else {
		if (blackTime > 0.0f) {
			blackTime -= FPSKeeper::DeltaTime();
			if (blackTime <= 0.0f) {
				blackTime = 0.0f;
			}
		}
	}
	black_->SetColor({ 0.0f,0.0f,0.0f,Lerp(0.0f,1.0f,(1.0f / blackLimmite * blackTime)) });
	XINPUT_STATE pad;
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		if (blackTime == 0.0f) {
			isChangeFase = true;
		}
	} else if (Input::GetInstance()->GetGamepadState(pad)) {
		if (Input::GetInstance()->TriggerButton(PadInput::A)) {
			if (blackTime == 0.0f) {
				isChangeFase = true;
			}
		}
	}
#ifdef _DEBUG
	if (Input::GetInstance()->PushKey(DIK_RETURN) && Input::GetInstance()->PushKey(DIK_P) && Input::GetInstance()->PushKey(DIK_D) && Input::GetInstance()->TriggerKey(DIK_S)) {
		if (blackTime == 0.0f) {
			isChangeFase = true;
			isParticleDebugScene_ = true;
		}
	}
#endif // _DEBUG
}

void TitleScene::ApplyGlobalVariables() {


}
