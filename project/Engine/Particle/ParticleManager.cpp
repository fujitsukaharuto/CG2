#include "ParticleManager.h"
#include "DXCom.h"
#include "SRVManager.h"
#include "CameraManager.h"
#include "Particle.h"
#include "Random.h"
#include "FPSKeeper.h"



ParticleManager::ParticleManager() {
}

ParticleManager::~ParticleManager() {

}

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize(DXCom* dxcom, SRVManager* srvManager) {

	dxCommon_ = dxcom;
	srvManager_ = srvManager;
	this->camera_ = CameraManager::GetInstance()->GetCamera();

	vertex_.push_back({ {-1.0f,1.0f,0.0f,1.0f},{0.0f,0.0f},{0.0f,0.0f,-1.0f} });
	vertex_.push_back({ {-1.0f,-1.0f,0.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f,-1.0f} });
	vertex_.push_back({ {1.0f,-1.0f,0.0f,1.0f},{1.0f,1.0f},{0.0f,0.0f,-1.0f} });
	vertex_.push_back({ {1.0f,1.0f,0.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f,-1.0f} });

	index_.push_back(0);
	index_.push_back(3);
	index_.push_back(1);

	index_.push_back(1);
	index_.push_back(3);
	index_.push_back(2);


	vBuffer_ = DXCom::GetInstance()->CreateBufferResource(DXCom::GetInstance()->GetDevice(), sizeof(VertexDate) * vertex_.size());
	iBuffer_ = DXCom::GetInstance()->CreateBufferResource(DXCom::GetInstance()->GetDevice(), sizeof(uint32_t) * index_.size());

	VertexDate* vData = nullptr;
	vBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vData));
	std::memcpy(vData, vertex_.data(), sizeof(VertexDate) * vertex_.size());

	vbView.BufferLocation = vBuffer_->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<UINT>(sizeof(VertexDate) * vertex_.size());
	vbView.StrideInBytes = static_cast<UINT>(sizeof(VertexDate));

	uint32_t* iData = nullptr;
	iBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&iData));
	std::memcpy(iData, index_.data(), sizeof(uint32_t) * index_.size());

	ibView.BufferLocation = iBuffer_->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * index_.size());





}

void ParticleManager::Finalize() {
	for (auto& groupPair : particleGroups_) {

		delete groupPair.second;

	}
	particleGroups_.clear();
	for (auto& groupPair : animeGroups_) {

		for (auto& i : groupPair.second->objects_) {
			delete i;
		}
		groupPair.second->lifeTime.clear();
		groupPair.second->startLifeTime_.clear();
		groupPair.second->isLive_.clear();
		groupPair.second->accele.clear();
		groupPair.second->speed.clear();

		delete groupPair.second;

	}

}

