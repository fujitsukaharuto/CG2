#pragma once
#include "Character.h"
#include "ParticleEmitter.h"
#include "Object/NoteEnemyState/NoteEnemyState_Base.h"

/// <summary>
/// 音符になる敵
/// </summary>
class NoteEnemy
	:public Character{
public:
	NoteEnemy();
	~NoteEnemy()override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="models"></param>
	void Initialize(Object3d* model)override;
	void Initialize(std::vector<Object3d*> models, const Vector3& initPos);

	void Initialize(Object3d* model, const Vector3& initPos);

	/// <summary>
	/// 更新
	/// </summary>
	void Update()override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw()override;

	/// <summary>
	/// 衝突時の応答
	/// </summary>
	/// <param name="other"></param>
	void OnCollision([[maybe_unused]] Character* other)override;

	/// <summary>
	/// 状態を遷移
	/// </summary>
	/// <param name="newState"></param>
	void ChangeState(std::unique_ptr<NoteEnemyState_Base> newState);

	/// <summary>
	/// 移動
	/// </summary>
	void Move();
	
	/// <summary>
	/// 音符に変わったか
	/// </summary>
	/// <returns></returns>
	bool GetIsChangedNote()const{ return isChangedNote_; }

	void MarkForRemoval(){isRemoved_ = true;}

	bool GetIsRemoved(){ return isRemoved_; }

	Vector3 GetCenterPos()const override;

	Object3d* GetObject3d()const{ return models_[0]; }

	void SetFieldIndex(const uint32_t index){ fieldIndex_ = index; }

	void ApplyGlobalVariabls();

private:
	//音符用モデル
	Object3d* noteModel_ = nullptr;

	//音符に変わったかどうか
	bool isChangedNote_;

	// effect
	bool isChanegeEffect_ = true;

	//移動速度
	float moveSpeed_ = 1.0f;

	//音符になったときのスピード(移動速度*倍率)
	float noteSpeedMagnification_ = 1.3f;

	uint32_t fieldIndex_;

	//判定用aabbのサイズ
	Vector3 size_ = {1.0f,1.0f,1.0f};

	//現在の状態
	std::unique_ptr<NoteEnemyState_Base> currentState_ = nullptr;

	//リストから削除フラグ
	bool isRemoved_ = false;

	ParticleEmitter emit;
};