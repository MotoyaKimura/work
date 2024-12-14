#include "GameOverScene.h"
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
#include "TitleScene.h"

GameOverScene::GameOverScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

GameOverScene::~GameOverScene()
{
}

//�V�[���̏�����
bool GameOverScene::SceneInit()
{
	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	if (!TextureInit()) return false;
	if (!RendererBuffInit()) return false;
	if (!RendererDrawInit()) return false;
	ButtonInit();
	return true;
}

//�V�[���̍X�V
void GameOverScene::SceneUpdate(void)
{
	//�|�[�Y����Ə��߂̃t�F�[�h�C��
	_peraRenderer->FadeIn();
	ButtonUpdate();
}

//�V�[���̕`��
void GameOverScene::SceneRender(void)
{
	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
}

//�V�[���̏I��
void GameOverScene::SceneFinal(void)
{
	Application::SetMenu();
	_restartButton->Destroy();
	_titleButton->Destroy();
}

//�V�[���̃��T�C�Y
void GameOverScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* GameOverScene::GetSceneName(void)
{
	return "GameOverScene";
}

bool GameOverScene::PeraInit()
{
	//�y���|���S���̏�����
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("�y���|���S���̏������G���[\n ");
		return false;
	}
	return true;
}

bool GameOverScene::CameraInit()
{
	//�J�����̏�����
	_camera.reset(new Camera(Application::_dx, _pera));
	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("�J�����̏������G���[\n ");
		return false;
	}
}

bool GameOverScene::RendererBuffInit()
{
	//�����_���[�̃o�b�t�@�[������
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	if (!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRenderer�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	return true;
}

bool GameOverScene::TextureInit()
{
	//�e�N�X�`���̏�����
	_textures.resize(3);
	_textures[0].reset(new Texture(Application::_dx, L"texture/GameOver.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/restart.png"));
	_textures[2].reset(new Texture(Application::_dx, L"texture/BackToTitle.png"));
	for (auto tex : _textures)
	{
		if (!tex->Init())
		{
			Application::DebugOutputFormatString("�e�N�X�`���̏������G���[\n ");
			return false;
		}
		_pera->SetSRV(tex->GetTexBuff(), tex->GetMetadata().format);
	}
	return true;
}

bool GameOverScene::RendererDrawInit()
{
	//�e�탌���_���[�N���X�̕`�敔���̏�����
	if (!_peraRenderer->RendererInit(
		L"GameOverPeraVertexShader.hlsl", "VS",
		L"GameOverPeraPixelShader.hlsl", "PS"
	))
	{
		Application::DebugOutputFormatString("PeraRenderer�̃����_���[�������G���[\n ");
		return false;
	}
	return true;
}

void GameOverScene::ButtonInit()
{
	//�{�^���̏�����
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_restartButton.reset(new Button("Restart"));
	_restartButton->Create(
		_T("Restart"),
		(int)(dx * 0.45), (int)(dy * 0.4),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)5
	);
	_titleButton.reset(new Button("Title"));
	_titleButton->Create(
		_T("Back to Title"),
		(int)(dx * 0.45), (int)(dy * 0.6),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)6
	);
}

//�{�^���̍X�V
void GameOverScene::ButtonUpdate()
{
	//�N���b�N����
	_restartButton->Update();
	_titleButton->Update();
	//�{�^����������Ă��Ȃ�
	//�z�o�[���Ƀ{�^�����t�F�[�h�C���E�A�E�g����
	if (!_restartButton->IsActive() && !_titleButton->IsActive())
	{
		if (_restartButton->IsHover())
		{
			_peraRenderer->HoverButton(_restartButton->GetName());
		}
		else if (_titleButton->IsHover())
		{
			_peraRenderer->HoverButton(_titleButton->GetName());
		}
		else
		{
			_peraRenderer->HoverCntReset();
		}
	}
	//�����ꂽ
	else {
		_peraRenderer->HoverCntReset();
		//���X�^�[�g�{�^���������ꂽ��
		if (_restartButton->IsActive())
		{
			_restartButton->Hide();
			_titleButton->Hide();
			if (_peraRenderer->FadeOut())
			{
				SceneFinal();
				_controller.ChangeScene(new GameScene(_controller));
				return;
			}
		}
		//�^�C�g���{�^���������ꂽ��
		if (_titleButton->IsActive())
		{
			_restartButton->Hide();
			_titleButton->Hide();
			if (_peraRenderer->FadeOut())
			{
				SceneFinal();
				_controller.ChangeScene(new TitleScene(_controller));
				return;
			}
		}
	}
	
}


