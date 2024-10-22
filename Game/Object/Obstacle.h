#pragma once
#include "Character.h"
#include "Object3d.h"

class Obstacle :
	public Character{
public:

	Obstacle();
	~Obstacle() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3d* model)override;
	void Initialize(Object3d* model, const Vector3& initPos);

	/// <summary>
	/// 更新
	/// </summary>
	void Update()override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw()override;

	bool GetIsAlive()const{ return isAlive_; }

	/// <summary>
	/// 衝突時反応
	/// </summary>
	virtual void OnCollision([[maybe_unused]] Character* othre){};

	/// <summary>
	/// モデルの中心座標を取得
	/// </summary>
	/// <returns></returns>
	virtual Vector3 GetCenterPos()const override;

	/// <summary>
	/// 自身のシリアルナンバーを取得
	/// </summary>
	/// <returns></returns>
	uint32_t GetSerialNumber()const{ return serialNumber_; }

private:
	uint32_t lifeTime_ = 180;

	bool isAlive_ = true;

	//シリアルナンバー
	uint32_t serialNumber_ = 0;
	static uint32_t nextSerialNumber;
};
