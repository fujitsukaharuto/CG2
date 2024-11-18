#include "DXCom.h"
#include "Logger.h"
#include "SRVManager.h"
#include "ImGuiManager.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


DXCom* DXCom::GetInstance()
{
	static DXCom instance;
	return &instance;
}

void DXCom::Initialize(MyWin* myWin)
{
	assert(myWin);
	myWin_ = myWin;
	pipeManager_ = PipelineManager::GetInstance();

	CreateDevice();
	CreateCommand();
	CreateSwapChain();
	CreateRenderTargets();
	CreateDepthBuffer();


	CreateCompiler();
	InitializeFPSKeeper();
	SettingRootSignature();
	SettingGraphicPipeline();

	//SettingTexture();
	//SettingImgui();

}

void DXCom::CreateDevice()
{

#ifdef _DEBUG
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_))))
	{
		debugController_->EnableDebugLayer();
		debugController_->SetEnableGPUBasedValidation(true);
	}
#endif // _DEBUG


	//DXGIファクトリーの作成
	dxgiFactory_ = nullptr;
	//HRESULTはWindows系のエラーコードで、関数が成功したかどうかをマクロで判断できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	//初期化の根本的な部分でエラーが出た場合は間違えているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	useAdapter_ = nullptr;
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			Logger::Log((std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr;//ソフトウェアタブの場合見つからなかったことに
	}
	assert(useAdapter_ != nullptr);


	device_ = nullptr;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		if (SUCCEEDED(hr))
		{
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device_ != nullptr);
	Logger::Log("Complete creat D3D12Device!!\n");

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);

		infoQueue->Release();
	}
#endif // _DEBUG

}

void DXCom::CreateCommand()
{

	command_.reset(new DXCommand());
	command_->Initialize(device_.Get());

}

void DXCom::CreateSwapChain()
{
	swapChain_ = nullptr;
	swapChainDesc_.Width = MyWin::kWindowWidth;
	swapChainDesc_.Height = MyWin::kWindowHeight;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(
		command_->GetQueue(), myWin_->GetHwnd(), &swapChainDesc_,
		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));


}

void DXCom::CreateRenderTargets()
{

	rtvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);

	HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr));


	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc_, rtvHandles_[0]);
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc_, rtvHandles_[1]);

	rtvHandles_[2].ptr = rtvHandles_[1].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	offscreenrtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	offscreenrtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	clearColorValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearColorValue.Color[0] = 0.1f;
	clearColorValue.Color[1] = 0.25f;
	clearColorValue.Color[2] = 0.5f;
	clearColorValue.Color[3] = 1.0f;

	offscreenrt_ = CreateOffscreenTextureResource(
		device_.Get(), MyWin::kWindowWidth, MyWin::kWindowHeight, clearColorValue);

	device_->CreateRenderTargetView(offscreenrt_.Get(), &offscreenrtvDesc_, rtvHandles_[2]);


	shockResource_ = CreateBufferResource(device_, sizeof(ShockWaveData));
	shockData_ = nullptr;
	shockResource_->Map(0, nullptr, reinterpret_cast<void**>(&shockData_));
	shockData_->center = { 0.5f,0.5f,0.0f,0.0f };
	shockData_->shockTime = 0.0f;
	shockData_->radius = 1.0f;
	shockData_->intensity = 0.15f;
	shockData_->padding = 0.0f;


	fireResource_ = CreateBufferResource(device_, sizeof(FireElement));
	fireData_ = nullptr;
	fireResource_->Map(0, nullptr, reinterpret_cast<void**>(&fireData_));
	fireData_->animeTime = 0.0f;
	fireData_->resolution = { 1280.0f, 720.0f };
	fireData_->distortionStrength = 0.09f;
	fireData_->highlightStrength = 0.6f;
	fireData_->detailScale = 8.88f;
	fireData_->rangeMin = { 0.0f,0.0f };
	fireData_->rangeMax = { 1.0f,1.0f };
	fireData_->scale = 1.44f;
	fireData_->speed = 4.56f;
	fireData_->noiseSpeed = -0.12f;
	fireData_->blendStrength = 2.0f;

}

void DXCom::CreateDepthBuffer()
{

	depthStencilResource_ = CreateDepthStencilTextureResource(
		device_.Get(), MyWin::kWindowWidth, MyWin::kWindowHeight);

	dsvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	dsvDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc_.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc_, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());


}

