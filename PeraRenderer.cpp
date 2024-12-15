#include "PeraRenderer.h"
#include "Application.h"
#include "Model.h"
#include "Wrapper.h"
#include "Pera.h"

//�y���|���S���̕`��N���X
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

//�o�b�t�@�[������
bool PeraRenderer::Init(void)
{
	peraEffectBuffInit();
	_pera->SetCBV(_peraEffectBuff);
	return true;
}

//�V�F�[�_�[�A���[�g�V�O�l�`���A�p�C�v���C��������
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

//�`��
void PeraRenderer::Draw()
{
	SetBarrierState(_dx->GetBackBuff(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_dx->GetRTVHeap(), nullptr, true);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawPera();
	SetBarrierState(_dx->GetBackBuff(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

//���C�v�o�b�t�@�[������
bool PeraRenderer::peraEffectBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(peraEffectBuffData) + 0xff) & ~0xff);

	_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_peraEffectBuff.ReleaseAndGetAddressOf())
	);
	auto result = _peraEffectBuff->Map(0, nullptr, (void**)&_peraEffectBuffData);
	if (FAILED(result)) return false;
	DataReset();
	return true;
}

//�f�[�^���Z�b�g
void PeraRenderer::DataReset()
{
	_peraEffectBuffData->_startWipeOpen = Application::GetWindowSize().cy / 2;
	_peraEffectBuffData->_endWipeClose = Application::GetWindowSize().cy / 10;
	_peraEffectBuffData->_fade = 0.0f;
	_peraEffectBuffData->_gameOverFade = 1.0f;
	_peraEffectBuffData->_clearFade = 0.0f;
	_peraEffectBuffData->_monochromeRate = 0.0f;
	_peraEffectBuffData->ScreenWidth = Application::GetWindowSize().cx;
	_peraEffectBuffData->ScreenHeight = Application::GetWindowSize().cy;
	_peraEffectBuffData->_startHoverCnt = 1.0f;
	_peraEffectBuffData->_restartHoverCnt = 1.0f;
	_peraEffectBuffData->_titleHoverCnt = 1.0f;
	_peraEffectBuffData->_milliSecond = 0;
	_peraEffectBuffData->credit = 0.0f;
	_peraEffectBuffData->_isPause = Application::GetPause();
}


//�{�^���z�o�[����
void PeraRenderer::HoverButton(std::string buttonName)
{
	if (buttonName == "Start")
	{
		_peraEffectBuffData->_startHoverCnt = (sin(hoverCnt + DirectX::XM_PIDIV2) + 1) * 0.5;
	}
	else if (buttonName == "Restart")
	{
		_peraEffectBuffData->_restartHoverCnt = (sin(hoverCnt + DirectX::XM_PIDIV2) + 1) * 0.5;
	}
	else if (buttonName == "Title")
	{
		_peraEffectBuffData->_titleHoverCnt = (sin(hoverCnt + DirectX::XM_PIDIV2) + 1) * 0.5;
	}
	hoverCnt += 0.05f;
}

//�z�o�[�J�E���g���Z�b�g
void PeraRenderer::HoverCntReset()
{
	hoverCnt = 0.0f;
	if (_peraEffectBuffData->_startHoverCnt < 1.0f)
		_peraEffectBuffData->_startHoverCnt += 0.1;
	if (_peraEffectBuffData->_restartHoverCnt < 1.0f)
		_peraEffectBuffData->_restartHoverCnt += 0.1;
	if (_peraEffectBuffData->_titleHoverCnt < 1.0f)
		_peraEffectBuffData->_titleHoverCnt += 0.1;
}

//���Ԍv��
void PeraRenderer::CalcTime()
{
	_peraEffectBuffData->_milliSecond = timeGetTime() - startTime;
}

//�^�C�����~�b�g����
bool PeraRenderer::TimeLimit()
{
	if (_peraEffectBuffData->_milliSecond >= 30000)
		return true;
	return false;
}

//�G���f�B���O���[��
void PeraRenderer::creditRoll()
{
	_peraEffectBuffData->credit += 0.001f;
}

//�|�[�Y����
bool PeraRenderer::IsPause()
{
	_peraEffectBuffData->_isPause = Application::GetPause();
	if (_peraEffectBuffData->_isPause)
	{
		while (ShowCursor(true) < 0);
	}
	return _peraEffectBuffData->_isPause;
}

//���Ԓ�~
void PeraRenderer::TimeStop()
{
	startTime = timeGetTime() - _peraEffectBuffData->_milliSecond;
}

//�t�F�[�h�C��
void PeraRenderer::FadeIn()
{
	if (_peraEffectBuffData->_fade >= 1.0f) return;
	_peraEffectBuffData->_fade += 0.1f;
	return;
}

//�t�F�[�h�A�E�g
bool PeraRenderer::FadeOut()
{
	if (_peraEffectBuffData->_fade <= 0.0f) return true;
	_peraEffectBuffData->_fade -= 0.2f;
	return false;
}

//�Q�[���I�[�o�[���̃t�F�[�h�A�E�g
bool PeraRenderer::GameOverFadeOut()
{
	if (_peraEffectBuffData->_gameOverFade <= 0.0f) return true;
	_peraEffectBuffData->_gameOverFade -= 0.005f;
	return false;
}

//�N���A���̃t�F�[�h�A�E�g
bool PeraRenderer::ClearFadeOut()
{
	if (_peraEffectBuffData->_clearFade >= 1.0f) return true;
	_peraEffectBuffData->_clearFade += 0.005f;
	return false;
}

//���C�v�I�[�v��
bool PeraRenderer::IsWipeOpen()
{
	if (_peraEffectBuffData->_startWipeOpen < 0)
		return false;
	_peraEffectBuffData->_startWipeOpen -= pow(wipeOpenCnt++, 2) * 0.001;
	return true;
}

//���C�v�N���[�Y
bool PeraRenderer::IsWipeClose()
{
		if (_peraEffectBuffData->_endWipeClose > Application::GetWindowSize().cy / 2)
			return true;
		_peraEffectBuffData->_endWipeClose += pow(wipeCloseCnt++, 2) * 0.001;
	return false;
}

//���m�N������J���[��
bool PeraRenderer::MonochromeToColor()
{
	if (_peraEffectBuffData->_monochromeRate >= 1.0f) return false;
	_peraEffectBuffData->_monochromeRate += pow(monochromeCnt, 2);
	monochromeCnt += 0.001f;
	return true;
}

