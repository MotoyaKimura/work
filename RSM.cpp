#include "RSM.h"
#include "Application.h"
#include "Wrapper.h"


bool RSM::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "shadeVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "RSMPS", "ps_5_0", psBlob))) return false;
	if (!RootSignatureInit()) return false;
	if (!ShadowPipelineStateInit()) return false;
	SetNumBuffers(3);
	SetResSize(rsm_difinition, rsm_difinition);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;
	
	return true;
}

void RSM::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	SetSRVDesc(DXGI_FORMAT_R8G8B8A8_UNORM);

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _buffers) {
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
		_depthBuffer.Get(),
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
		_depthBuffer.Get(),
		&srvDesc,
		handle);
}

void RSM::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap);
	SetVPAndSR(rsm_difinition, rsm_difinition);
	BeforeDraw(_shadowPipelinestate.Get(), rootsignature.Get());
	
	DrawModel();

	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

RSM::RSM(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard)
	: Renderer(dx, pera, _keyboard), _dx(dx), _pera(pera), _keyboard(_keyboard)
{
}

RSM::~RSM()
{
}
