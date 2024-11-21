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

	_texture.reset(new Texture(Application::_dx, _pera));
	_texture->Init(L"texture/start.png");

	_peraRenderer->RendererInit(
		L"MenuPeraVertexShader.hlsl", "VS", 
		L"MenuPeraPixelShader.hlsl", "PS"
	);


	_button.reset(new Button());
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_button->Create(
		_T("Menu"), 
		(int)(dx * 0.45), (int)(dy * 0.9), 
		(int)(dx * 0.1), (int)(dy * 0.1), 
		(HMENU)3
	);

	return true;
}

void MenuScene::SceneUpdate(void)
{
}

void MenuScene::SceneRender(void)
{

	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
	while (ShowCursor(true) < 0);
	if(_button->IsClicked())
	{
		SceneFinal();
		_controller.PopScene();
		return;
	}

}

void MenuScene::SceneFinal(void)
{
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
