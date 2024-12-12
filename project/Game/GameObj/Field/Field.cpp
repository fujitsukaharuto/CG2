#include "Field.h"

#include "assert.h"


Vector3 Field::baseScale_ = {70,1,70};

Field::Field() {}

Field::~Field() {}

void Field::Initialize() {

	OriginGameObject::Initialize();
	OriginGameObject::SetModel("Field.obj");
	
	model_->transform.scale = baseScale_;
}

void Field::Update() {
	OriginGameObject::Update();
}


void Field::Draw(Material*material) {
	OriginGameObject::Draw(material);
}
