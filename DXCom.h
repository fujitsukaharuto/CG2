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
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


#include "MatrixCalculation.h"





class DXCom
{
public:
	

	DXCom* GetInstance();

	void InitDX();

	void CreateDXGI();
	void SettingAdapter();
	void SettingDevice();
	void SettingCommand();
	void SettingSwapChain();
	void CreateRTVSRVHeap();
	void SettingRTV();
	void SettingDepth();
	void CreateFence();
	void SettingDxcUtil();
	void SettingIncludeHandle();
	void SettingRootSignature();
	void SettingGraphicPipeline();


	void SettingVertex();
	void SettingSpriteVertex();
	void SettingResource();
	void SettingSpriteResource();
	void SettingTexture();
	void SettingImgui();


	void FirstFrame();
	void SetBarrier();
	void ClearRTV();
	void Command();
	void LastFrame();

	void SetWVPData(const Matrix4x4& world, const Matrix4x4& wvp);
	void SetSpriteWVPData(const Matrix4x4& world, const Matrix4x4& wvp);


	void SetVertexData(const uint32_t index, const Vector4& position, const Vector2& texcord, const Vector3& normal);


	void ReleaseData();

	void Log(const std::string& message);
	std::wstring ConvertString(const std::string& str);
	std::string ConvertString(const std::wstring& str);

	IDxcBlob* CompileShader(const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	ID3D12DescriptorHeap* CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible
	);

	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	[[nodiscard]]
	ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
		ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

private:

	DXCom() = default;
	~DXCom() = default;

	IDXGIFactory7* dxgiFactory_;
	IDXGIAdapter4* useAdapter_;
	DXGI_ADAPTER_DESC3 adapterDesc_{};
	ID3D12Device* device_;
	ID3D12CommandQueue* commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_{};
	ID3D12CommandAllocator* commandAllocator_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	IDXGISwapChain4* swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	ID3D12DescriptorHeap* rtvDescriptorHeap_ = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	ID3D12DescriptorHeap* srvDescriptorHeap_ = nullptr;
	ID3D12Resource* swapChainResources_[2] = { nullptr };
	ID3D12Resource* depthStencilResource_ = nullptr;
	ID3D12DescriptorHeap* dsvDescriptorHeap_ = nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{};
	ID3D12Fence* fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;
	ID3D12RootSignature* roootSignature_ = nullptr;
	
	IDxcBlob* vertexShaderBlob_ = nullptr;
	IDxcBlob* pixelShaderBlob_ = nullptr;
	ID3D12PipelineState* graphicsPipelineState_ = nullptr;

	ID3D12Resource* vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexDate_ = nullptr;
	ID3D12Resource* vertexResourceSprite_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};
	VertexData* vertexDataSprite_ = nullptr;

	ID3D12Resource* wvpResource_ = nullptr;
	TransformationMatrix* wvpDate_ = nullptr;
	ID3D12Resource* materialResource_ = nullptr;
	Material* materialDate_ = nullptr;
	ID3D12Resource* directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	ID3D12Resource* transformationMatResourceSprite_ = nullptr;
	TransformationMatrix* transformationMatDataSprite_ = nullptr;

	ID3D12Resource* materialResourceSprite_ = nullptr;
	Material* materialDateSprite_ = nullptr;


	DirectX::ScratchImage mipImages_;
	DirectX::ScratchImage mipImages2_;

	ID3D12Resource* textureResource_;
	ID3D12Resource* textureResource2_;

	ID3D12Resource* intermediateResource1;
	ID3D12Resource* intermediateResource2;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;
	bool useMonsterBall = true;

#ifdef _DEBUG
	ID3D12Debug1* debugController_ = nullptr;
#endif // _DEBUG


};
