#include "HowToPlayScene.h"
#include "Application.h"
#include "GameScene.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Keyboard.h"
#include "PeraRenderer.h"
#include "Camera.h"
#include "Texture.h"
#include "Button.h"
#include <tchar.h>

//�V�ѕ��V�[���N���X
HowToPlayScene::HowToPlayScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

HowToPlayScene::~HowToPlayScene()
{
}

//�V�[���̏�����
bool HowToPlayScene::SceneInit()
{
	Application::SetIsShowHowToPlay(true);

	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	if (!TextureInit()) return false;
	if (!RendererBuffInit()) return false;
	if (!RendererDrawInit()) return false;
	ButtonInit();

	return true;
}

//�V�[���̍X�V
void HowToPlayScene::SceneUpdate(void)
{
	_peraRenderer->FadeIn();
	ButtonUpdate();
}

//�V�[���̕`��
void HowToPlayScene::SceneRender(void)
{

	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	//�{�^���������ꂽ
	if (_backButton->IsActive()) {
		_backButton->Hide();
		if (_peraRenderer->FadeOut())
		{
			SceneFinal();
			SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);
			_controller.PopScene();
			return;
		}
	}
}

//�V�[���̏I��
void HowToPlayScene::SceneFinal(void)
{
	_backButton->Destroy();
}

//�V�[���̃��T�C�Y
void HowToPlayScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* HowToPlayScene::GetSceneName(void)
{
	return "HowToPlayScene";
}

//�y���|���S���̏�����
bool HowToPlayScene::PeraInit()
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("�y���|���S���̏������G���[\n ");
		return false;
	}
	return true;
}

//�J�����̏�����
bool HowToPlayScene::CameraInit()
{
	_camera.reset(new Camera(Application::_dx, _pera));

	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("�J�����̏������G���[\n ");
		return false;
	}
	return true;
}

//�e�N�X�`���̏�����
bool HowToPlayScene::TextureInit()
{
	_howToPlayTex.reset(new Texture(Application::_dx, L"texture/asobikata.png"));
	if (!_howToPlayTex->Init())
	{
		Application::DebugOutputFormatString("�e�N�X�`���̏������G���[\n ");
		return false;
	}
	_pera->SetSRV(_howToPlayTex->GetTexBuff(), _howToPlayTex->GetMetadata().format);
	return true;
}

//�����_���[�̃o�b�t�@�[������
bool HowToPlayScene::RendererBuffInit()
{
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	if (!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRenderer�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	return true;
}

//�����_���[�̕`�揉����
bool HowToPlayScene::RendererDrawInit()
{
	if(!_peraRenderer->RendererInit(
		L"HowToPlayPeraVertexShader.hlsl", "VS",
		L"HowToPlayPeraPixelShader.hlsl", "PS"
	))
	{
		Application::DebugOutputFormatString("PeraRenderer�̃����_���[�������G���[\n ");
		return false;
	}
	return true;
}

//�{�^���̏�����
void HowToPlayScene::ButtonInit()
{
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_backButton.reset(new Button("Back"));
	_backButton->Create(
		_T("��"),
		(int)(dx * 0.025f), (int)(dy * 0.1f),
		(int)(dx * 0.05), (int)(dy * 0.05),
		(HMENU)9
	);

}

//�{�^���̍X�V
void HowToPlayScene::ButtonUpdate()
{
	//�N���b�N����
	_backButton->Update();

	
}
