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

bool MenuScene::SceneInit()
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_camera.reset(new Camera(Application::_dx, _pera));

	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("カメラの初期化エラー\n ");
		return false;
	}

	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer->Init();

	_startTex.reset(new Texture(Application::_dx, _pera));
	_startTex->Init("texture/start.png");
	_startTex->SetSRV();
	_restartTex.reset(new Texture(Application::_dx, _pera));
	_restartTex->Init("texture/restart.png");
	_restartTex->SetSRV();
	_titleTex.reset(new Texture(Application::_dx, _pera));
	_titleTex->Init("texture/BackToTitle.png");
	_titleTex->SetSRV();
	_backGroundTex.reset(new Texture(Application::_dx, _pera));
	_backGroundTex->Init("texture/backGround.png");
	_backGroundTex->SetSRV();
	_menuTex.reset(new Texture(Application::_dx, _pera));
	_menuTex->Init("texture/menu.png");
	_menuTex->SetSRV();
	_backTex.reset(new Texture(Application::_dx, _pera));
	_backTex->Init("texture/back.png");
	_backTex->SetSRV();

	_peraRenderer->RendererInit(
		L"MenuPeraVertexShader.hlsl", "VS", 
		L"MenuPeraPixelShader.hlsl", "PS"
	);


	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_backButton.reset(new Button("Back"));
	_backButton->Create(
		_T("←"), 
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

	return true;
}

void MenuScene::SceneUpdate(void)
{
	if (_backButton->IsHover())
	{
		_peraRenderer->HoverButton(_backButton->GetName());
	}
	else if(_restartButton->IsHover())
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
	_backButton->Update();
	_restartButton->Update();
	_titleButton->Update();
	_peraRenderer->Update();
}

void MenuScene::SceneRender(void)
{

	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	if(_backButton->IsActive() || !Application::GetMenu())
	{
		if(_peraRenderer->FadeOut())
		{
			SceneFinal();
			SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);
			_controller.PopScene();
			return;
		}
	}

	if (_restartButton->IsActive())
	{
		if (_peraRenderer->FadeOut())
		{
			SceneFinal();
			_controller.ChangeScene(new GameScene(_controller));
			return;
		}
	}

	if (_titleButton->IsActive())
	{
		if (_peraRenderer->FadeOut())
		{
			SceneFinal();
			_controller.ChangeScene(new TitleScene(_controller));
			return;
		}
	}

}

void MenuScene::SceneFinal(void)
{
	Application::SetMenu();
	_backButton->Hide();
	_restartButton->Hide();
	_titleButton->Hide();
	_backButton->Destroy();
	_restartButton->Destroy();
	_titleButton->Destroy();
}

void MenuScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* MenuScene::GetSceneName(void)
{
	return "MenuScene";
}


MenuScene::MenuScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

MenuScene::~MenuScene()
{
}
