#include "SSAO.h"
#include "Application.h"
#include "Wrapper.h"

bool SSAO::Init()
{
	SetNumBuffers(1);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R32_FLOAT);
	SetClearValue(1.0f, 1.0f, 1.0f, 1.0f);
	if (!CreateBuffers()) return false;

	if (FAILED(!CompileShaderFile(L"SSAOVertexShader.hlsl", "ssaoVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"SSAOPixelShader.hlsl", "ssaoPS", "ps_5_0", psBlob))) return false;
	SetRootSigParam();
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!PipelineStateInit()) return false;
	return true;
}

void SSAO::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap,false);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawPera();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SSAO::SetRootSigParam()
{
	CD3DX12_DESCRIPTOR_RANGE descTblRange;
	//ペラポリゴン用テクスチャ、視点深度テクスチャ
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0);
	ranges.emplace_back(descTblRange);
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	ranges.emplace_back(descTblRange);
	
	rootParam.InitAsDescriptorTable(2, ranges.data());
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);
	samplers.emplace_back(samplerDesc);
}


SSAO::SSAO(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard, std::vector<std::shared_ptr<Model>> models, std::shared_ptr<Camera> camera)
	: Renderer(dx, pera, _keyboard, models, camera), _dx(dx), _pera(pera), _keyboard(_keyboard), _models(models), _camera(camera)
{
}

SSAO::~SSAO()
{
}