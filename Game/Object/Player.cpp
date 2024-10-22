#include "Object/Player.h"
#include "Input.h"

#include "Collision/CollisionManager.h"
#include "Collision/SphereCollider.h"
#include "Collision/BoxCollider.h"
#include "GlobalVariables/GlobalVariables.h"
#include "Object/Boss.h"
#include "Object/Obstacle.h"

#include "Field/Field.h"

#include <algorithm>
#undef max
#undef min

Player::Player() : Character(std::make_unique<SphereCollider>()){
    SphereCollider* sphereCollider = dynamic_cast< SphereCollider* >(collider_.get());
    if (sphereCollider){
        sphereCollider->radius_ = 1.0f;
        sphereCollider->position_ = {100.0f, 0.0f, 0.0f};
    }

    //識別id
    sphereCollider->SetTypeID(static_cast< uint32_t >(CollisionTypeIdDef::kPlayer));
    CollisionManager::GetInstance()->AddCollider(this);


    //初期値として5に設定
    life_ = 5;

    //調整項目
    const char* groupName = "player";
    GlobalVariables::GetInstance()->CreateGroup(groupName);
    GlobalVariables::GetInstance()->AddItem(groupName, "life", life_);
}

void Player::Initialize(std::vector<Object3d*> Object3ds){
    Character::Initialize(Object3ds);

    junpSE_ = Audio::GetInstance()->SoundLoadWave("jump.wav");
    damageSE_ = Audio::GetInstance()->SoundLoadWave("damage.wav");

    emit.name = "playerHit";
    emit.count = 1;
    emit.animeData.lifeTime = 20;
    emit.RandomSpeed({-0.00f,0.00f}, {-0.00f,0.00f}, {0.0f,0.0f});
    emit.RandomTranslate({-0.13f,-0.13f}, {0.0f,0.0f}, {-1.7f,-1.7f});
    emit.animeData.startSize = {2.0f,2.0f};
    emit.animeData.endSize = {2.0f,2.0f};

    const char* groupName = "player";
    life_ = GlobalVariables::GetInstance()->GetIntValue(groupName,"life");
}

void Player::Update(){
    collider_->Update(GetCenterPos());

    // ノックバック中の処理
    if (isKnockedBack_){
        knockbackTimer_ -= 0.016f; // タイマーを減らす (例: 60FPSで1フレーム = 0.016秒)

        // 重力を適用
        const float kGravity = -1.0f; // 重力の値
        const float kDeltaTime = 0.016f; // フレーム間の時間 (60FPS想定)
        velocity_.y += kGravity * kDeltaTime; // 重力をY軸方向に加算

        // Z軸の回転を追加 (ノックバック中のみ)
        const float kRotationSpeed = 0.16f; // Z軸回転の速度
        models_[0]->transform.rotate.z += kRotationSpeed; // Z軸を回転

        // 地面に着いたらノックバックを終了
        if (models_[0]->transform.translate.y <= 0.0f && !isKnockedBack_){
            models_[0]->transform.translate.y = 0.0f;
            velocity_.y = 0.0f; // Y軸速度をリセットして地面に着いた状態にする
            isKnockedBack_ = false; // ノックバックを終了
            models_[0]->transform.rotate.z = 0.0f; // Z軸の回転をリセット
        }


        // ノックバックが終了したら通常状態に戻す
        if (knockbackTimer_ <= 0.0f){
            isKnockedBack_ = false;
            record_.Clear();//接触履歴をリセット

            knockbackTimer_ = 0.0f;
            models_[0]->transform.rotate.z = 0.0f; // Z軸の回転をリセット
        }
    } else{
        // 通常の動作 (ジャンプと移動)
        this->Jump();
        this->Move();
    }

    models_[0]->transform.translate += velocity_;

    //地面より下に行かないように
    models_[0]->transform.translate.y = std::max(models_[0]->transform.translate.y,0.0f);

    //生存フラグの管理
    Character::Update();
}



void Player::Draw(){

    if (isAlive_){
        Character::Draw();
    }
}

void Player::Move(){
    if (!isKnockedBack_){  // ノックバック中は通常の移動を無効化
        if (Input::GetInstance()->PushKey(DIK_A)){
            moveSpeed_ = -0.2f;
        } else if (Input::GetInstance()->PushKey(DIK_D)){
            moveSpeed_ = 0.2f;
        } else{
            moveSpeed_ = 0.0f;
        }

        velocity_.x = moveSpeed_;
    }
}

void Player::Jump(){
    const float kGravity = -1.0f; // 要調整
    const float kDeltaTime = 0.016f;

    // ジャンプ開始
    if (Input::GetInstance()->TriggerKey(DIK_SPACE)){
        isJumping_ = true;
        velocity_.y = 0.3f;
        Audio::GetInstance()->SoundPlayWave(junpSE_);
    }

    // ジャンプ中の挙動
    if (isJumping_){
        velocity_.y += kGravity * kDeltaTime;

        // 地面に着いたらジャンプリセット
        if (models_[0]->transform.translate.y < 0.0f){
            models_[0]->transform.translate.y = 0.0f;
            isJumping_ = false;
            velocity_ = {0.0f, 0.0f, 0.0f};
        }
    }
}

Vector3 Player::GetCenterPos() const{
    Vector3 offset = {0.0f, 0.5f, 0.0f};
    Vector3 worldPos = Transform(offset, models_[0]->GetMatWorld());
    return worldPos;
}


