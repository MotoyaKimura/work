#include "Renderer.h"
#include "Pera.h"
#include "Model.h"
#include "Wrapper.h"
#include "Keyboard.h"


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

bool Renderer::RootSignatureInit()
{
	CD3DX12_DESCRIPTOR_RANGE descTblRange[2] = {};
	//カメラ、モデル座標、マテリアル
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0);
	
	//モデルテクスチャ（今は使っていない） 、ライト深度テクスチャ
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

	CD3DX12_ROOT_PARAMETER rootParam = {};
	rootParam.InitAsDescriptorTable(2, &descTblRange[0]);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].Init(0);
	samplerDesc[1].Init(
		1,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(
		1,
		&rootParam,
		2,
		samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

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
		IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	rootSigBlob->Release();

	return true;
}



bool Renderer::PipelineStateInit()
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
		"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};

	gpipelineDesc.pRootSignature = rootsignature.Get();
	gpipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	gpipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	gpipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	gpipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipelineDesc.NumRenderTargets = 2;
	gpipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipelineDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipelineDesc.SampleDesc.Count = 1;
	gpipelineDesc.SampleDesc.Quality = 0;
	
	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(&gpipelineDesc, IID_PPV_ARGS(_pipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}

bool Renderer::PeraRootSignatureInit()
{
	
	CD3DX12_DESCRIPTOR_RANGE descTblRange[2] = {};
	//ペラポリゴン用テクスチャ、視点深度テクスチャ
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0);
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_ROOT_PARAMETER rootParam = {};
	rootParam.InitAsDescriptorTable(2, &descTblRange[0]);
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(1,
		&rootParam,
		1,
		&samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

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
	peraGpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	peraGpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	peraGpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	peraGpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	peraGpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	peraGpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
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
		"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	
	};

	gpipelineDesc.pRootSignature = rootsignature.Get();
	gpipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipelineDesc.SampleDesc.Count = 1;
	gpipelineDesc.SampleDesc.Quality = 0;


	gpipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	gpipelineDesc.InputLayout.NumElements = _countof(inputLayout);
				 
	gpipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	gpipelineDesc.NumRenderTargets = 3;
	gpipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipelineDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipelineDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
	

	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(&gpipelineDesc, IID_PPV_ARGS(_shadowPipelinestate.ReleaseAndGetAddressOf()));
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

	peraGpipeline.pRootSignature = peraRootsignature.Get();
	peraGpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	peraGpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	peraGpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	peraGpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	peraGpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	peraGpipeline.SampleDesc.Count = 1;
	peraGpipeline.SampleDesc.Quality = 0;
	peraGpipeline.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	peraGpipeline.InputLayout.pInputElementDescs = inputLayout;
	peraGpipeline.InputLayout.NumElements = _countof(inputLayout);
	peraGpipeline.NumRenderTargets = 1;
	peraGpipeline.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
	peraGpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	peraGpipeline.BlendState.RenderTarget[0].BlendEnable = false;
	peraGpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	peraGpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(
		&peraGpipeline, 
		IID_PPV_ARGS(_ssaoPipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}


Renderer::Renderer(shared_ptr<Wrapper> dx, shared_ptr<Pera> pera, shared_ptr<Keyboard> keyboard) : _dx(dx), _pera(pera), _keyboard(keyboard)
{
}

bool Renderer::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if(FAILED(!CompileShaderFile(L"PixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!RootSignatureInit()) return false;
	if (!PipelineStateInit()) return false;
	if (FAILED(!CompileShaderFile(L"PeraVertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PeraPixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!PeraRootSignatureInit()) return false;
	if (!PeraPipelineStateInit()) return false;
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "shadeVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "RSMPS", "ps_5_0", psBlob))) return false;
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
	for (auto& _models : _models) {
		_models->Update();
	}
	_dx->Update();
}

void Renderer::ResizeBuffers()
{
	for (auto& _models : _models) {
		_models->MTransBuffInit();
		_models->CreateMTransView();
	}
}

void Renderer::Move()
{
	int modelNum = _models.size();
	BYTE keycode[256];
	GetKeyboardState(keycode);
	if (keycode[VK_NUMPAD1] & 0x80) modelID = 0;
	if (keycode[VK_NUMPAD2] & 0x80) modelID = 1;
	if (keycode[VK_NUMPAD3] & 0x80) modelID = 2;
	if (keycode[VK_NUMPAD4] & 0x80) modelID = 3;
	if (keycode[VK_NUMPAD5] & 0x80) modelID = 4;
	if (keycode[VK_NUMPAD6] & 0x80) modelID = 5;
	if (keycode[VK_NUMPAD7] & 0x80) modelID = 6;
	if (keycode[VK_NUMPAD8] & 0x80) modelID = 7;

	switch (modelID){
	case 0:
		_keyboard->Move(_models[0]->GetPos(), _models[0]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	case 1:
		if (modelNum > modelID)
		_keyboard->Move(_models[1]->GetPos(), _models[1]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	case 2:
		if (modelNum > modelID)
		_keyboard->Move(_models[2]->GetPos(), _models[2]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	case 3:
		if (modelNum > modelID)
		_keyboard->Move(_models[3]->GetPos(), _models[3]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	case 4:
		if (modelNum > modelID)
		_keyboard->Move(_models[4]->GetPos(), _models[4]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	case 5:
		if (modelNum > modelID)
		_keyboard->Move(_models[5]->GetPos(), _models[5]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	case 6:
		if (modelNum > modelID)
		_keyboard->Move(_models[6]->GetPos(), _models[6]->GetRotate(), _dx->GetEyePos(), _dx->GetTargetPos());
		break;
	default:
		break;
	 }
}

void Renderer::BeforeDrawModel()
{
	_dx->GetCommandList()->SetPipelineState(_pipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(rootsignature.Get());
}

void Renderer::BeforeDrawRSM()
{
	_dx->GetCommandList()->SetPipelineState(_shadowPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(rootsignature.Get());
}

void Renderer::DrawModel()
{
	for (auto& _models : _models) {
		_models->Draw(false);
	}
}

void Renderer::DrawRSM()
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