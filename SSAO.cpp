#include "SSAO.h"
#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"


//SSAOの初期化
SSAO::SSAO(
	std::shared_ptr<Wrapper> dx,
	std::shared_ptr<Pera> pera,
	std::shared_ptr<Keyboard> _keyboard,
	std::vector<std::shared_ptr<Model>> models,
	std::shared_ptr<Camera> camera
) : Renderer(dx, pera, _keyboard, models, camera), _dx(dx), _pera(pera), _keyboard(_keyboard), _models(models), _camera(camera)
{
}

SSAO::~SSAO()
{
}


//バッファー初期化
bool SSAO::Init()
{
	SetNumBuffers(1);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R32_FLOAT);
	SetClearValue(1.0f, 1.0f, 1.0f, 1.0f);
	if (!CreateBuffers()) return false;
	for (auto RTBuff : GetBuffers())
		_pera->SetSRV(RTBuff, GetFormat());
	return true;
}

//シェーダー、ルートシグネチャ、パイプライン初期化
bool SSAO::RendererInit(std::wstring VShlslFile, std::string VSEntryPoint, std::wstring PShlslFile, std::string PSEntryPoint)
{
	if (FAILED(!CompileShaderFile(VShlslFile, VSEntryPoint, "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(PShlslFile, PSEntryPoint, "ps_5_0", psBlob))) return false;
	SetRootSigParamForPera(_pera->GetCbvDescs(), _pera->GetSrvDescs());
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!PipelineStateInit()) return false;
	_pera->SetViews();
	return true;
}

//描画
void SSAO::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap,false);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawPera();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}