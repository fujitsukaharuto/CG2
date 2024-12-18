#pragma once
#include "Scene/BaseScene.h"


class GameScene :public BaseScene
{
public:
	GameScene();
	~GameScene();

	void Initialize()override;

	void Update()override;

	void Draw()override;

private:

	void ApplyGlobalVariables();//値読み込みテスト用今度Objectクラス作って継承で使えるようにする

	std::unique_ptr<Object3dCommon> obj3dCommon = nullptr;

	std::unique_ptr<Object3d> sphere = nullptr;
	std::unique_ptr<Object3d> suzunne = nullptr;
	std::unique_ptr<Object3d> fence = nullptr;
	std::unique_ptr<Object3d> terrain = nullptr;

	std::vector<std::unique_ptr<Object3d>> suzunnes;

	std::unique_ptr<Material> mate = nullptr;

	std::unique_ptr<Sprite> test = nullptr;
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
	Vector3 rightDir = { 1.0f,0.0f,0.0f };

	float spherePara = 10;
	Vector3 spherevec = { 0.0f,1.0f,0.0f };

	float fencePara = 10;
	Vector3 fencevec = { -2.0f,-1.0f,1.0f };

	bool isDebugCameraMode_ = false;


	SoundData soundData1;
	SoundData soundData2;

	ParticleEmitter emit;

};
