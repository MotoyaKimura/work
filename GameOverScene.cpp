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

bool GameOverScene::SceneInit()
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

	_gameOverTex.reset(new Texture(Application::_dx));
	_gameOverTex->Init(L"texture/GameOver.png");
	_pera->SetSRV(_gameOverTex->GetTexBuff(), _gameOverTex->GetMetadata().format);
	_restartTex.reset(new Texture(Application::_dx));
	_restartTex->Init(L"texture/restart.png");
	_pera->SetSRV(_restartTex->GetTexBuff(), _restartTex->GetMetadata().format);
	_titleTex.reset(new Texture(Application::_dx));
	_titleTex->Init(L"texture/BackToTitle.png");
	_pera->SetSRV(_titleTex->GetTexBuff(), _titleTex->GetMetadata().format);


	_peraRenderer->RendererInit(
		L"GameOverPeraVertexShader.hlsl", "VS",
		L"GameOverPeraPixelShader.hlsl", "PS"
	);


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

	return true;
}

void GameOverScene::SceneUpdate(void)
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
	
	_restartButton->Update();
	_titleButton->Update();
	_peraRenderer->Update();
}

void GameOverScene::SceneRender(void)
{

	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();


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

void GameOverScene::SceneFinal(void)
{
	Application::SetMenu();
	
	_restartButton->Hide();
	_titleButton->Hide();
	
	_restartButton->Destroy();
	_titleButton->Destroy();
}

void GameOverScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* GameOverScene::GetSceneName(void)
{
	return "GameOverScene";
}


GameOverScene::GameOverScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

GameOverScene::~GameOverScene()
{
}
