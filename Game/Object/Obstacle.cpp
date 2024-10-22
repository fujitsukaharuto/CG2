#include "Object/Obstacle.h"

#include "Collision/CollisionManager.h"
#include "Collision/SphereCollider.h"


uint32_t Obstacle::nextSerialNumber = 0;

Obstacle::Obstacle() : Character(std::make_unique<SphereCollider>()){
	serialNumber_ = nextSerialNumber;
	++nextSerialNumber;

	SphereCollider* sphereCollider = dynamic_cast< SphereCollider* >(collider_.get());
	if (sphereCollider){
		sphereCollider->radius_ = 0.5f;
	}
	//識別id
	sphereCollider->SetTypeID(static_cast< uint32_t >(CollisionTypeIdDef::kObstacle));
	CollisionManager::GetInstance()->AddCollider(this);
}

void Obstacle::Initialize(Object3d* model){
	Character::Initialize(model);
}

void Obstacle::Initialize(Object3d* model, const Vector3& initPos){
	Character::Initialize(model);

	models_[0]->transform.translate = initPos;
	models_[0]->UpdateWorldMat();


}

void Obstacle::Update(){
	if (isAlive_){
		lifeTime_--;
		if (lifeTime_ <= 0){	//生きている時間
			isAlive_ = false;
		}
	}
	models_[0]->UpdateWorldMat();

	collider_->Update(GetCenterPos());
}

void Obstacle::Draw(){
	if (isAlive_){
		//モデルの描画
		Character::Draw();
	}
}

Vector3 Obstacle::GetCenterPos() const{
	Vector3 offset = {0.0f, 0.5f, 0.0f};
	Vector3 worldPos = Transform(offset, models_[0]->GetMatWorld());
	return worldPos;
}
