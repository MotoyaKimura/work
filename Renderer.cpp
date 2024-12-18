#include "Renderer.h"
#include "Camera.h"
#include "Pera.h"
#include "Model.h"
#include "Wrapper.h"
#include "Keyboard.h"

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;
using namespace Microsoft::WRL;

//レンダラーの共通クラス
Renderer::Renderer(
	shared_ptr<Wrapper> dx,
	shared_ptr<Pera> pera, 
	shared_ptr<Keyboard> keyboard,
	std::vector<std::shared_ptr<Model>> models, 
	std::shared_ptr<Camera> camera
) :
	_dx(dx),
_pera(pera),
_keyboard(keyboard),
_models(models),
_camera(camera)
{
}

Renderer::~Renderer()
{
}


//シェーダーのコンパイル
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

//シェーダーのコンパイルが成功したかどうかを返す
bool Renderer::CheckResult(HRESULT result) const
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

//ルートシグネチャの初期化
bool Renderer::RootSignatureInit()
{

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(
		rootParams.size(),
		rootParams.data(),
		samplers.size(),
		samplers.data(),
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

//ペラポリゴン用のルートシグネチャのパラメータ設定
void Renderer::SetRootSigParamForPera(size_t cbvDescs, size_t srvDescs)
{
	CD3DX12_DESCRIPTOR_RANGE descTblRange;
	//ペラポリゴン用テクスチャ、視点深度テクスチャ
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, cbvDescs, 0);
	ranges.emplace_back(descTblRange);
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvDescs, 0);
	ranges.emplace_back(descTblRange);
	CD3DX12_ROOT_PARAMETER rootParam;
	rootParam.InitAsDescriptorTable(ranges.size(), ranges.data());
	rootParams.emplace_back(rootParam);
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);
	samplers.emplace_back(samplerDesc);

	samplerDesc.Init(
		1,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	samplers.emplace_back(samplerDesc);
	samplerDesc.Init(
		2,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	samplers.emplace_back(samplerDesc);

}

//モデル用のルートシグネチャのパラメータ設定
void Renderer::SetRootSigParamForModel(size_t cbvDescs, size_t srvDescs)
{
	CD3DX12_DESCRIPTOR_RANGE descTblRange;
	//カメラ、(ワールド, ボーン）, invボーン
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0);
	ranges.emplace_back(descTblRange);
	//ライト深度
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges.emplace_back(descTblRange);
	//マテリアル
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);
	ranges.emplace_back(descTblRange);
	//マテリアルテクスチャ
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1);
	ranges.emplace_back(descTblRange);

	CD3DX12_ROOT_PARAMETER rootParam;
	rootParam.InitAsDescriptorTable(2, ranges.data());
	rootParams.emplace_back(rootParam);
	rootParam.InitAsDescriptorTable(2, &ranges[2]);
	rootParams.emplace_back(rootParam);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);
	samplers.emplace_back(samplerDesc);

	samplerDesc.Init(
		1,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	samplers.emplace_back(samplerDesc);

	samplerDesc.Init(
		2,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplers.emplace_back(samplerDesc);
}


//パイプラインステートの初期化
bool Renderer::PipelineStateInit()
{
	gpipelineDesc.pRootSignature = rootsignature.Get();
	gpipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	gpipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpipelineDesc.BlendState.AlphaToCoverageEnable = false;
	gpipelineDesc.BlendState.IndependentBlendEnable = false;
	/*const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};*/
	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
		true, false,
		D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};
	gpipelineDesc.BlendState.RenderTarget[0].BlendEnable = false;
	//gpipelineDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//gpipelineDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	gpipelineDesc.InputLayout.pInputElementDescs = inputElements.data();
	gpipelineDesc.InputLayout.NumElements = inputElements.size();
	gpipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipelineDesc.NumRenderTargets = _numBuffers;
	for (int i = 0; i < _numBuffers; i++)
	{
		gpipelineDesc.RTVFormats[i] = _format;
	}
	if (_depthBuffer)
	{
		gpipelineDesc.BlendState.RenderTarget[0] = defaultRenderTargetBlendDesc;
		gpipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		//gpipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		gpipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	}
	gpipelineDesc.SampleDesc.Count = 1;
	gpipelineDesc.SampleDesc.Quality = 0;
	
	auto result = _dx->GetDevice()->CreateGraphicsPipelineState(&gpipelineDesc, IID_PPV_ARGS(_pipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}


//モデルの追加
void Renderer::AddModel(std::shared_ptr<Model> model)
{
	_models.emplace_back(model);
}

//モデルの更新
void Renderer::Update(bool isStart)
{
	for (auto& _models : _models) {
		_models->Update(isStart);
	}
}

//モデルの描画
void Renderer::DrawModel() const
{
	for (auto& _models : _models) {
		_models->Draw();
	}
}

//ペラポリゴンの描画
void Renderer::DrawPera() const
{
	_pera->Draw();
}

//描画前の処理
void Renderer::BeforeDraw(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelinestate,
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature) const
{
	_dx->GetCommandList()->SetPipelineState(pipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(rootsignature.Get());
}

//バリアの状態を設定（ペラレンダラー用）
void Renderer::SetBarrierState(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		before,
		after
	);
	_dx->GetCommandList()->ResourceBarrier(1, &barrier);

}

//バリアの状態を設定
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

//レンダーターゲットの設定
void Renderer::SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, bool isBackBuffer)
{
	int numDescs = rtvHeap->GetDesc().NumDescriptors;
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	auto baseHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize =
		_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	uint32_t offset = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle;
	//バックバッファの場合
	if (isBackBuffer)
	{
		numDescs = 1;
		offset = rtvIncSize * _dx->GetSwapChain()->GetCurrentBackBufferIndex();
		handle.InitOffsetted(baseHandle, offset);
		rtvHandles.emplace_back(handle);
	}
	else
	{
		for (int i = 0; i < numDescs; ++i)
		{
			handle.InitOffsetted(baseHandle, offset);
			rtvHandles.emplace_back(handle);
			offset += rtvIncSize;
		}
	}
	//デプスバッファがある場合
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

//ビューポートとシザー矩形の設定
void Renderer::SetVPAndSR(UINT windowWidth, UINT windowHeight)
{
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, windowWidth, windowHeight);
	CD3DX12_RECT rc(0, 0, windowWidth, windowHeight);
	_dx->GetCommandList()->RSSetViewports(1, &vp);
	_dx->GetCommandList()->RSSetScissorRects(1, &rc);
}

//SRVの設定
void Renderer::SetSRVDesc(DXGI_FORMAT format)
{
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}

//バッファーの作成
bool Renderer::CreateBuffers()
{
	_buffers.resize(_numBuffers);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = _dx->GetBackBuff()->GetDesc();
	resDesc.Width = resWidth;
	resDesc.Height = resHeight;
	resDesc.Format = _format;

	CD3DX12_CLEAR_VALUE clearValue(_format, clsClr);
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
		IID_PPV_ARGS(_rtvHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	auto handle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = _format;
	for (auto& res : _buffers) {
		_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			&rtvDesc,
			handle);
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	return true;
}

//デプスバッファーの作成
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
		IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	auto handle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_dx->GetDevice()->CreateDepthStencilView(
		_depthBuffer.Get(),
		&dsvDesc,
		handle
	);
	return true;
}


//入力レイアウトの追加
void Renderer::AddElement(const char* semantics, DXGI_FORMAT format)
{
	D3D12_INPUT_ELEMENT_DESC element = {
			semantics, 0, format, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
	};
	inputElements.emplace_back(element);
}
