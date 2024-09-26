#include "Renderer.h"
#include "Pera.h"
#include "Model.h"
#include "Wrapper.h"


#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;
using namespace Microsoft::WRL;

bool Renderer::CheckResult(HRESULT result)
{
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else
		{
			std::string errstr;
			errstr.resize(errBlob->GetBufferSize());
			std::copy_n((char*)errBlob->GetBufferPointer(),
				errBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";
			::OutputDebugStringA(errstr.c_str());

		}
		return false;
	}
	return true;
}


bool Renderer::CompileShaderFile(std::wstring hlslFile, std::string EntryPoint, std::string model, Microsoft::WRL::ComPtr<ID3DBlob>& _xsBlob)
{
	auto result = D3DCompileFromFile(
		hlslFile.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint.c_str(),
		model.c_str(),
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		_xsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());

	return CheckResult(result);
}

bool Renderer::TeapotRootSignatureInit()
{
	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};
	//シーン変換 //ティーポットテクスチャ
	descTblRange[0].NumDescriptors = 2;
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[0].BaseShaderRegister = 0;
	descTblRange[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	//モデル座標変換 //ライト深度テクスチャ
	descTblRange[1].NumDescriptors = 2;
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[1].BaseShaderRegister = 0;
	descTblRange[1].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER rootParam = {};
	//シーン変換,ティーポットテクスチャ,モデル座標変換
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam.DescriptorTable.pDescriptorRanges = &descTblRange[0];
	rootParam.DescriptorTable.NumDescriptorRanges = 2;

	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[0].MinLOD = 0.0f;
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	
	samplerDesc[1] = samplerDesc[0];
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplerDesc[1].Filter =
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc[1].MaxAnisotropy = 1;
	samplerDesc[1].ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	ComPtr< ID3DBlob> rootSigBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&rootSigBlob,
		errBlob.ReleaseAndGetAddressOf());

	if (!CheckResult(result)) return false;

	result = _dx->GetDevice()->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(teapotRootsignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	rootSigBlob->Release();

	return true;
}

