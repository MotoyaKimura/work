#include "PeraRenderer.h"
#include "Application.h"
#include "Wrapper.h"

bool PeraRenderer::Init(void)
{
	if (FAILED(!CompileShaderFile(L"PeraVertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PeraPixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	SetRootSigParam();
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	SetNumBuffers(1);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	if (!PipelineStateInit()) return false;
	return true;
}


void PeraRenderer::Draw()
{
	SetBarrierState(_dx->GetBackBuff(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_dx->GetRTVHeap(), nullptr, true);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawPera();
	SetBarrierState(_dx->GetBackBuff(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void PeraRenderer::SetRootSigParam()
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

PeraRenderer::PeraRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> keyboard)
	: Renderer(dx, pera, keyboard), _dx(dx), _pera(pera), _keyboard(keyboard)
{
}

PeraRenderer::~PeraRenderer()
{
}