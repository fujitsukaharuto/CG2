#include "SkyDome.h"

#include "assert.h"


SkyDome::SkyDome() {}

SkyDome::~SkyDome() {}

void SkyDome::Initialize() {

	OriginGameObject::Initialize();
	OriginGameObject::SetModel("SkyDome.obj");

	model_->transform.scale = { 500,500,500 };

}

void SkyDome::Update() {
	OriginGameObject::Update();
}


void SkyDome::Draw(Material* material) {
	OriginGameObject::Draw(material);
}
