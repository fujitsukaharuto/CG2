#pragma once
#include "Audio.h"
#include "DXCom.h"
#include "GameScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "SRVManager.h"
#include "MyWindow.h"
#include "GlobalVariables.h"
#include "ModelManager.h"
#include "PointLightManager.h"
#include "CameraManager.h"
#include "Particle/ParticleManager.h"
#include "Scene/SceneManager.h"

class Framework {
public:
	Framework();
	virtual ~Framework() = default;

public:

	virtual void Initialize();
	virtual void Finalize();
	virtual void Update();
	virtual void Draw() = 0;
	virtual bool IsEndRequest() { return endRequest_; }

	void Init();
	void Run();


private:


protected:

	bool endRequest_ = false;

	MyWin* win_ = nullptr;
	DXCom* dxCommon_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	// 汎用
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;
	FPSKeeper* fpsKeeper_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	CameraManager* cameraManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	PointLightManager* pointLightManager_ = nullptr;
	ParticleManager* pManager_ = nullptr;

};