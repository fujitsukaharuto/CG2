#pragma once
#include "Collision/Collider.h"

class SphereCollider 
    : public Collider{
public:
    SphereCollider() = default;
    ~SphereCollider()override{};

    /// <summary>
    /// 基本的な判定
    /// </summary>
    /// <param name="other"></param>
    /// <returns></returns>
    bool Intersects(const Collider& other) const override;

    /// <summary>
    /// 座標の更新
    /// </summary>
    void Update(const Vector3& pos)override;

    // Sphere vs Sphere の衝突判定
    bool IntersectsSphere(const SphereCollider& other) const override;

    // Sphere vs AABB の衝突判定
    bool IntersectsBox(const BoxCollider& other) const override;

public:
    /////////////////////////////////////////////////////////////////////////////////////////
    //				アクセッサ
    /////////////////////////////////////////////////////////////////////////////////////////
    const float GetRadius()const{ return radius_; }
    void SetRadius(const float radius){ radius_ = radius; }

    const Vector3& GetPosition()const{ return position_; }
    void SetPosition(const Vector3& position){ position_ = position; }

    const Vector3& GetCollisionNormal()const{ return collisionNormal_; }
    void SetCollisionNormal(const Vector3& normal){ collisionNormal_ = normal; }

public:
    float radius_;

    Vector3 position_;

    mutable Vector3 collisionNormal_; // mutableを使用して、constメソッド内でも変更可能に
};