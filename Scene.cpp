#include "Scene.h"
#include "Wrapper.h"
#include "Application.h"
#include "Pera.h"
#include "Model.h"
#include "Renderer.h"
#include "Keyboard.h"


std::shared_ptr<Pera> Scene::_pera = nullptr;
std::shared_ptr<Renderer> Scene::_renderer = nullptr;
std::shared_ptr<Model> Scene::_model = nullptr;
std::shared_ptr<Model> Scene::_model2 = nullptr;
std::shared_ptr<Model> Scene::_model3 = nullptr;
std::shared_ptr<Model>Scene::_model4 = nullptr;
std::shared_ptr<Keyboard> Scene::_keyboard = nullptr;

// 初期化
bool Scene::SceneInit()
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

	_model.reset(new Model(Application::_dx));
	if (!_model->Load("modelData/bunny/bunny.obj")) return false;

	if (!_model->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model->Move(30, 0, 30);

	_renderer->AddModel(_model);

	_model2 = std::make_shared<Model>(Application::_dx);
	if (!_model2->Load("modelData/RSMScene/floor/floor.obj")) return false;
	if (!_model2->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model2->Move(30, 0, 30);
	_renderer->AddModel(_model2);

	_model3 = std::make_shared<Model>(Application::_dx);
	if (!_model3->Load("modelData/RSMScene/wall/wall_red.obj")) return false;
	if (!_model3->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model3->Move(30, 30, 0);
	_renderer->AddModel(_model3);

	_model4 = std::make_shared<Model>(Application::_dx);
	if (!_model4->Load("modelData/RSMScene/wall/wall_green.obj")) return false;
	if (!_model4->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model4->Move(0, 30, 30);
	_renderer->AddModel(_model4);

	return true;
}

bool Scene::SceneInit2()
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

	_model.reset(new Model(Application::_dx));
	if (!_model->Load("modelData/bunny/bunny.obj")) return false;

	if (!_model->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model->Move(30, 0, 30);

	_renderer->AddModel(_model);

	/*_model2 = std::make_shared<Model>(Application::_dx);
	if (!_model2->Load("modelData/RSMScene/floor/floor.obj")) return false;
	if (!_model2->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model2->Move(30, 0, 30);
	_renderer->AddModel(_model2);*/

	_model3 = std::make_shared<Model>(Application::_dx);
	if (!_model3->Load("modelData/RSMScene/wall/wall_red.obj")) return false;
	if (!_model3->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model3->Move(30, 30, 0);
	_renderer->AddModel(_model3);

	_model4 = std::make_shared<Model>(Application::_dx);
	if (!_model4->Load("modelData/RSMScene/wall/wall_green.obj")) return false;
	if (!_model4->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model4->Move(0, 30, 30);
	_renderer->AddModel(_model4);

	return true;
}

// 解放
void Scene::SceneFinal(void)
{
}

// 更新
void Scene::SceneUpdate(void)
{
	Application::_dx->ResizeBackBuffers();
	_renderer->ResizeBuffers();
	
}

// 描画
void Scene::SceneRender(void)
{
	_renderer->Move();
	_renderer->Update();

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

// シーン処理の設定
SceneProc Scene::SetupTestScene(void)
{
	SceneProc proc =
	{
		"Test",
		SceneInit,
		SceneFinal,
		SceneUpdate,
		SceneRender,
	};
	return proc;
}

SceneProc Scene::SetupTestScene2(void)
{
	SceneProc proc =
	{
		"Test",
		SceneInit2,
		SceneFinal,
		SceneUpdate,
		SceneRender,
	};
	return proc;
}

Scene::Scene()
{
}

Scene::~Scene()
{
}