#include "GameScene.h"
#include"Camera/CameraManager.h"

#include "Particle/ParticleManager.h"
#include "Scene/SceneManager.h"

#include<imgui.h>


GameScene::GameScene() {}

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	Init();

	///-------------------------------------------------------------
	///　生成
	///-------------------------------------------------------------- 
	obj3dCommon = std::make_unique<Object3dCommon>();
	player_ = std::make_unique<Player>();
	field_ = std::make_unique<Field>();
	skydome_ = std::make_unique<SkyDome>();
	gameCamera_ = std::make_unique<GameCamera>();

	///-------------------------------------------------------------
	///　初期化
	///-------------------------------------------------------------- 
	obj3dCommon->Initialize();
	field_->Initialize();
	skydome_->Initialize();
	player_->Initialize();
	gameCamera_->Initialize();

	gameCamera_->SetTarget(&player_->GetTrans());
	

}

void GameScene::Update() {

	Debug();

	///-------------------------------------------------------------
	///　更新
	///-------------------------------------------------------------- 
	field_->Update();
	skydome_->Update();
	player_->Update();
	gameCamera_->Update();

	ParticleManager::GetInstance()->Update();

}

void GameScene::Draw() {

#pragma region 背景描画

	///-------------------------------------------------------------
	///　背景描画
	///--------------------------------------------------------------

	dxCommon_->ClearDepthBuffer();
#pragma endregion


#pragma region 3Dオブジェクト
	obj3dCommon->PreDraw();

	///-------------------------------------------------------------
	///　モデル描画
	///-------------------------------------------------------------- 
	skydome_->Draw();
	field_->Draw();
	player_->Draw();


	ParticleManager::GetInstance()->Draw();

#pragma endregion


#pragma region 前景スプライト

	dxCommon_->PreSpriteDraw();

	///-------------------------------------------------------------
	///　前景スプライト描画
	///-------------------------------------------------------------- 


#pragma endregion

}

void GameScene::Debug() {
	ImGui::Begin("PositionDebug");
	player_->Debug();
	ImGui::End();

}