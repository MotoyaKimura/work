#include "MenuScene.h"
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

//���j���[�V�[���N���X
MenuScene::MenuScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

MenuScene::~MenuScene()
{
}

//�V�[���̏�����
bool MenuScene::SceneInit()
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
void MenuScene::SceneUpdate(void)
{
	_peraRenderer->FadeIn();
	ButtonUpdate();
}

//�V�[���̕`��
void MenuScene::SceneRender(void)
{
	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	//�߂�{�^���������ꂽ�猳�̃Q�[���V�[���֑J��
	if (_backButton->IsActive() || !Application::GetMenu())
	{
		_backButton->Hide();
		_restartButton->Hide();
		_titleButton->Hide();
		if (_peraRenderer->FadeOut())
		{
			SceneFinal();
			SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);
			_controller.PopScene();
			return;
		}
	}

	//���X�^�[�g�{�^���������ꂽ��V�����Q�[���V�[���֑J��
	if (_restartButton->IsActive())
	{
		_backButton->Hide();
		_restartButton->Hide();
		_titleButton->Hide();
		if (_peraRenderer->FadeOut())
		{
			SceneFinal();
			_controller.ChangeScene(new GameScene(_controller));
			return;
		}
	}

	//�^�C�g���{�^���������ꂽ��^�C�g���V�[���֑J��
	if (_titleButton->IsActive())
	{
		_backButton->Hide();
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

//�V�[���̏I��
void MenuScene::SceneFinal(void)
{
	Application::SetMenu();
	_backButton->Destroy();
	_restartButton->Destroy();
	_titleButton->Destroy();
}

//�V�[���̃��T�C�Y
void MenuScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* MenuScene::GetSceneName(void)
{
	return "MenuScene";
}


bool MenuScene::PeraInit()
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

bool MenuScene::CameraInit()
{
	//�J�����̏�����
	_camera.reset(new Camera(Application::_dx, _pera));
	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("�J�����̏������G���[\n ");
		return false;
	}
	return true;
}

bool MenuScene::TextureInit()
{
	//�e�N�X�`���̏�����
	_textures.resize(6);
	_textures[0].reset(new Texture(Application::_dx, L"texture/start.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/restart.png"));
	_textures[2].reset(new Texture(Application::_dx, L"texture/BackToTitle.png"));
	_textures[3].reset(new Texture(Application::_dx, L"texture/backGround.png"));
	_textures[4].reset(new Texture(Application::_dx, L"texture/menu.png"));
	_textures[5].reset(new Texture(Application::_dx, L"texture/back.png"));
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

bool MenuScene::RendererBuffInit()
{
	//�����_���[�̃o�b�t�@�[������
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	if(!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRenderer�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	return true;
}

bool MenuScene::RendererDrawInit()
{
	//�����_���[�̕`�揉����
	if (!_peraRenderer->RendererInit(
		L"MenuPeraVertexShader.hlsl", "VS",
		L"MenuPeraPixelShader.hlsl", "PS"
	))
	{
		Application::DebugOutputFormatString("PeraRenderer�̃����_���[�������G���[\n ");
		return false;
	}
	return true;
}

void MenuScene::ButtonInit()
{
	//�{�^���̏�����
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_backButton.reset(new Button("Back"));
	_backButton->Create(
		_T("��"),
		(int)(dx * 0.0f), (int)(dy * 0.0f),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)2
	);

	_restartButton.reset(new Button("Restart"));
	_restartButton->Create(
		_T("Restart"),
		(int)(dx * 0.45), (int)(dy * 0.4),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)3
	);

	_titleButton.reset(new Button("Title"));
	_titleButton->Create(
		_T("Back to Title"),
		(int)(dx * 0.45), (int)(dy * 0.6),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)4
	);

}

void MenuScene::ButtonUpdate()
{
	//�N���b�N����
	_backButton->Update();
	_restartButton->Update();
	_titleButton->Update();


	//�{�^����������Ă��Ȃ�
	if (!_backButton->IsActive() && Application::GetMenu() && !_restartButton->IsActive() && !_titleButton->IsActive())
	{
		//�z�o�[���Ƀ{�^�����t�F�[�h�C���E�A�E�g����
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
	else
	{
		_peraRenderer->HoverCntReset();
	}
	
}
