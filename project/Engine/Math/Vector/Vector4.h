#pragma once
#include <cmath>


class Vector4 {
public:

	float x;
	float y;
	float z;
	float w;

	/*----- コンストラクタ ----------------------------------------------------------*/
	Vector4() : x(0), y(0), z(0), w(0) {}
	Vector4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {}
	Vector4(const Vector4& v) :x(v.x), y(v.y), z(v.z), w(v.w) {}


	Vector4& operator = (const Vector4& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

};