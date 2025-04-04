#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include "MatrixCalculation.h"

struct SpotLightData {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float pading[1];
};

class SpotLight {
public:
	SpotLight() = default;
	~SpotLight() = default;

public:

	void Initialize();

	void SetLightCommand(ID3D12GraphicsCommandList* commandList);

	SpotLightData* spotLightData_ = nullptr;

	void Debug();

private:

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_ = nullptr;

};
