#include "SSAO.h"
#include "Application.h"
#include "Wrapper.h"

bool SSAO::Init()
{
	if (FAILED(!CompileShaderFile(L"SSAOVertexShader.hlsl", "ssaoVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"SSAOPixelShader.hlsl", "ssaoPS", "ps_5_0", psBlob))) return false;
	if (!PeraRootSignatureInit()) return false;
	if (!SSAOPipelineStateInit()) return false;
	SetNumBuffers(1);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R32_FLOAT);
	SetClearValue(1.0f, 1.0f, 1.0f, 1.0f);
	if (!CreateBuffers()) return false;

	return true;
}

void SSAO::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_ssaoPipelinestate.Get(), peraRootsignature.Get());
	DrawPera();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SSAO::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	auto handle = heap->GetCPUDescriptorHandleForHeapStart();

	for (auto& res : _buffers) {
		handle = heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
		_dx->GetDevice()->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}
}

SSAO::SSAO(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard)
	: Renderer(dx, pera, _keyboard), _dx(dx), _pera(pera), _keyboard(_keyboard)
{
}

SSAO::~SSAO()
{
}