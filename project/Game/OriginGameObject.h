#pragma once
#include <memory>
#include "Model/Object3d.h"


class OriginGameObject {
public:
	OriginGameObject();
	virtual ~OriginGameObject() = default;

	virtual void Initialize();
	virtual void Update();
	virtual void Draw(Material* mate = nullptr);

#ifdef _DEBUG
	virtual void Debug();
#endif // _DEBUG

public:

	void CreateModel(const std::string& name);
	void SetModel(const std::string& name);
	Object3d* GetModel() { return model_.get(); }
	Trans& GetTrans() { return model_->transform; }

protected:

	std::unique_ptr<Object3d> model_;

};
