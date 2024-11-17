#include "FirePipe.h"
#include "DXCom.h"
#include "DXCommand.h"
#include "DXCompil.h"
#include "Logger.h"
#include <cassert>





FirePipe::~FirePipe() {}


void FirePipe::CreateRootSignature(ID3D12Device* device) {

	HRESULT hr;

	D3D12_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	CD3DX12_DESCRIPTOR_RANGE descriptorRangeSRV[4] = {};
	descriptorRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0: g_InputTexture
	descriptorRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1: maskTexture
	descriptorRangeSRV[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); // t1: VoronoiTexture
	descriptorRangeSRV[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // t2: fineNoiseTexture

	D3D12_ROOT_PARAMETER rootParameters[5] = {};
	// SRV t0: g_InputTexture
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRangeSRV[0];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

	// SRV t1: maskTexture
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRangeSRV[1];
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

	// SRV t1: VoronoiTexture
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRangeSRV[2];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	// SRV t2: fineNoiseTexture
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRangeSRV[3];
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 0;

	rootDesc.pParameters = rootParameters;
	rootDesc.NumParameters = _countof(rootParameters);


	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootDesc.pStaticSamplers = staticSamplers;
	rootDesc.NumStaticSamplers = _countof(staticSamplers);


	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

}


void FirePipe::CreatePSO(ID3D12Device* device) {

	HRESULT hr;

	D3D12_INPUT_ELEMENT_DESC element[2] = {};
	element[0].SemanticName = "POSITION";
	element[0].SemanticIndex = 0;
	element[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	element[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	element[1].SemanticName = "TEXCOORD";
	element[1].SemanticIndex = 0;
	element[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	element[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC layout{};
	layout.pInputElementDescs = element;
	layout.NumElements = _countof(element);


	vs = DXCom::GetInstance()->GetDXCompil()->CompileShader(kDirectoryPath_ + L"Fire.VS.hlsl", L"vs_6_0");
	assert(vs != nullptr);
	ps = DXCom::GetInstance()->GetDXCompil()->CompileShader(kDirectoryPath_ + L"Fire.PS.hlsl", L"ps_6_0");
	assert(ps != nullptr);


	D3D12_DEPTH_STENCIL_DESC depth{};
	depth.DepthEnable = FALSE;
	depth.StencilEnable = FALSE;
	depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_BLEND_DESC blend{};
	blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blend.RenderTarget[0].BlendEnable = TRUE;
	blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc{};
	stateDesc.pRootSignature = rootSignature_.Get();
	stateDesc.InputLayout = layout;
	stateDesc.VS = { vs->GetBufferPointer(),vs->GetBufferSize() };
	stateDesc.PS = { ps->GetBufferPointer(),ps->GetBufferSize() };
	stateDesc.DepthStencilState = depth;
	stateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	stateDesc.BlendState = blend;
	stateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	stateDesc.NumRenderTargets = 1;
	stateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	stateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	stateDesc.SampleDesc.Count = 1;
	stateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	hr = device->CreateGraphicsPipelineState(
		&stateDesc, IID_PPV_ARGS(&pso_));
	assert(SUCCEEDED(hr));

}
