#include "RSM.h"
#include "Application.h"
#include "Wrapper.h"
#include "Model.h"
#include "Pera.h"

bool RSM::Init()
{
	SetNumBuffers(3);
	SetResSize(rsm_difinition, rsm_difinition);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;
	for (auto model : _models)
		model->SetSRV(GetDepthBuffer(), DXGI_FORMAT_R32_FLOAT);
	for (auto RTBuff : GetBuffers())
		_pera->SetSRV(RTBuff, GetFormat());
	_pera->SetSRV(GetDepthBuffer(), DXGI_FORMAT_R32_FLOAT);
	return true;
}

bool RSM::RendererInit(std::wstring VShlslFile, std::string VSEntryPoint, std::wstring PShlslFile, std::string PSEntryPoint)
{
	if (FAILED(!CompileShaderFile(VShlslFile, VSEntryPoint, "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(PShlslFile, PSEntryPoint, "ps_5_0", psBlob))) return false;
	SetRootSigParam(_models[0]->GetCbvDescs(), _models[0]->GetSrvDescs());
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	AddElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
	if (!PipelineStateInit()) return false;

	return true;
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
	SetRenderTargets(_rtvHeap, _dsvHeap,false);
	SetVPAndSR(rsm_difinition, rsm_difinition);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawModel();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}


RSM::RSM(
	std::shared_ptr<Wrapper> dx, 
	std::shared_ptr<Pera> pera, 
	std::shared_ptr<Keyboard> keyboard, 
	std::vector<std::shared_ptr<Model>> models, 
	std::shared_ptr<Camera> camera
) : Renderer(dx, pera, keyboard, models, camera), _dx(dx), _pera(pera), _keyboard(keyboard), _models(models), _camera(camera)
{
}

RSM::~RSM()
{
}
