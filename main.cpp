#include "Audio.h"
#include "DXCom.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "SRVManager.h"
#include "MyWindow.h"
#include "GlobalVariables.h"
#include "ModelManager.h"
#include "PointLightManager.h"
#include "CameraManager.h"
#include "Rendering/PrimitiveDrawer.h"
#include "ParticleManager.h"


// やること
// オフスクリーンのかんり
// モデルマネージャー、スプライトマネージャー
// wave対応
// クラス分け



int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	MyWin* win = nullptr;
	DXCom* dxCommon = nullptr;
	SRVManager* srvManager = nullptr;
	// 汎用
	Input* input = nullptr;
	Audio* audio = nullptr;
	FPSKeeper* fpsKeeper = nullptr;
	SceneManager* sceneManager = nullptr;
	TextureManager* textureManager = nullptr;
	PrimitiveDrawer* primitiveDrawer = nullptr;
	ModelManager* modelManager = nullptr;

	PointLightManager* pointLightManager = nullptr;
	ParticleManager* pManager = nullptr;


	// ゲームウィンドウの作成
	win = MyWin::GetInstance();
	win->Initialize();

	// DirectX初期化
	dxCommon = DXCom::GetInstance();
	dxCommon->Initialize(win);

	srvManager = SRVManager::GetInstance();
	srvManager->Initialize();

	fpsKeeper = FPSKeeper::GetInstance();
	fpsKeeper->Initialize();

	DebugCamera* camera = nullptr;
	camera = DebugCamera::GetInstance();
	camera->Initialize();

	CameraManager* cameraManager = nullptr;
	cameraManager = CameraManager::GetInstance();
	cameraManager->Initialize();


#pragma region 汎用機能初期化
	// ImGuiの初期化
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Init(win,dxCommon);

	// 入力の初期化
	input = Input::GetInstance();
	input->Initialize();

	// オーディオの初期化
	audio = Audio::GetInstance();
	audio->Initialize();

	audio->LoadWave("testLongBGM.wav");
	audio->LoadWave("jump.wav");
	audio->LoadWave("damage.wav");




	// 描画用リソースの管理の初期化
	textureManager = TextureManager::GetInstance();
	modelManager = ModelManager::GetInstance();

	pointLightManager = PointLightManager::GetInstance();
	pointLightManager->AddPointLight();
	pointLightManager->AddSpotLight();

	// Particle作成
	pManager = ParticleManager::GetInstance();
	pManager->Initialize(dxCommon, srvManager);
	pManager->CreateParticleGroup("test", "uvChecker.png");
	pManager->CreateParticleGroup("noteChange", "bakuha.png");
	

	pManager->CreateAnimeGroup("animetest", "uvChecker.png");
	pManager->AddAnime("animetest", "white2x2.png", 10.0f);

	pManager->CreateAnimeGroup("bossHit", "explosion1.png");
	pManager->AddAnime("bossHit", "explosion3.png", 8.0f);
	pManager->AddAnime("bossHit", "explosion4.png", 16.0f);
	pManager->AddAnime("bossHit", "explosion5.png", 24.0f);
	pManager->AddAnime("bossHit", "explosion6.png", 32.0f);


	pManager->CreateAnimeGroup("playerHit", "playerDamage01.png");
	pManager->AddAnime("playerHit", "playerDamage02.png", 5.0f);
	pManager->AddAnime("playerHit", "playerDamage03.png", 10.0f);
	pManager->AddAnime("playerHit", "playerDamage04.png", 15.0f);



	primitiveDrawer = PrimitiveDrawer::GetInstance();

#pragma endregion

	dxCommon->SettingTexture();

	GlobalVariables::GetInstance()->LoadFiles();

	sceneManager = SceneManager::GetInstance();
	sceneManager->Initialize();
	sceneManager->StartScene("TITLE");



	modelManager->LoadOBJ("ground.obj");
	modelManager->LoadOBJ("player.obj"); 
	modelManager->LoadOBJ("boss.obj");
	modelManager->LoadOBJ("chorus.obj");
	modelManager->LoadOBJ("enemy.obj");
	modelManager->LoadOBJ("note.obj");
	modelManager->LoadOBJ("obstacle.obj");
	modelManager->LoadOBJ("chainNote.obj");
	modelManager->LoadOBJ("verticalBar.obj");
	modelManager->LoadOBJ("horizontalBar.obj");


	textureManager->Load("curtain.png");


	//BYTE keys[256] = { 0 };
	//BYTE preKeys[256] = { 0 };

	//ウィンドウのxボタンが押されるまでループ
	while (!win->ProcessMessage())
	{

		fpsKeeper->Update();

#ifdef _DEBUG
		// ImGui受付
		imguiManager->Begin();
#endif // _DEBUG

		// 入力関連の毎フレーム処理
		input->Update();

		cameraManager->Update();
#ifdef _DEBUG
		if (input->TriggerKey(DIK_F12)) {
			if (cameraManager->GetDebugMode()) {
				cameraManager->SetDebugMode(false);

			}
			else {
				cameraManager->SetDebugMode(true);

			}
		}

		GlobalVariables::GetInstance()->Update();

#endif // _DEBUG

		

		// ゲームシーンの毎フレーム処理
		sceneManager->Update();

		// ImGui受付
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();
		// ゲームシーンの描画
		sceneManager->Draw();
		dxCommon->Command();
		dxCommon->PostEffect();
		// ImGuiの描画
		imguiManager->Draw();
		// 描画終了
		dxCommon->PostDraw();
	}

	// 解放
	sceneManager->Finalize();


	primitiveDrawer->Finalize();
	
	audio->Finalize();
	imguiManager->Fin();
	pManager->Finalize();
	textureManager->Finalize();
	modelManager->Finalize();
	srvManager->Finalize();

	// ゲームウィンドウの破棄
	win->Finalize();

	return 0;
}

