#include "PeraRenderer.h"
#include "Application.h"
#include "Model.h"
#include "Wrapper.h"
#include "Pera.h"

bool PeraRenderer::Init(void)
{
	wipeBuffInit();
	_pera->SetCBV(_wipeBuff);
	
	return true;
}

bool PeraRenderer::RendererInit(std::wstring VShlslFile, std::string VSEntryPoint, std::wstring PShlslFile, std::string PSEntryPoint)
{
	if (FAILED(!CompileShaderFile(VShlslFile, VSEntryPoint, "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(PShlslFile, PSEntryPoint, "ps_5_0", psBlob))) return false;
	SetRootSigParamForPera(_pera->GetCbvDescs(), _pera->GetSrvDescs());
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	SetNumBuffers(1);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!PipelineStateInit()) return false;
	_pera->SetViews();

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


bool PeraRenderer::wipeBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(wipeBuffData) + 0xff) & ~0xff);

	_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_wipeBuff.ReleaseAndGetAddressOf())
	);
	auto result = _wipeBuff->Map(0, nullptr, (void**)&_wipeBuffData);
	if (FAILED(result)) return false;
	DataReset();
	return true;
}

void PeraRenderer::DataReset()
{
	_wipeBuffData->_startWipeOpen = Application::GetWindowSize().cy / 2;
	_wipeBuffData->_endWipeClose = Application::GetWindowSize().cy / 10;
	_wipeBuffData->_fade = 0.0f;
	_wipeBuffData->_monochromeRate = 0.0f;
	_wipeBuffData->ScreenWidth = Application::GetWindowSize().cx;
	_wipeBuffData->ScreenHeight = Application::GetWindowSize().cy;
	_wipeBuffData->_startHoverCnt = 1.0f;
	_wipeBuffData->_restartHoverCnt = 1.0f;
	_wipeBuffData->_titleHoverCnt = 1.0f;
	_wipeBuffData->_isPause = Application::GetPause();
}

bool PeraRenderer::Update()
{
	if (IsPause()) return true;
	if (FadeIn()) return true;
}

void PeraRenderer::HoverButton(std::string buttonName)
{
	if (buttonName == "Start")
	{
		_wipeBuffData->_startHoverCnt = sin(hoverCnt + DirectX::XM_PIDIV2) * 0.5 + 1;
	}
	else if (buttonName == "Restart")
	{
		_wipeBuffData->_restartHoverCnt = sin(hoverCnt + DirectX::XM_PIDIV2) * 0.5 + 1;
	}
	else if (buttonName == "Title")
	{
		_wipeBuffData->_titleHoverCnt = sin(hoverCnt + DirectX::XM_PIDIV2) * 0.5 + 1;
	}
	hoverCnt += 0.05f;
}

void PeraRenderer::HoverCntReset()
{
	hoverCnt = 0.0f;
	if (_wipeBuffData->_startHoverCnt < 1.0f)
		_wipeBuffData->_startHoverCnt += 0.1;
	if (_wipeBuffData->_restartHoverCnt < 1.0f)
		_wipeBuffData->_restartHoverCnt += 0.1;
	if (_wipeBuffData->_titleHoverCnt < 1.0f)
		_wipeBuffData->_titleHoverCnt += 0.1;
}



bool PeraRenderer::IsPause()
{
	_wipeBuffData->_isPause = Application::GetPause();
	if (_wipeBuffData->_isPause)
		while (ShowCursor(true) < 0);
	return _wipeBuffData->_isPause;
}

bool PeraRenderer::FadeIn()
{
	if (_wipeBuffData->_fade >= 1.0f) return false;
	_wipeBuffData->_fade += 0.1f;
	return true;
}

bool PeraRenderer::FadeOut()
{
	if (_wipeBuffData->_fade <= 0.0f) return true;
	_wipeBuffData->_fade -= 0.2f;
	return false;
}

bool PeraRenderer::WipeStart()
{
	if (_wipeBuffData->_startWipeOpen < 0)
		return true;
	_wipeBuffData->_startWipeOpen -= pow(wipeOpenCnt++, 2) * 0.001;
	return false;
}

bool PeraRenderer::WipeEnd()
{
		if (_wipeBuffData->_endWipeClose > Application::GetWindowSize().cy / 2)
			return true;
		_wipeBuffData->_endWipeClose += pow(wipeCloseCnt++, 2) * 0.001;
	return false;
}

bool PeraRenderer::MonochromeToColor()
{
	if (_wipeBuffData->_monochromeRate >= 1.0f) return true;
	_wipeBuffData->_monochromeRate += pow(monochromeCnt, 2);
	monochromeCnt += 0.001f;
	return false;
}

PeraRenderer::PeraRenderer(std::shared_ptr<Wrapper> dx, 
	std::shared_ptr<Pera> pera,
	std::shared_ptr<Keyboard> keyboard, 
	std::vector<std::shared_ptr<Model>> models, 
	std::shared_ptr<Camera> camera
) : Renderer(dx, pera, keyboard, models, camera), _dx(dx), _pera(pera), _keyboard(keyboard), _models(models), _camera(camera)
{
}

PeraRenderer::~PeraRenderer()
{
}