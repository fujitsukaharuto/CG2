#pragma once
#include <Windows.h>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <cassert>
#include <wrl.h>


#include "MatrixCalculation.h"
#include "MyWindow.h"
#include "DXCommand.h"
#include "DXCompil.h"

#include "FPSKeeper.h"
#include "PipelineManager.h"
#include "TextureManager.h"
#include "Camera.h"



const int particleIndex = 400;
const UINT instanceCount_ = 10;

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



enum BlendMode
{
	// ブレンド無し
	kBlendModeNone,
	// 通常αブレンド
	kBlendModeNormal,
	// 加算
	kBlendModeAdd,
	// 現在
	kBlendModeSubtract,
	// 乗算
	kBlendModeMultiply,
	// スクリーン
	kBlendModeScreen,
};




class DebugCamera;//今だけ、後から消す


class DXCom
{
public:

	static DXCom* GetInstance();

	void Initialize(MyWin* myWin);

	void SettingTexture();


	void PreDraw();
	void Command();
	void PostEffect();
	void PostDraw();

	void PreSpriteDraw();

	void CommandExecution();

	void SetRenderTargets();
	void ClearRenderTarget();
	void ClearDepthBuffer();


	ID3D12Device* GetDevice() const { return device_.Get(); }

	size_t GetBackBufferCount() const { return swapChainDesc_.BufferCount; }

	ID3D12GraphicsCommandList* GetCommandList() const { return command_->GetList(); }

	void UpDate();

	void ReleaseData();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateOffscreenTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height, D3D12_CLEAR_VALUE color);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	DXCommand* GetDXCommand() const { return command_.get(); }

	DXCompil* GetDXCompil() const { return compiler_.get(); }

	PipelineManager* GetPipelineManager()const { return pipeManager_; }

	/*void Tick();*/

private:

	void CreateDevice();

	void CreateCommand();

	void CreateSwapChain();

	void CreateRenderTargets();

	void CreateDepthBuffer();

	void CreateCompiler();

	void InitializeFPSKeeper();

	void SettingRootSignature();
	void SettingGraphicPipeline();

	/// <summary>
	/// swapchainのバリア
	/// </summary>
	/// <param name="before"></param>
	/// <param name="after"></param>
	void CreateBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);


private:

	DXCom() = default;
	~DXCom() = default;

	D3DResourceLeakChecker leakCheck_;

	MyWin* myWin_;


#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
#endif // _DEBUG


	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;


	std::unique_ptr<DXCommand> command_ = nullptr;


	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[3];

	Microsoft::WRL::ComPtr<ID3D12Resource> offscreenrt_ = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC offscreenrtvDesc_{};
	D3D12_CLEAR_VALUE clearColorValue{};
	uint32_t offscreenSRVIndex_;
	D3D12_GPU_DESCRIPTOR_HANDLE offTextureHandle_;
	D3D12_CPU_DESCRIPTOR_HANDLE offTextureHandleCPU_;


	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2] = { nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{};


	std::unique_ptr<DXCompil> compiler_ = nullptr;


	FPSKeeper* fpsKeeper_ = nullptr;

	PipelineManager* pipeManager_;



	Microsoft::WRL::ComPtr<ID3D12Resource> shockResource_ = nullptr;
	ShockWaveData* shockData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> fireResource_ = nullptr;
	FireElement* fireData_;

	Texture* baseTex_;
	Texture* voronoTex_;
	Texture* noiseTex_;

	bool isGrayscale_ = true;
	bool isNonePost_ = true;
	bool isMetaBall_ = true;
	bool isGaussian_ = true;
	bool isShockWave_ = true;
	bool isFire_ = true;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexGrayResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexGrayBufferView_{};
	GrayscaleVertex* grayVertexDate_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexGrayResourece_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexGrayBufferView_{};
	uint32_t* indexGrayData_ = nullptr;


	bool isPlaneAndSprite_ = true;
	bool isPlaneParticle_ = false;
	bool isSphere_ = false;
	bool isFenceModel_ = false;
	bool isSuzanneModel_ = false;
	bool isMMesh_ = false;


};