void Player::OnCollision(Character* other){
    uint32_t collisionType = other->GetCollider()->GetTypeID();

    // 音符に変わる敵と衝突したとき
    if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kNoteEnemy)){
        SphereCollider* enemyCollider = static_cast< SphereCollider* >(other->GetCollider());
        SphereCollider* playerCollider = static_cast< SphereCollider* >(this->GetCollider());

        Vector3 playerPos = playerCollider->GetPosition();
        Vector3 enemyPos = enemyCollider->GetPosition();
        float playerRadius = playerCollider->GetRadius();
        float enemyRadius = enemyCollider->GetRadius();

        // プレイヤーがまだ空中にいるかどうかのチェック
        if (isJumping_ && enemyPos.y > 0.5f){
            Vector3 collisionNormal = playerPos - enemyPos;
            float distance = collisionNormal.Lenght();

            collisionNormal.Normalize();

            // 反射時の速度減衰率
            const float kReflectionDamping = 0.5f;

            Vector3 playerVelocity = velocity_;

            // 速度ベクトルを反射し、減衰させる
            Vector3 reflectedVelocity = playerVelocity - 2 * collisionNormal * playerVelocity.Dot(collisionNormal);
            reflectedVelocity *= kReflectionDamping;

            // Y軸方向の速度が大きくなりすぎないように上限を設定
            const float kMaxVelocityY = 0.3f;
            reflectedVelocity.y = std::min(reflectedVelocity.y, kMaxVelocityY);

            // プレイヤーの速度を更新
            velocity_ = reflectedVelocity;

            // プレイヤーを敵から少し離れた位置に移動させてめり込みを防止
            float penetrationDepth = (playerRadius + enemyRadius) - distance;
            playerPos += collisionNormal * (penetrationDepth * 0.5f);
            playerCollider->SetPosition(playerPos);
        }
    }

    // ボスと衝突したときは常にノックバック
    else if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kBoss)){
        Boss* boss = static_cast< Boss* >(other);
        SphereCollider* bossCollider = static_cast< SphereCollider* >(other->GetCollider());
        SphereCollider* playerCollider = static_cast< SphereCollider* >(this->GetCollider());

        uint32_t serialNum = boss->GetSerialNumber();

        if (record_.CheckRecord(serialNum)){
            return;
        }

        // 履歴に登録
        record_.AddRecord(serialNum);

        Vector3 playerPos = playerCollider->GetPosition();
        Vector3 bossPos = bossCollider->GetPosition();

        // プレイヤーがボスと衝突した際に右方向（+X方向）に飛ばす
        const float kBossCollisionXVelocity = 0.2f;  // 右方向に飛ばす速度
        const float kBossCollisionYVelocity = 0.2f;  // 上方向に飛ばす速度

        // X方向の速度を設定して右に飛ばす
        velocity_.x = kBossCollisionXVelocity;
        // Y方向の速度も設定してジャンプさせる
        velocity_.y = kBossCollisionYVelocity;

        // プレイヤーをボスから少し右に押し出す
        playerPos.x += 0.5f;  // 右方向に押し出す距離を調整
        playerCollider->SetPosition(playerPos);

        // ノックバック状態に設定
        isKnockedBack_ = true;
        knockbackTimer_ = 0.5f;  // ノックバックの継続時間を設定

        life_--;
        Audio::GetInstance()->SoundPlayWave(damageSE_);
        emit.pos = GetCenterPos();
        emit.BurstAnime();
    }


    // 障害物と衝突したとき
    else if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kObstacle)){
        Obstacle* obstacle = static_cast< Obstacle* >(other);
        SphereCollider* obstacleCollider = static_cast< SphereCollider* >(other->GetCollider());
        SphereCollider* playerCollider = static_cast< SphereCollider* >(this->GetCollider());

        uint32_t serialNum = obstacle->GetSerialNumber();

        if (record_.CheckRecord(serialNum)){
            return;
        }

        // 履歴に登録
        record_.AddRecord(serialNum);

        Vector3 playerPos = playerCollider->GetPosition();
        Vector3 obstaclePos = obstacleCollider->GetPosition();

        // プレイヤーと障害物のX座標の差を計算
        float xDifference = playerPos.x - obstaclePos.x;

        // ノックバックの方向を決定（右から衝突：右に、左から衝突：左に飛ばす）
        float direction = (xDifference > 0) ? 1.0f : -1.0f;

        const float kObstacleCollisionXVelocity = 0.2f * direction;  // 水平方向の速度
        const float kObstacleCollisionYVelocity = 0.2f;  // 上方向の速度

        // X方向の速度を設定
        velocity_.x = kObstacleCollisionXVelocity;
        // Y方向の速度も設定
        velocity_.y = kObstacleCollisionYVelocity;

        // プレイヤーを障害物から少し押し出す（右方向または左方向）
        playerPos.x += 0.5f * direction;
        playerCollider->SetPosition(playerPos);

        // ノックバック状態に設定
        isKnockedBack_ = true;
        knockbackTimer_ = 0.5f;  // ノックバックの継続時間を設定

        life_--;
        Audio::GetInstance()->SoundPlayWave(damageSE_);
        emit.pos = GetCenterPos();
        emit.BurstAnime();
    }
}