void ParticleManager::Update() {


	Matrix4x4 billboardMatrix = MakeIdentity4x4();

	if (camera_) {
		const Matrix4x4& viewMatrix = camera_->GetViewMatrix();

		billboardMatrix.m[0][0] = viewMatrix.m[0][0];
		billboardMatrix.m[0][1] = viewMatrix.m[1][0];
		billboardMatrix.m[0][2] = viewMatrix.m[2][0];

		billboardMatrix.m[1][0] = viewMatrix.m[0][1];
		billboardMatrix.m[1][1] = viewMatrix.m[1][1];
		billboardMatrix.m[1][2] = viewMatrix.m[2][1];

		billboardMatrix.m[2][0] = viewMatrix.m[0][2];
		billboardMatrix.m[2][1] = viewMatrix.m[1][2];
		billboardMatrix.m[2][2] = viewMatrix.m[2][2];
	}


	for (auto& groupPair : particleGroups_) {

		ParticleGroup* group = groupPair.second;

		int particleCount = 0;
		group->drawCount_ = 0;
		for (auto& particle : group->particles_) {

			if (particle.lifeTime_ <= 0) {
				particle.isLive_ = false;
				continue;
			}

			particle.lifeTime_--;

			SizeType sizeType = SizeType(particle.type);
			float t = (1.0f - float(float(particle.lifeTime_) / float(particle.startLifeTime_)));
			switch (sizeType) {
			case kNormal:
				break;
			case kReduction:

				t = 1 - powf(1 - t, 4);
				particle.transform.scale.x = Lerp(particle.startSize.x, particle.endSize.x, t);
				particle.transform.scale.y = Lerp(particle.startSize.y, particle.endSize.y, t);

				break;
			case kExpantion:

				particle.transform.scale.x = Lerp(particle.startSize.x, particle.endSize.x, t);
				particle.transform.scale.y = Lerp(particle.startSize.y, particle.endSize.y, t);

				break;
			}

			particle.speed += particle.accele * FPSKeeper::DeltaTime();

			particle.transform.translate += particle.speed * FPSKeeper::DeltaTime();
			Matrix4x4 worldViewProjectionMatrix;
			Matrix4x4 worldMatrix = MakeIdentity4x4();

			if (!isBillBoard_) {
				worldMatrix = MakeAffineMatrix(particle.transform.scale, particle.transform.rotate, particle.transform.translate);
			}
			if (isBillBoard_) {
				worldMatrix = Multiply(MakeScaleMatrix(particle.transform.scale), MakeRotateXYZMatrix(particle.transform.rotate));
				worldMatrix = Multiply(worldMatrix, billboardMatrix);
				worldMatrix = Multiply(worldMatrix, MakeTranslateMatrix(particle.transform.translate));
			}

			if (camera_) {
				const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
				worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			}
			else {
				worldViewProjectionMatrix = worldMatrix;
			}

			group->instancingData_[particleCount].World = worldMatrix;
			group->instancingData_[particleCount].WVP = worldViewProjectionMatrix;

			particleCount++;
			group->drawCount_++;
		}
	}


	for (auto& groupPair : animeGroups_) {

		AnimeGroup* group = groupPair.second;
		for (int i = 0; i < group->objects_.size(); i++) {


			if (group->lifeTime[i] <= 0) {
				group->isLive_[i] = false;
				continue;
			}


			group->lifeTime[i] -= FPSKeeper::DeltaTime();
			group->animeTime[i] += FPSKeeper::DeltaTime();

			for (auto& animeChange : group->anime_) {
				if (group->animeTime[i] >= animeChange.second * FPSKeeper::DeltaTime()) {
					group->objects_[i]->SetTexture(animeChange.first);
				}
			}


			SizeType sizeType = SizeType(group->type);
			float t = (1.0f - float(float(group->lifeTime[i]) / float(group->startLifeTime_[i])));
			switch (sizeType) {
			case kNormal:
				break;
			case kReduction:

				group->objects_[i]->transform.scale.x = Lerp(group->startSize.x, group->endSize.x, t);
				group->objects_[i]->transform.scale.y = Lerp(group->startSize.y, group->endSize.y, t);

				break;
			case kExpantion:

				group->objects_[i]->transform.scale.x = Lerp(group->startSize.x, group->endSize.x, t);
				group->objects_[i]->transform.scale.y = Lerp(group->startSize.y, group->endSize.y, t);

				break;
			}

			group->speed[i] += group->accele[i] * FPSKeeper::DeltaTime();

			group->objects_[i]->transform.translate += group->speed[i] * FPSKeeper::DeltaTime();
			group->objects_[i]->SetBillboardMat(billboardMatrix);
		}
	}


}

void ParticleManager::Draw() {

	dxCommon_->GetDXCommand()->SetViewAndscissor();
	dxCommon_->GetPipelineManager()->SetPipeline(Pipe::Normal);
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& groupPair : animeGroups_) {

		AnimeGroup* group = groupPair.second;
		for (int i = 0; i < group->objects_.size(); i++) {
			if (group->isLive_[i]) {
				group->objects_[i]->AnimeDraw();
			}

		}
	}



	dxCommon_->GetDXCommand()->SetViewAndscissor();
	dxCommon_->GetPipelineManager()->SetPipeline(Pipe::particle);
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView);
	dxCommon_->GetCommandList()->IASetIndexBuffer(&ibView);

	for (auto& groupPair : particleGroups_) {
		ParticleGroup* group = groupPair.second;

		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, group->material_.GetMaterialResource()->GetGPUVirtualAddress());
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(group->srvIndex_));
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, group->material_.GetTexture()->gpuHandle);

		dxCommon_->GetCommandList()->DrawIndexedInstanced(6, group->drawCount_, 0, 0, 0);
	}

}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& fileName) {

	ParticleManager* instance = GetInstance();

	auto iterator = instance->particleGroups_.find(name);
	if (iterator != instance->particleGroups_.end()) {
		return;
	}


	ParticleGroup* newGroup = new ParticleGroup();
	newGroup->material_.SetTextureNamePath(fileName);
	newGroup->material_.CreateMaterial();


	newGroup->insstanceCount_ = 20;
	newGroup->instancing_ = instance->dxCommon_->CreateBufferResource(instance->dxCommon_->GetDevice(), (sizeof(TransformationParticleMatrix) * newGroup->insstanceCount_));
	newGroup->instancing_->Map(0, nullptr, reinterpret_cast<void**>(&newGroup->instancingData_));
	uint32_t max = newGroup->insstanceCount_;
	for (uint32_t index = 0; index < max; ++index) {
		newGroup->instancingData_[index].WVP = MakeIdentity4x4();
		newGroup->instancingData_[index].World = MakeIdentity4x4();
	}

	newGroup->srvIndex_ = instance->srvManager_->Allocate();
	instance->srvManager_->CreateStructuredSRV(newGroup->srvIndex_, newGroup->instancing_.Get(), newGroup->insstanceCount_, sizeof(TransformationParticleMatrix));


	//ここでパーティクルをあらかじめ作る
	float add = 0.1f;
	for (int i = 0; i < int(max); i++) {
		Particle p{};
		p.transform.scale = { 1.0f,1.0f,1.0f };
		p.transform.translate.x += add;
		p.transform.translate.y += add;
		newGroup->particles_.push_back(p);
		add += 0.1f;
	}


	instance->particleGroups_.insert(std::make_pair(name, newGroup));

}

