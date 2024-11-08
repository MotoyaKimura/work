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

void Renderer::DrawModel()
{
	for (auto& _models : _models) {
		_models->Draw();
	}
}

void Renderer::DrawPera()
{
	_pera->Draw();
}


void Renderer::BeforeDraw(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelinestate, Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature)
{
	_dx->GetCommandList()->SetPipelineState(pipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(rootsignature.Get());
}

void Renderer::SetBarrierState(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		before,
		after
	);
	_dx->GetCommandList()->ResourceBarrier(1, &barrier);
}

void Renderer::SetBarrierState(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const
{
	vector<CD3DX12_RESOURCE_BARRIER> barriers;
	for (auto& buffer : buffers) {
		barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(
			buffer.Get(),
			before,
			after
		));
	}
	_dx->GetCommandList()->ResourceBarrier(barriers.size(), barriers.data());
}

void Renderer::SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap)
{
	int numDescs = rtvHeap->GetDesc().NumDescriptors;
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	auto baseHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize =
		_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	uint32_t offset = 0;
	for (int i = 0; i < numDescs; ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle;
		handle.InitOffsetted(baseHandle, offset);
		rtvHandles.emplace_back(handle);
		offset += rtvIncSize;
	}

	if (dsvHeap)
	{
		auto dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		_dx->GetCommandList()->OMSetRenderTargets(
			numDescs,
			rtvHandles.data(),
			true,
			&dsvHandle
		);
		_dx->GetCommandList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);
	}
	else
	{
		_dx->GetCommandList()->OMSetRenderTargets(
			numDescs,
			rtvHandles.data(),
			true,
			nullptr
		);
	}
	for (int i = 0; i < numDescs; ++i)
	{
		_dx->GetCommandList()->ClearRenderTargetView(
			rtvHandles[i], clsClr, 0, nullptr);
	}
}
void Renderer::SetVPAndSR(UINT windowWidth, UINT windowHeight)
{
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, windowWidth, windowHeight);
	CD3DX12_RECT rc(0, 0, windowWidth, windowHeight);
	_dx->GetCommandList()->RSSetViewports(1, &vp);
	_dx->GetCommandList()->RSSetScissorRects(1, &rc);
}

void Renderer::SetSRVDesc(DXGI_FORMAT format)
{
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}

bool Renderer::CreateBuffers()
{
	_buffers.resize(_numBuffers);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM, resWidth, resHeight);

	CD3DX12_CLEAR_VALUE clearValue(resDesc.Format, clsClr);
	for (auto& res : _buffers) {
		auto result = _dx->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf())
		);
		if (FAILED(result)) return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = _buffers.size();

	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_RTVHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	auto handle = _RTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _buffers) {
		_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	return true;
}

bool Renderer::CreateDepthBuffer()
{
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32_TYPELESS, resWidth, resHeight);
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = _dx->GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = _dx->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(_DSVHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	auto handle = _DSVHeap->GetCPUDescriptorHandleForHeapStart();
	_dx->GetDevice()->CreateDepthStencilView(
		_depthBuffer.Get(),
		&dsvDesc,
		handle
	);
	return true;
}


Renderer::~Renderer()
{
}