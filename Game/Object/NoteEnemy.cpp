#include"Object/NoteEnemy.h"
#include "Object/NoteEnemyState/NoteEnemyState_Enemy.h"

#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"

#include "Collision/CollisionManager.h"
#include "Object/Boss.h"
#include "Object/EnemyManager.h"
#include "Field/Field.h"

#include "GlobalVariables/GlobalVariables.h"

uint32_t NoteEnemy::nextSerialNumber = 0;

NoteEnemy::NoteEnemy() : Character(std::make_unique<SphereCollider>()){
	serialNumber_ = nextSerialNumber;
	++nextSerialNumber;

	SphereCollider* sphereCollider = dynamic_cast< SphereCollider* >(collider_.get());
	if (sphereCollider){
		sphereCollider->radius_ = 1.0f;
	}

	sphereCollider->SetTypeID(static_cast< uint32_t >(CollisionTypeIdDef::kNoteEnemy));
	CollisionManager::GetInstance()->AddCollider(this);
}

NoteEnemy::~NoteEnemy(){}

void NoteEnemy::Initialize(Object3d* model){
	Character::Initialize(model);
	moveSpeed_ = 1.0f * Field::influenceOnSpeed_[fieldIndex_];

	//最初の状態をセット
	currentState_ = std::make_unique<NoteEnemyState_Enemy>(this);
}

void NoteEnemy::Initialize(std::vector<Object3d*> models, const Vector3& initPos){
	Character::Initialize(models);
	models_[0]->transform.translate = initPos;

	moveSpeed_ = moveSpeed_ * Field::influenceOnSpeed_[fieldIndex_];

	//最初の状態をセット
	currentState_ = std::make_unique<NoteEnemyState_Enemy>(this);
}

void NoteEnemy::Initialize(Object3d* model, const Vector3& initPos){
	Character::Initialize(model);
	models_[0]->transform.translate = initPos;
	models_[0]->transform.rotate = { -1.57f,1.57f,0.0f };
	models_[0]->UpdateWorldMat();

	//座標を設定
	collider_->Update(GetWorldPosition());

	moveSpeed_ = moveSpeed_ * Field::influenceOnSpeed_[fieldIndex_];

	//最初の状態をセット
	currentState_ = std::make_unique<NoteEnemyState_Enemy>(this);


	emit.name = "noteChange";
	emit.count = 2;
	emit.grain.lifeTime_ = 20;
	emit.RandomSpeed({-0.07f,0.07f}, {-0.00f,0.04f}, {-0.002f,0.000f});
	emit.RandomTranslate({-0.1f,0.1f}, {-0.1f,0.1f}, {-1.5f,-1.0f});
	emit.grain.transform.scale = {2.5f,2.5f,1.0f};

	const char* groupName = "noteEnemy";
	GlobalVariables::GetInstance()->CreateGroup(groupName);
	GlobalVariables::GetInstance()->AddItem(groupName, "noteSpeedMagnification", noteSpeedMagnification_);
}

void NoteEnemy::Update(){
#ifdef _DEBUG
	ApplyGlobalVariabls();
#endif // _DEBUG


	collider_->Update(GetWorldPosition());

	//移動速度の更新
	moveSpeed_ = Field::influenceOnSpeed_[fieldIndex_];

	//音符に変わったらモデルを変える(一時的に色を変えている)
	if (isChangedNote_){
		if (!isChainEnemy_){
			models_[0]->SetModel("note.obj");
		}
		models_[0]->transform.rotate = {0.0f,-1.5f,0.0f};

		
		if (isChanegeEffect_){
			emit.pos = GetCenterPos();
			emit.Burst();
			isChanegeEffect_ = false;
		}
	}

	//状態の更新
	if (currentState_){
		currentState_->Update();
	}

}

void NoteEnemy::Draw(){
	if (!isRemoved_){
		Character::Draw();
	}
}

void NoteEnemy::OnCollision(Character* other){
	uint32_t collisionType = other->GetCollider()->GetTypeID();

	// 衝突相手がnoteEnemyなら早期return
	if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kNoteEnemy)){
		return;
	}

	else if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kPlayer)){
		// プレイヤーのスフィアコライダーを取得
		SphereCollider* player = static_cast< SphereCollider* >(other->GetCollider());
		// プレイヤーと敵の位置
		Vector3 playerPos = player->GetPosition();

		SphereCollider* enemy = static_cast< SphereCollider* >(collider_.get());
		Vector3 enemyPos = enemy->GetPosition();

		bool movedToNewField = false; // 段が移動したかどうかを確認

		// プレイヤーが敵の上にある場合（敵を踏んだ場合）
		if (playerPos.y > enemyPos.y){
			if (fieldIndex_ > 0){
				fieldIndex_--;  // 下の段に移動
			}
			movedToNewField = true;
		}
		// プレイヤーが敵の下にある場合（下から当たった場合）
		else if (playerPos.y < enemyPos.y){
			if (fieldIndex_ < Field::staffNotation_.size() - 1 && fieldIndex_ != 0){
				fieldIndex_++;  // 上の段に移動
			}
			movedToNewField = true;
		}

		// 段が移動した場合、敵を新しい段に移動
		if (movedToNewField){
			Vector3 movedPos = {
				enemyPos.x,
				Field::staffNotation_[fieldIndex_]->transform.translate.y,
				enemyPos.z
			};
			SetTranslate(movedPos);

			// 段が移動した、または上下の面に触れた場合に音符に変わる
			isChangedNote_ = true;
		}
	}

	// 衝突相手が障害物の時音符状態なら敵状態に戻す
	else if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kBoss)){

		// 音符に代わっているときにボスと衝突したら消す
		if (isChangedNote_){
			isChangedNote_ = false;

		}

	}

	// 衝突相手がbossなら消去する
	else if (collisionType == static_cast< uint32_t >(CollisionTypeIdDef::kBoss)){

		// 音符に代わっているときにボスと衝突したら消す
		if (isChangedNote_){
			isRemoved_ = true;

			//bossの中心で止まるようにする(つながっている敵用)
			if (GetWorldPosition().x <= other->GetCenterPos().x){
				moveSpeed_ = 0.0f;

				//敵のx座標で止める
				SetTranslate({other->GetWorldPosition().x,this->GetWorldPosition().y,0.0f});
			}

		}

	}
	
	
}


void NoteEnemy::ChangeState(std::unique_ptr<NoteEnemyState_Base> newState){
	currentState_ = std::move(newState);
	if (currentState_){
		currentState_->Initialize();
	}
}

void NoteEnemy::Move(){
	const float kDeltaTime = 0.017f;
	if (isChangedNote_){
		models_[0]->transform.translate.x -= moveSpeed_ * noteSpeedMagnification_ * kDeltaTime;
	} else{
		models_[0]->transform.translate.x -= moveSpeed_  * kDeltaTime;
	}
}

Vector3 NoteEnemy::GetCenterPos() const{
	Vector3 offset = {0.0f, 0.5f, 0.0f};
	Vector3 worldPos = Transform(offset, models_[0]->GetMatWorld());
	return worldPos;
}

void NoteEnemy::ApplyGlobalVariabls(){
	const char* groupName = "noteEnemy";
	noteSpeedMagnification_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "noteSpeedMagnification");

}


