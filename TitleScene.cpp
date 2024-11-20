#include "TitleScene.h"
#include "Application.h"
#include "GameScene.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Keyboard.h"
#include "RSM.h"
#include "ModelRenderer.h"
#include "SSAO.h"
#include "PeraRenderer.h"
#include "Camera.h"
#include "Texture.h"
#include "Button.h"
#include <tchar.h>

bool TitleScene::SceneInit()
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_camera.reset(new Camera(Application::_dx, _pera));

	if(!_camera->Init())
	{
		Application::DebugOutputFormatString("カメラの初期化エラー\n ");
		return false;
	}


	modelNum = 3;
	_models.resize(modelNum);
	_models[0].reset(new Model(Application::_dx, _camera, "modelData/bunny/bunny.obj"));
	_models[0]->Move(0, 0, 0);
	_models[1] = std::make_shared<Model>(Application::_dx, _camera, "modelData/RSMScene/wall/wall_green.obj");
	_models[1]->Move(2.5, 2.5, 0);
	_models[2] = std::make_shared<Model>(Application::_dx, _camera, "modelData/RSMScene/wall/wall_red.obj");
	_models[2]->Move(0, 2.5, 2.5);
	for (auto model : _models)
	{
		if (!model->Init())
		{
			Application::DebugOutputFormatString("モデルの初期化エラー\n ");
			return false;
		}
	}

	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));
	
	_keyboard->Init();

	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_rsm->Init();
	_modelRenderer->Init();
	_ssao->Init();
	_peraRenderer->Init();

	_texture.reset(new Texture(Application::_dx, _pera));
	_texture->Init(L"texture/start.png");

	_rsm->RendererInit(L"VertexShader.hlsl", "rsmVS", L"PixelShader.hlsl", "rsmPS");
	_modelRenderer->RendererInit(L"VertexShader.hlsl", "VS", L"PixelShader.hlsl", "PS");
	_ssao->RendererInit(L"SSAOVertexShader.hlsl", "ssaoVS", L"SSAOPixelShader.hlsl", "ssaoPS");
	_peraRenderer->RendererInit(L"TitlePeraVertexShader.hlsl", "VS", L"TitlePeraPixelShader.hlsl", "PS");


	_button.reset(new Button());
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_button->Create(_T("Title"), (int)(dx * 0.45), (int)(dy * 0.9), (int)(dx * 0.1), (int)(dy * 0.1), (HMENU)1);

	
	return true;
}

void TitleScene::SceneUpdate(void)
{
	if(_peraRenderer->Update())
	{}
	else {
		_rsm->Update();
		_modelRenderer->Update();
	}
}

void TitleScene::SceneRender(void)
{

	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();
	
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
	
	/*if (_button->IsClicked())
	{
		std::cout << "Clicked" << std::endl;
	}*/

	if (_peraRenderer->LinearWipe())
	{
		SceneFinal();
		_controller.ChangeScene(new GameScene(_controller));
	}
}

void TitleScene::SceneFinal(void)
{
	_button->Destroy();
	_renderer.reset();
	_pera.reset();
	_keyboard.reset();
	_models.clear();
	
}

void TitleScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
	//_modelRenderer->CreateDepthBuffer();
	//_ssao->CreateBuffers();
	//_rsm->ResizeBuffers();
	//_modelRenderer->ResizeBuffers();

}

const char* TitleScene::GetSceneName(void)
{
	return "TitleScene";
}


TitleScene::TitleScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

TitleScene::~TitleScene()
{
}
