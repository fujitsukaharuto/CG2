#include "PipelineManager.h"

#include "Pipeline.h"
#include "ParticlePipeline.h"
#include "GrayPipeline.h"
#include "MetaBallPipeline.h"
#include "GaussPipeline.h"
#include "NonePipeline.h"
#include "Line3dPipe.h"



PipelineManager::~PipelineManager() {
	pipelines_.clear();
}

PipelineManager* PipelineManager::GetInstance() {
	static PipelineManager instance;
	return &instance;
}

void PipelineManager::Initialize() {
}

void PipelineManager::Finalize() {
}

void PipelineManager::CreatePipeline() {


	std::unique_ptr<Pipeline> pipeline = nullptr;
	std::unique_ptr<ParticlePipeline> particlePipline = nullptr;
	std::unique_ptr<GrayPipeline> grayPipeline = nullptr;
	std::unique_ptr<MetaBallPipeline> metaballPipeline = nullptr;
	std::unique_ptr<GaussPipeline> gaussPipeline = nullptr;
	std::unique_ptr<NonePipeline> nonePipeline = nullptr;
	std::unique_ptr<Line3dPipe> line3dPipe = nullptr;


	nonePipeline.reset(new NonePipeline());
	nonePipeline->Initialize();
	pipelines_.push_back(std::move(nonePipeline));


	pipeline.reset(new Pipeline());
	pipeline->Initialize();
	pipelines_.push_back(std::move(pipeline));


	particlePipline.reset(new ParticlePipeline());
	particlePipline->Initialize();
	pipelines_.push_back(std::move(particlePipline));


	grayPipeline.reset(new GrayPipeline());
	grayPipeline->Initialize();
	pipelines_.push_back(std::move(grayPipeline));


	gaussPipeline.reset(new GaussPipeline());
	gaussPipeline->Initialize();
	pipelines_.push_back(std::move(gaussPipeline));


	metaballPipeline.reset(new MetaBallPipeline());
	metaballPipeline->Initialize();
	pipelines_.push_back(std::move(metaballPipeline));


	line3dPipe.reset(new Line3dPipe());
	line3dPipe->Initialize();
	pipelines_.push_back(std::move(line3dPipe));

}

void PipelineManager::SetPipeline(Pipe type) {
	pipelines_[static_cast<int>(type)]->SetPipelineState();
}
