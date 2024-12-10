#include "ModelRenderer.h"
#include "Application.h"
#include "Wrapper.h"
#include "Model.h"
#include "Pera.h"


bool ModelRenderer::Init()
{
	SetNumBuffers(2);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;
	for (auto RTBuff : GetBuffers())
		_pera->SetSRV(RTBuff, GetFormat());
	_pera->SetSRV(GetDepthBuffer(), DXGI_FORMAT_R32_FLOAT);
	
	return true;
}

bool ModelRenderer::RendererInit(std::wstring VShlslFile, std::string VSEntryPoint, std::wstring PShlslFile, std::string PSEntryPoint)
{
	if (FAILED(!CompileShaderFile(VShlslFile, VSEntryPoint, "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(PShlslFile, PSEntryPoint, "ps_5_0", psBlob))) return false;
	SetRootSigParamForModel(_models[0]->GetCbvDescs(), _models[0]->GetSrvDescs());
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	AddElement("MORPHPOSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("MORPHUV", DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddElement("SDEFC", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("SDEFRZERO", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("SDEFRONE", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("QZERO", DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddElement("QONE", DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddElement("BONENO", DXGI_FORMAT_R32G32B32A32_UINT);
	AddElement("WEIGHT", DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddElement("WEIGHTTYPE", DXGI_FORMAT_R8_UINT);
	if (!PipelineStateInit()) return false;
	return true;
}

void ModelRenderer::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap,false);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawModel();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}


ModelRenderer::ModelRenderer(
	std::shared_ptr<Wrapper> dx,
	std::shared_ptr<Pera> pera, 
	std::shared_ptr<Keyboard> keyboard, 
	std::vector<std::shared_ptr<Model>> models, 
	std::shared_ptr<Camera> camera
) : Renderer(dx, pera, keyboard, models, camera), _dx(dx), _pera(pera), _keyboard(keyboard), _models(models), _camera(camera)
{
}

ModelRenderer::~ModelRenderer()
{
}