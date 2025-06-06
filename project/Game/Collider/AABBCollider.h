#pragma once
#include <array>
#include "Game/Collider/BaseCollider.h"


class AABBCollider : public BaseCollider {
public:
	AABBCollider();
	~AABBCollider()override = default;

public:

	void DebugGUI();

	// stateによって呼び出す関数を決める
	void OnCollision(const ColliderInfo& other);

	// 衝突判定
	void OnCollisionEnter(const ColliderInfo& other)override;
	void OnCollisionStay(const ColliderInfo& other)override;
	void OnCollisionExit(const ColliderInfo& other)override;

	// イベントハンドラの設定
	void SetCollisionEnterCallback(std::function<void(const ColliderInfo&)> callback) {
		onCollisionEvents_[static_cast<int>(CollisionState::CollisionEnter)] = callback;
	}
	void SetCollisionStayCallback(std::function<void(const ColliderInfo&)> callback) {
		onCollisionEvents_[static_cast<int>(CollisionState::CollisionStay)] = callback;
	}
	void SetCollisionExitCallback(std::function<void(const ColliderInfo&)> callback) {
		onCollisionEvents_[static_cast<int>(CollisionState::CollisionExit)] = callback;
	}

	void SetState(CollisionState change) { state = change; }

	void SetWidth(float w) { width = w; }
	void SetHeight(float h) { height = h; }
	void SetDepth(float d) { depth = d; }


	CollisionState GetState()const { return state; }

	float GetWidth() const { return width; }
	float GetHeight() const { return height; }
	float GetDepth() const { return depth; }


#ifdef _DEBUG
	void Debug();
	void DrawCollider();
#endif // _DEBUG


private:



private:

	CollisionState state;

	std::array<std::function<void(const ColliderInfo&)>, 3> onCollisionEvents_;

	float width = 1.0f;
	float height = 1.0f;
	float depth = 1.0f;

};