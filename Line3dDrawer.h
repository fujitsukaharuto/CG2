#pragma once
#include <wrl/client.h>
#include "MatrixCalculation.h"
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <string>

using namespace Microsoft::WRL;


class Camera;

class Line3dDrawer {
public:
	Line3dDrawer();
	~Line3dDrawer();

public:

	static const UINT kMaxLineCount = 4096;
	static const UINT kVertexCountLine = 2;
	static const UINT kIndexCountLine = 0;

	struct VertexPosColor {
		Vector3 pos;
		Vector4 color;
	};

	struct LineData {
		ComPtr<ID3D12Resource> vertBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vbView{};
		D3D12_INDEX_BUFFER_VIEW ibView{};
		VertexPosColor* vertMap = nullptr;
		uint16_t* indexMap = nullptr;
	};


	static Line3dDrawer* GetInstance();

	std::unique_ptr<LineData> CreateMesh(UINT vertexCount, UINT indexCount);

	void Initialize();

	void Finalize();

	void DrawLine3d(const Vector3& p1, const Vector3& p2, const Vector4& color);

	void Reset();

	void Render();

	void SetCamera(Camera* camera);


private:

	void CreateMeshes();

	void CreateResource();

private:

	std::unique_ptr<LineData> line_;

	uint32_t indexLine_ = 0;

	Camera* camera_ = nullptr;

	struct CBuffer {
		Matrix4x4 viewProject;
	};

	ComPtr<ID3D12Resource> cBufferResource_ = nullptr;
	CBuffer* cBufferData_ = nullptr;

};