void DXCom::CreateCompiler() {

	compiler_.reset(new DXCompil());
	compiler_->Initialize();

}

void DXCom::InitializeFPSKeeper() {
	fpsKeeper_ = FPSKeeper::GetInstance();
}


void DXCom::SettingRootSignature()
{
	
	pipeManager_->CreatePipeline();


}

void DXCom::SettingGraphicPipeline()
{

	/*for (uint32_t index = 0; index < instanceCount_; ++index)
	{
		particles_[index].transform.scale = { 1.0f,1.0f,1.0f };
		particles_[index].transform.rotate = { 0.0f,0.0f,0.0f };
		particles_[index].transform.translate = { index * 0.1f,index * 0.1f,index * 0.1f };

		particles_[index].velocity = { 0.0f,1.0f,0.0f };
	}*/


	vertexGrayResource_ = CreateBufferResource(device_.Get(), sizeof(GrayscaleVertex) * 6);

	vertexGrayBufferView_.BufferLocation = vertexGrayResource_->GetGPUVirtualAddress();
	vertexGrayBufferView_.SizeInBytes = sizeof(GrayscaleVertex) * 6;
	vertexGrayBufferView_.StrideInBytes = sizeof(GrayscaleVertex);

	grayVertexDate_ = nullptr;
	vertexGrayResource_->Map(0, nullptr, reinterpret_cast<void**>(&grayVertexDate_));

	grayVertexDate_[0] = { Vector4(-1.0f,-1.0f,0.0f,1.0f),Vector2(0.0f,1.0f) };
	grayVertexDate_[1] = { Vector4(-1.0f,1.0f,0.0f,1.0f),Vector2(0.0f,0.0f) };
	grayVertexDate_[2] = { Vector4(1.0f,1.0f,0.0f,1.0f),Vector2(1.0f,0.0f) };
	grayVertexDate_[3] = { Vector4(1.0f,-1.0f,0.0f,1.0f),Vector2(1.0f,1.0f) };


	indexGrayResourece_ = CreateBufferResource(device_.Get(), sizeof(uint32_t) * 6);

	indexGrayBufferView_.BufferLocation = indexGrayResourece_->GetGPUVirtualAddress();
	indexGrayBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexGrayBufferView_.Format = DXGI_FORMAT_R32_UINT;

	indexGrayResourece_->Map(0, nullptr, reinterpret_cast<void**>(&indexGrayData_));

	indexGrayData_[0] = 0;
	indexGrayData_[1] = 1;
	indexGrayData_[2] = 2;
	indexGrayData_[3] = 0;
	indexGrayData_[4] = 2;
	indexGrayData_[5] = 3;

	isGrayscale_ = false;
	isNonePost_ = true;
	isMetaBall_ = false;
	isGaussian_ = false;
	isShockWave_ = false;
	isFire_ = false;
}

void DXCom::CreateBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources_[swapChain_->GetCurrentBackBufferIndex()].Get();
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	command_->GetList()->ResourceBarrier(1, &barrier);
}




void DXCom::SettingTexture()
{
	//const uint32_t descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	SRVManager* srvManager = SRVManager::GetInstance();
	offscreenSRVIndex_ = srvManager->Allocate();

	srvManager->CreateTextureSRV(offscreenSRVIndex_, offscreenrt_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);

	offTextureHandleCPU_ = srvManager->GetCPUDescriptorHandle(offscreenSRVIndex_);
	offTextureHandle_ = srvManager->GetGPUDescriptorHandle(offscreenSRVIndex_);

	baseTex_ = TextureManager::GetInstance()->LoadTexture("Gradient02.jpg");
	voronoTex_ = TextureManager::GetInstance()->LoadTexture("T_Noise04.jpg");
	noiseTex_= TextureManager::GetInstance()->LoadTexture("T_Noise02-300x300.jpg");



	CommandExecution();
}


void DXCom::PreDraw()
{
	D3D12_RESOURCE_BARRIER offbarrier{};
	offbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	offbarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	offbarrier.Transition.pResource = offscreenrt_.Get();
	offbarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	offbarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	command_->GetList()->ResourceBarrier(1, &offbarrier);

	SRVManager::GetInstance()->SetDescriptorHeap();

	SetRenderTargets();
	ClearRenderTarget();
	ClearDepthBuffer();
}

