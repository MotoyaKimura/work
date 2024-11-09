#include "ModelRenderer.h"
#include "Application.h"
#include "Wrapper.h"


bool ModelRenderer::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!RootSignatureInit()) return false;
	if (!PipelineStateInit()) return false;
	SetNumBuffers(2);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;

	return true;
}

void ModelRenderer::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawModel();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}


void ModelRenderer::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

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


ModelRenderer::ModelRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> keyboard)
	: Renderer(dx, pera, keyboard), _dx(dx), _pera(pera), _keyboard(keyboard)
{
}

ModelRenderer::~ModelRenderer()
{
}