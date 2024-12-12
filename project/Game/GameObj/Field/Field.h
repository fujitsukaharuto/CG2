#pragma once

#include"OriginGameObject.h"

class Field:public OriginGameObject {
private:

	

public:

	static Vector3 baseScale_;

	Field();
	~Field();
	/// ===================================================
	///public method
	/// ===================================================

	//* 初期化、更新、
	void Initialize()override;
	void Update()override;
	void Draw(Material* mate = nullptr)override;
};