bool Renderer::TeapotPipelineStateInit()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
				{
			"INDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}

	};

	teapotGpipeline.pRootSignature = teapotRootsignature.Get();
	teapotGpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	teapotGpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	teapotGpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	teapotGpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
	teapotGpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	teapotGpipeline.RasterizerState.MultisampleEnable = false;
	teapotGpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	teapotGpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	teapotGpipeline.RasterizerState.DepthClipEnable = true;
	teapotGpipeline.DepthStencilState.DepthEnable = true;
	teapotGpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	teapotGpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	teapotGpipeline.BlendState.AlphaToCoverageEnable = false;
	teapotGpipeline.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	teapotGpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	teapotGpipeline.InputLayout.pInputElementDescs = inputLayout;
	teapotGpipeline.InputLayout.NumElements = _countof(inputLayout);
	teapotGpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	teapotGpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	teapotGpipeline.NumRenderTargets = 2;
	teapotGpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	teapotGpipeline.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	teapotGpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	teapotGpipeline.SampleDesc.Count = 1;
	teapotGpipeline.SampleDesc.Quality = 0;

	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(&teapotGpipeline, IID_PPV_ARGS(_teapotPipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}

bool Renderer::PeraRootSignatureInit()
{
	
	CD3DX12_DESCRIPTOR_RANGE descTblRange[2] = {};
	//ペラポリゴン用テクスチャ、視点深度テクスチャ
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_ROOT_PARAMETER rootParam = {};
	rootParam.InitAsDescriptorTable(2, &descTblRange[0]);
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rootSigBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());
	if (!CheckResult(result)) return false;

	result = _dx->GetDevice()->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(peraRootsignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	rootSigBlob->Release();

	return true;
}

bool Renderer::PeraPipelineStateInit()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};
	

	peraGpipeline.pRootSignature = peraRootsignature.Get();
	peraGpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	peraGpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	peraGpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	peraGpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
	peraGpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	peraGpipeline.RasterizerState.MultisampleEnable = false;
	peraGpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	peraGpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	peraGpipeline.RasterizerState.DepthClipEnable = true;
	peraGpipeline.BlendState.AlphaToCoverageEnable = false;
	peraGpipeline.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	peraGpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	peraGpipeline.InputLayout.pInputElementDescs = inputLayout;
	peraGpipeline.InputLayout.NumElements = _countof(inputLayout);
	peraGpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	peraGpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	peraGpipeline.NumRenderTargets = 1;
	peraGpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	peraGpipeline.SampleDesc.Count = 1;
	peraGpipeline.SampleDesc.Quality = 0;
	peraGpipeline.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(&peraGpipeline, IID_PPV_ARGS(_peraPipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}

bool Renderer::ShadowPipelineStateInit()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
				{
			"INDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};

	teapotGpipeline.InputLayout.pInputElementDescs = inputLayout;
	teapotGpipeline.InputLayout.NumElements = _countof(inputLayout);

	teapotGpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	teapotGpipeline.PS.BytecodeLength = 0;
	teapotGpipeline.PS.pShaderBytecode = nullptr;
	teapotGpipeline.NumRenderTargets = 0;
	teapotGpipeline.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	teapotGpipeline.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;

	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(&teapotGpipeline, IID_PPV_ARGS(_shadowPipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}

bool Renderer::SSAOPipelineStateInit()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};

	peraGpipeline.InputLayout.pInputElementDescs = inputLayout;
	peraGpipeline.InputLayout.NumElements = _countof(inputLayout);
	peraGpipeline.NumRenderTargets = 1;
	peraGpipeline.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
	peraGpipeline.BlendState.RenderTarget[0].BlendEnable = false;
	peraGpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	peraGpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(
		&peraGpipeline, 
		IID_PPV_ARGS(_ssaoPipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}


Renderer::Renderer(shared_ptr<Wrapper> dx, shared_ptr<Pera> pera) : _dx(dx), _pera(pera)
{
}

bool Renderer::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if(FAILED(!CompileShaderFile(L"PixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!TeapotRootSignatureInit()) return false;
	if (!TeapotPipelineStateInit()) return false;
	if (FAILED(!CompileShaderFile(L"PeraVertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PeraPixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!PeraRootSignatureInit()) return false;
	if (!PeraPipelineStateInit()) return false;
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "shadowVS", "vs_5_0", vsBlob))) return false;
	if (!ShadowPipelineStateInit()) return false;
	if (FAILED(!CompileShaderFile(L"SSAOVertexShader.hlsl", "ssaoVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"SSAOPixelShader.hlsl", "ssaoPS", "ps_5_0", psBlob))) return false;
	if (!SSAOPipelineStateInit()) return false;
	return true;
}

void Renderer::AddModel(std::shared_ptr<Model> model)
{
	_models.emplace_back(model);
}

void Renderer::Update()
{
}



void Renderer::BeforeDrawTeapot()
{
	_dx->GetCommandList()->SetPipelineState(_teapotPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(teapotRootsignature.Get());
}

void Renderer::BeforeDrawShadow()
{
	_dx->GetCommandList()->SetPipelineState(_shadowPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(teapotRootsignature.Get());
}

void Renderer::DrawTeapot()
{
	for (auto& _models : _models) {
		_models->Draw(false);
	}
}

void Renderer::DrawShadow()
{
	for (auto& _models : _models) {
		_models->Draw(true);
	}
}

void Renderer::BeforeDrawSSAO()
{
	_dx->GetCommandList()->SetPipelineState(_ssaoPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(peraRootsignature.Get());
}

void Renderer::DrawSSAO()
{
	_pera->Draw();
}


void Renderer::BeforeDrawPera()
{
	_dx->GetCommandList()->SetPipelineState(_peraPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(peraRootsignature.Get());
}

void Renderer::DrawPera()
{
	_pera->Draw();
}

Renderer::~Renderer()
{
}