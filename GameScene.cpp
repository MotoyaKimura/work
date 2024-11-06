#include "GameScene.h"
#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Renderer.h"
#include "Keyboard.h"


bool GameScene::SceneInit(void)
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_keyboard.reset(new Keyboard(Application::hwnd));

	_renderer.reset(new Renderer(Application::_dx, _pera, _keyboard));
	if (!_renderer->Init())
	{
		Application::DebugOutputFormatString("レンダラー周りの初期化エラー\n ");
		return false;
	}

	modelNum = 4;
	_models.resize(modelNum);
	_models[0].reset(new Model(Application::_dx, "modelData/bunny/bunny.obj"));
	_models[0]->Move(30, 0, 30);
	_models[1] = std::make_shared<Model>(Application::_dx, "modelData/RSMScene/floor/floor.obj");
	_models[1]->Move(30, 0, 30);
	_models[2] = std::make_shared<Model>(Application::_dx, "modelData/RSMScene/wall/wall_red.obj");
	_models[2]->Move(30, 30, 0);
	_models[3] = std::make_shared<Model>(Application::_dx, "modelData/RSMScene/wall/wall_green.obj");
	_models[3]->Move(0, 30, 30);
	for (auto model : _models)
	{
		if (!model->Init())
		{
			Application::DebugOutputFormatString("モデルの初期化エラー\n ");
			return false;
		}
		_renderer->AddModel(model);
	}

	

	return true;
}

void GameScene::SceneUpdate(void)
{
	_renderer->Move();
	_renderer->Update();
}


void GameScene::SceneRender(void)
{
	

	Application::_dx->BeginDrawShade();
	_renderer->BeforeDrawShade();
	_renderer->DrawShade();
	Application::_dx->EndDrawShade();

	Application::_dx->BeginDrawTeapot();
	_renderer->BeforeDrawTeapot();
	_renderer->DrawTeapot();
	Application::_dx->EndDrawTeapot();

	Application::_dx->BeginDrawSSAO();
	_renderer->BeforeDrawSSAO();
	_renderer->DrawSSAO();
	Application::_dx->EndDrawSSAO();

	Application::_dx->BeginDrawPera();
	_renderer->BeforeDrawPera();
	_renderer->DrawPera();
	Application::_dx->EndDrawPera();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
}

void GameScene::SceneFinal(void)
{
}

void GameScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
	_renderer->ResizeBuffers();
}

const char* GameScene::GetSceneName(void)
{
	return "GameScene";
}

GameScene::GameScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
	SceneInit();
}

GameScene::~GameScene()
{
}