void DXCom::Command()
{


	command_->SetViewAndscissor();
	pipeManager_->SetPipeline(Pipe::Normal);


}

void DXCom::PostEffect()
{

	ID3D12GraphicsCommandList* commandList = command_->GetList();

	D3D12_RESOURCE_BARRIER offbarrier{};
	offbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	offbarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	offbarrier.Transition.pResource = offscreenrt_.Get();
	offbarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	offbarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &offbarrier);


	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	CreateBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandle);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);




	if (isGrayscale_)
	{
		command_->SetViewAndscissor();
		pipeManager_->SetPipeline(Pipe::Gray);

		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetIndexBuffer(&indexGrayBufferView_);
		commandList->IASetVertexBuffers(0, 1, &vertexGrayBufferView_);
		commandList->SetGraphicsRootDescriptorTable(0, offTextureHandle_);
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}


	if (isGaussian_)
	{
		command_->SetViewAndscissor();
		pipeManager_->SetPipeline(Pipe::Gauss);

		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetIndexBuffer(&indexGrayBufferView_);
		commandList->IASetVertexBuffers(0, 1, &vertexGrayBufferView_);
		commandList->SetGraphicsRootDescriptorTable(0, offTextureHandle_);
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	if (isNonePost_||isMetaBall_)
	{
		command_->SetViewAndscissor();
		pipeManager_->SetPipeline(Pipe::None);

		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetIndexBuffer(&indexGrayBufferView_);
		commandList->IASetVertexBuffers(0, 1, &vertexGrayBufferView_);
		commandList->SetGraphicsRootDescriptorTable(0, offTextureHandle_);
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	if (isShockWave_) {
		command_->SetViewAndscissor();
		pipeManager_->SetPipeline(Pipe::ShockWave);

		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetIndexBuffer(&indexGrayBufferView_);
		commandList->IASetVertexBuffers(0, 1, &vertexGrayBufferView_);
		commandList->SetGraphicsRootDescriptorTable(0, offTextureHandle_);
		commandList->SetGraphicsRootConstantBufferView(1, shockResource_->GetGPUVirtualAddress());
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	if (isFire_) {
		command_->SetViewAndscissor();
		pipeManager_->SetPipeline(Pipe::Fire);

		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetIndexBuffer(&indexGrayBufferView_);
		commandList->IASetVertexBuffers(0, 1, &vertexGrayBufferView_);
		commandList->SetGraphicsRootDescriptorTable(0, offTextureHandle_);
		commandList->SetGraphicsRootDescriptorTable(1, baseTex_->gpuHandle);
		commandList->SetGraphicsRootDescriptorTable(2, voronoTex_->gpuHandle);
		commandList->SetGraphicsRootDescriptorTable(3, noiseTex_->gpuHandle);
		commandList->SetGraphicsRootConstantBufferView(4, fireResource_->GetGPUVirtualAddress());
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

}

void DXCom::PostDraw() {


	CreateBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// 命令のクローズ
	command_->Close();
	// コマンドリストの実行
	command_->Execution();
	fpsKeeper_->FixFPS();
	swapChain_->Present(1, 0);
	command_->Reset();
}


void DXCom::PreSpriteDraw() {
	command_->SetViewAndscissor();
	pipeManager_->SetPipeline(Pipe::Normal);
	command_->GetList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DXCom::CommandExecution() {
	command_->Close();

	command_->Execution();

	command_->Reset();
}

void DXCom::SetRenderTargets() {

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	command_->GetList()->OMSetRenderTargets(1, &rtvHandles_[2], false, &dsvHandle);
}

void DXCom::ClearRenderTarget() {

	command_->GetList()->ClearRenderTargetView(rtvHandles_[2], clearColorValue.Color, 0, nullptr);
}

void DXCom::ClearDepthBuffer() {

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	command_->GetList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DXCom::UpDate()
{
	/*Tick();*/

#ifdef _DEBUG
	ImGui::Begin("debug");


	bool preIsGrayscale_ = isGrayscale_;
	bool preIsNonePost_ = isNonePost_;
	bool preIsMetaBall_ = isMetaBall_;
	bool preIsGaussian_ = isGaussian_;
	bool preIsShock_ = isShockWave_;
	bool preIsFire_ = isFire_;

	if (ImGui::TreeNode("OffScreen ShaderPath"))
	{
		ImGui::Checkbox("Gray", &isGrayscale_);
		ImGui::Checkbox("None", &isNonePost_);
		ImGui::Checkbox("Meta", &isMetaBall_);
		ImGui::Checkbox("Blur", &isGaussian_);
		ImGui::Checkbox("shock", &isShockWave_);
		ImGui::Checkbox("fire", &isFire_);
		ImGui::TreePop();
	}
	if (isGrayscale_ && !(preIsGrayscale_))
	{
		isNonePost_ = false;
		isMetaBall_ = false;
		isGaussian_ = false;
		isShockWave_ = false;
		isFire_ = false;
	}
	if (isNonePost_ && !(preIsNonePost_))
	{
		isGrayscale_ = false;
		isMetaBall_ = false;
		isGaussian_ = false;
		isShockWave_= false;
		isFire_ = false;
	}
	if (isMetaBall_ && !(preIsMetaBall_))
	{
		isGrayscale_ = false;
		isNonePost_ = false;
		isGaussian_ = false;
		isShockWave_= false;
		isFire_ = false;
	}
	if (isGaussian_ && !(preIsGaussian_))
	{
		isGrayscale_ = false;
		isNonePost_ = false;
		isMetaBall_ = false;
		isShockWave_= false;
		isFire_ = false;
	}
	if (isShockWave_ && !(preIsShock_)) {
		isGrayscale_ = false;
		isNonePost_ = false;
		isMetaBall_ = false;
		isGaussian_ = false;
		isFire_ = false;
	}
	if (isFire_ && !(preIsFire_)) {
		isGrayscale_ = false;
		isNonePost_ = false;
		isMetaBall_ = false;
		isGaussian_ = false;
		isShockWave_ = false;
	}

	if (ImGui::Button("shock")) {
		shockData_->shockTime = 0.0f;
	}

	if (ImGui::TreeNode("FireData")) {
		ImGui::DragFloat("animeTime", &fireData_->animeTime, 0.1f, 0.0f, 60.0f);
		ImGui::DragFloat2("resolution", &fireData_->resolution.x);
		ImGui::DragFloat("distortionStrength", &fireData_->distortionStrength, 0.01f);
		ImGui::DragFloat("highlightStrength", &fireData_->highlightStrength, 0.01f);
		ImGui::DragFloat("detailScale", &fireData_->detailScale, 0.01f);
		ImGui::DragFloat2("rangeMin", &fireData_->rangeMin.x, 0.01f);
		ImGui::DragFloat2("rangeMax", &fireData_->rangeMax.x, 0.01f);
		ImGui::DragFloat("scale", &fireData_->scale, 0.01f);
		ImGui::DragFloat("speed", &fireData_->speed, 0.01f);
		ImGui::DragFloat("noiseSpeed", &fireData_->noiseSpeed, 0.01f);
		ImGui::DragFloat("blend", &fireData_->blendStrength, 0.01f);
		ImGui::TreePop();
	}


	ImGui::End();

	shockData_->shockTime += 0.025f;
	fireData_->animeTime += 0.025f;

#endif // _DEBUG



	/*transform.rotate.y += 0.05f;*/
	/*Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);*/
	/*Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));*/

	/*wvpDate_->World = worldMatrix;
	wvpDate_->WVP = worldViewProjectionMatrix;*/

}

void DXCom::ReleaseData()
{
	/*for (int i = 0; i < particleIndex; i++)
	{
		vertexParticleResource_[i]->Release();
		wvpParticleResource_[i]->Release();
		materialParticleResource_[i]->Release();
	}*/

	/*particles.clear();*/

#ifdef _DEBUG

#endif // _DEBUG

}



Microsoft::WRL::ComPtr<ID3D12Resource> DXCom::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC bufferResourceDesc{};
	bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferResourceDesc.Width = sizeInBytes;
	bufferResourceDesc.Height = 1;
	bufferResourceDesc.DepthOrArraySize = 1;
	bufferResourceDesc.MipLevels = 1;
	bufferResourceDesc.SampleDesc.Count = 1;

	bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&bufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DXCom::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXCom::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties,
		D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXCom::CreateOffscreenTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height, D3D12_CLEAR_VALUE color)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	//resourceDesc.Flags = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties,
		D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, &color,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXCom::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXCom::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}
