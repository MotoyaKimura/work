#include "RSM.h"
#include "Application.h"
#include "Wrapper.h"


bool RSM::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "shadeVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "RSMPS", "ps_5_0", psBlob))) return false;
	if (!RootSignatureInit()) return false;
	if (!ShadowPipelineStateInit()) return false;
	/*SetNumBuffers(3);
	SetResSize(rsm_difinition, rsm_difinition);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;*/
	if (!RSMBuffInit()) return false;
	if (!LightDepthBuffInit()) return false;
	return true;
}

bool RSM::RSMBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = _dx->GetBackBuff()->GetDesc();
	resDesc.Width = rsm_difinition;
	resDesc.Height = rsm_difinition;

	CD3DX12_CLEAR_VALUE clearValue(resDesc.Format, clsClr);
	_RSMBuff.resize(rsmBuffSize);
	for (auto& res : _RSMBuff) {
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
	heapDesc.NumDescriptors = _RSMBuff.size();

	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_RSMRTVHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	auto handle = _RSMRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _RSMBuff) {
		_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	return true;
}

bool RSM::LightDepthBuffInit()
{
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32_TYPELESS,rsm_difinition, rsm_difinition);
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
		IID_PPV_ARGS(_lightDepthBuff.ReleaseAndGetAddressOf())
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
		_lightDepthBuff.Get(),
		&dsvDesc,
		handle
	);
	return true;
}

void RSM::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	SetSRVDesc(DXGI_FORMAT_R8G8B8A8_UNORM);

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _RSMBuff) {
		handle = heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
		_dx->GetDevice()->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}
	handle = heap->GetCPUDescriptorHandleForHeapStart();
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
	_dx->GetDevice()->CreateShaderResourceView(
		_lightDepthBuff.Get(),
		&srvDesc,
		handle);

}

void RSM::SetDepthBuffToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
	_dx->GetDevice()->CreateShaderResourceView(
		_lightDepthBuff.Get(),
		&srvDesc,
		handle);
}

void RSM::Draw()
{
	SetBarrierState(_RSMBuff, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_RSMRTVHeap, _DSVHeap);
	SetVPAndSR(rsm_difinition, rsm_difinition);
	BeforeDraw(_shadowPipelinestate.Get(), rootsignature.Get());
	
	DrawModel();

	SetBarrierState(_RSMBuff, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

RSM::RSM(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard)
	: Renderer(dx, pera, _keyboard), _dx(dx), _pera(pera), _keyboard(_keyboard)
{
}

RSM::~RSM()
{
}
