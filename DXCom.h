#pragma once
#include <Windows.h>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <dxgidebug.h>
#include <vector>
#include <cassert>
#include <chrono>
#include <random>
#include <wrl.h>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


#include "MatrixCalculation.h"
#include "MyWindow.h"


const int particleIndex = 650;

struct D3DResourceLeakChecker
{
	~D3DResourceLeakChecker()
	{
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
		{
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

class DXCom
{
public:
	

	static DXCom* GetInstance();

	void InitDX(MyWin* myWin);

	void SettingTexture();

	void FirstFrame();
	void SetBarrier();
	void ClearRTV();
	void Command();
	void LastFrame();

	void SetWVPData(const Matrix4x4& world, const Matrix4x4& wvp);
	//void SetSpriteWVPData(const Matrix4x4& world, const Matrix4x4& wvp);


	ID3D12Device* GetDevice() const { return device_.Get(); }

	size_t GetBackBufferCount() const { return swapChainDesc_.BufferCount; }

	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	//仕方なくcommon内でupdate用。後からこんなのなくさなきゃ
	void UpDate();


	void ReleaseData();

	void Log(const std::string& message);
	std::wstring ConvertString(const std::string& str);
	std::string ConvertString(const std::wstring& str);

	IDxcBlob* CompileShader(const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible
	);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);

	void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages,
		Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/*void Tick();*/

private:

	void CreateDXGI();

	void InitializeCommand();

	void CreateSwapChain();

	void CreateRenderTargets();

	void CreateDepthBuffer();

	void CreateFence();

	void SettingDxcUtil();
	void SettingIncludeHandle();
	void SettingRootSignature();
	void SettingGraphicPipeline();
	void SettingVertex();
	void SettingSpriteVertex();
	void SettingResource();
	void SettingSpriteResource();

private:

	DXCom() = default;
	~DXCom() = default;

	D3DResourceLeakChecker leakCheck_;

	MyWin* myWin_;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	DXGI_ADAPTER_DESC3 adapterDesc_{};
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_{};
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{};
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	
	IDxcBlob* vertexShaderBlob_ = nullptr;
	IDxcBlob* pixelShaderBlob_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateNormal_ = nullptr;


	//三角形１
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexDate_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	TransformationMatrix* wvpDate_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialDate_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	bool useMonsterBall = true;
	bool isTriangleDraw_ = true;

	//三角形2
	/*Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource2_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView2_{};
	VertexData* vertexDate2_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource2_ = nullptr;
	TransformationMatrix* wvpDate2_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource2_ = nullptr;
	Material* materialDate2_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource2_ = nullptr;
	DirectionalLight* directionalLightData2_ = nullptr;*/
	/*bool useMonsterBall2 = true;*/

	//パーティクル用三角
	/*Microsoft::WRL::ComPtr<ID3D12Resource> vertexParticleResource_[particleIndex]{};
	D3D12_VERTEX_BUFFER_VIEW vertexParticleBufferView_[particleIndex]{};
	VertexData* vertexParticleDate_[particleIndex]{};
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpParticleResource_[particleIndex] = { nullptr };
	
	TransformationMatrix* wvpParticleDate_[particleIndex] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> materialParticleResource_[particleIndex] = { nullptr };
	Material* materialParticleDate_[particleIndex] = { nullptr };*/


	//modelData
	ModelData modelData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexModelResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexModelBufferView_{};
	VertexData* vertexDataModel_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceModel_ = nullptr;
	TransformationMatrix* wvpDateModel_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceModel_ = nullptr;
	Material* materialDateModel_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResourceModel_ = nullptr;
	DirectionalLight* directionalLightDataModel_ = nullptr;


	/*Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};
	VertexData* vertexDataSprite_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResoureceSprite = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	uint32_t* indexDataSprite = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatResourceSprite_ = nullptr;
	TransformationMatrix* transformationMatDataSprite_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite_ = nullptr;
	Material* materialDateSprite_ = nullptr;*/


	Trans transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Trans transform2{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Trans cameraTrans{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	Trans transSprite = { {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Trans uvTransSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	Trans transformModel{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


	Trans transformParticle[particleIndex]{};
	Vector3 particleVel[particleIndex]{};
	Vector3 particleRot[particleIndex]{};
	bool isParticleLive[particleIndex]{ false };
	Vector4 particleColor = { 1.0f,0.0f,0.0f,1.0f };
	float colorRandomAdd = 0;
	std::random_device repopSeed;

	DirectX::ScratchImage mipImages_;
	DirectX::ScratchImage mipImages2_;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2_;

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource1;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;

	/*std::chrono::high_resolution_clock::time_point lastTime;
	float deltaTime;
	float elapsedTime;
	int frameCount;
	int frameRate;*/

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
#endif // _DEBUG


};
