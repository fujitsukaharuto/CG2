#pragma once

#include"OriginGameObject.h"

class SkyDome :public OriginGameObject {
private:



public:

	
	SkyDome();
	~SkyDome();
	/// ===================================================
	///public method
	/// ===================================================

	//* 初期化、更新、
	void Initialize()override;
	void Update()override;
	void Draw(Material* mate = nullptr)override;
};