void ParticleManager::CreateAnimeGroup(const std::string& name, const std::string& fileName) {

	ParticleManager* instance = GetInstance();

	auto iterator = instance->animeGroups_.find(name);
	if (iterator != instance->animeGroups_.end()) {
		return;
	}

	AnimeGroup* newGroup = new AnimeGroup();

	newGroup->farst = fileName;
	TextureManager::GetInstance()->LoadTexture(fileName);
	for (int i = 0; i < 6; i++) {
		Object3d* newobj = new Object3d();
		newobj->Create("plane.obj");
		newobj->SetTexture(fileName);


		newGroup->objects_.push_back(newobj);
		newGroup->lifeTime.push_back(0.0f);
		newGroup->animeTime.push_back(0.0f);
		newGroup->startLifeTime_.push_back(0.0f);
		newGroup->isLive_.push_back(false);
		newGroup->accele.push_back({ 0.0f,0.0f,0.0f });
		newGroup->speed.push_back({ 0.0f,0.0f,0.0f });

	}

	instance->animeGroups_.insert(std::make_pair(name, newGroup));

}

void ParticleManager::Emit(const std::string& name, const Vector3& pos, const Particle& grain, const RandomParametor& para, uint32_t count) {

	ParticleManager* instance = GetInstance();

	auto iterator = instance->particleGroups_.find(name);
	if (iterator != instance->particleGroups_.end()) {
		uint32_t newCount = 0;

		ParticleGroup* group = iterator->second;
		for (auto& particle : group->particles_) {

			if (particle.isLive_ == false) {
				particle.transform = grain.transform;
				particle.transform.translate = Random::GetVector3(para.transx, para.transy, para.transz);
				particle.transform.translate += pos;
				particle.speed = Random::GetVector3(para.speedx, para.speedy, para.speedz);
				particle.lifeTime_ = grain.lifeTime_;
				particle.startLifeTime_ = particle.lifeTime_;

				SpeedType type = SpeedType(grain.speedType);
				switch (type) {
				case kConstancy:
					particle.accele = grain.accele;
					break;
				case kDecele:
					particle.accele = (particle.speed) * -0.05f;
					break;
				case kAccele:
					particle.accele = (particle.speed) * 0.05f;
					break;
				}

				particle.type = grain.type;
				particle.startSize = grain.startSize;
				particle.endSize = grain.endSize;

				particle.isLive_ = true;
				newCount++;
			}
			if (newCount == count) {
				return;
			}
		}
	}
	else {
		return;
	}


}

void ParticleManager::EmitAnime(const std::string& name, const Vector3& pos, const AnimeData& data, const RandomParametor& para, uint32_t count) {


	ParticleManager* instance = GetInstance();

	auto iterator = instance->animeGroups_.find(name);
	if (iterator != instance->animeGroups_.end()) {
		uint32_t newCount = 0;


		AnimeGroup* group = iterator->second;
		group->speedType = data.speedType;
		group->type = data.type;
		group->startSize = data.startSize;
		group->endSize = data.endSize;
		for (int i = 0; i < group->objects_.size(); i++) {

			if (!group->isLive_[i]) {
				group->objects_[i]->transform.translate = Random::GetVector3(para.transx, para.transy, para.transz);
				group->objects_[i]->transform.translate += pos;
				group->speed[i] = Random::GetVector3(para.speedx, para.speedy, para.speedz);
				group->lifeTime[i] = data.lifeTime;
				group->startLifeTime_[i] = group->lifeTime[i];
				group->animeTime[i] = 0.0f;

				SpeedType type = SpeedType(group->speedType);
				switch (type) {
				case kConstancy:
					group->accele[i] = data.accele;
					break;
				case kDecele:
					group->accele[i] = (group->speed[i]) * -0.05f;
					break;
				case kAccele:
					group->accele[i] = (group->speed[i]) * 0.05f;
					break;
				}

				group->objects_[i]->SetTexture(group->farst);
				group->isLive_[i] = true;
				newCount++;
			}
			if (newCount == count) {
				return;
			}
		}
	}
	else {
		return;
	}


}

void ParticleManager::AddAnime(const std::string& name, const std::string& fileName, float animeChangeTime) {

	ParticleManager* instance = GetInstance();

	auto iterator = instance->animeGroups_.find(name);
	if (iterator != instance->animeGroups_.end()) {

		AnimeGroup* group = iterator->second;
		TextureManager::GetInstance()->LoadTexture(fileName);
		group->anime_.insert(std::make_pair(fileName, animeChangeTime));

	}
	else {
		return;
	}

}