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
#include "TitleScene.h"

bool HowToPlayScene::SceneInit()
{
	Application::SetIsShowHowToPlay(true);

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

	_howToPlayTex.reset(new Texture(Application::_dx, L"texture/asobikata.png"));
	if(!_howToPlayTex->Init())
	{
		Application::DebugOutputFormatString("テクスチャの初期化エラー\n ");
		return false;
	}
	_pera->SetSRV(_howToPlayTex->GetTexBuff(), _howToPlayTex->GetMetadata().format);
	

	_peraRenderer->RendererInit(
		L"HowToPlayPeraVertexShader.hlsl", "VS",
		L"HowToPlayPeraPixelShader.hlsl", "PS"
	);


	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_backButton.reset(new Button("Back"));
	_backButton->Create(
		_T("←"),
		(int)(dx * 0.025f), (int)(dy * 0.1f),
		(int)(dx * 0.05), (int)(dy * 0.05),
		(HMENU)9
	);


	return true;
}

void HowToPlayScene::SceneUpdate(void)
{
	if (_backButton->IsHover())
	{
		_peraRenderer->HoverButton(_backButton->GetName());
	}
	else
	{
		_peraRenderer->HoverCntReset();
	}
	_backButton->Update();
	_peraRenderer->Update();
}

void HowToPlayScene::SceneRender(void)
{

	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	if (_backButton->IsActive() )
	{
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

void HowToPlayScene::SceneFinal(void)
{
	Application::SetMenu();

	_backButton->Destroy();
}

void HowToPlayScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* HowToPlayScene::GetSceneName(void)
{
	return "HowToPlayScene";
}


HowToPlayScene::HowToPlayScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

HowToPlayScene::~HowToPlayScene()
{
}
