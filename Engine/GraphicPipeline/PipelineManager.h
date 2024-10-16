#pragma once
#include "BasePipeline.h"
#include <memory>



enum class Pipe : int {

	None,
	Normal,
	particle,
	Gray,
	Gauss,
	MetaBall,
};


class PipelineManager {
public:
	PipelineManager() = default;
	~PipelineManager();

public:

	static PipelineManager* GetInstance();

	void Initialize();

	void Finalize();

	void CreatePipeline();



	void SetPipeline(Pipe type);


private:


private:


	std::vector<std::unique_ptr<BasePipeline>> pipelines_;


};
