#include "TextureManager.h"
#include "DXCom.h"
#include "ImGuiManager.h"

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


namespace {

	DirectX::ScratchImage LoadTextureFile(const std::string& filePath) {
		DirectX::ScratchImage image{};
		std::wstring filePathw = DXCom::GetInstance()->ConvertString(filePath);
		HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));

		DirectX::ScratchImage mipImages{};
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
			DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));

		return mipImages;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata) {
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = UINT(metadata.width);
		resourceDesc.Height = UINT(metadata.height);
		resourceDesc.MipLevels = UINT16(metadata.mipLevels);
		resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
		resourceDesc.Format = metadata.format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;


		Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_PPV_ARGS(&resource));
		assert(SUCCEEDED(hr));

		return resource;
	}


	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(),
			subresources);
		uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = DXCom::GetInstance()->CreateBufferResource(device, intermediateSize);
		UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = texture.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
		commandList->ResourceBarrier(1, &barrier);

		return intermediateResource;
	}
}



TextureManager::TextureManager() {}

TextureManager::~TextureManager() {}

TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}


void TextureManager::Finalize() {

}

Texture* TextureManager::LoadTexture(const std::string& filename) {

	// 既にロードされているかチェック
	if (m_textureCache.find(filename) != m_textureCache.end()) {
		return m_textureCache[filename].get();
	}

	DirectX::ScratchImage mipImages = LoadTextureFile(directoryPath_ + filename);
	std::unique_ptr<Texture> texture = std::make_unique<Texture>();
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	texture->textureResource = CreateTextureResource(DXCom::GetInstance()->GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;

	intermediateResource.Reset();


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	const uint32_t descriptorSizeSRV = DXCom::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	intermediateResource = UploadTextureData(texture->textureResource, mipImages, DXCom::GetInstance()->GetDevice(), DXCom::GetInstance()->GetCommandList());
	texture->cpuHandle = DXCom::GetInstance()->GetCPUDescriptorHandle(ImGuiManager::GetInstance()->GetsrvHeap(), descriptorSizeSRV, DXCom::GetInstance()->GetDescriptorIndex());
	texture->gpuHandle = DXCom::GetInstance()->GetGPUDescriptorHandle(ImGuiManager::GetInstance()->GetsrvHeap(), descriptorSizeSRV, DXCom::GetInstance()->GetDescriptorIndex());
	DXCom::GetInstance()->IncreaseDescriptorIndex();

	DXCom::GetInstance()->CommandExecution();

	DXCom::GetInstance()->GetDevice()->CreateShaderResourceView(texture->textureResource.Get(), &srvDesc, texture->cpuHandle);

	m_textureCache[filename] = std::move(texture);

	return m_textureCache[filename].get();
}

Texture* TextureManager::GetTexture(const std::string& filename) const {
	auto it = m_textureCache.find(filename);
	if (it != m_textureCache.end()) {
		return it->second.get();
	}
	return nullptr;
}

void TextureManager::ReleaseTexture(const std::string& filename) {
	m_textureCache.erase(filename);
}