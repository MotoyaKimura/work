#include "TitleScene.h"
#include "Application.h"
#include "GameScene.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Renderer.h"
#include "Keyboard.h"
#include "RSM.h"
#include "ModelRenderer.h"
#include "SSAO.h"
#include "PeraRenderer.h"

void TitleScene::FadeoutUpdate()
{
	_controller.ChangeScene(new GameScene(_controller));
}

bool TitleScene::SceneInit()
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_keyboard.reset(new Keyboard(Application::hwnd));

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
	}

	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models));
	_rsm->Init();
	_modelRenderer->Init();
	_ssao->Init();
	_peraRenderer->Init();

	_modelRenderer->SetRTsToHeapAsSRV(_pera->GetHeap(), 0);
	_rsm->SetRTsToHeapAsSRV(_pera->GetHeap(), 3);
	_ssao->SetRTsToHeapAsSRV(_pera->GetHeap(), 7);
	Application::_dx->SetCBVToHeap(_pera->GetHeap(), 8);
	
	return true;
}

void TitleScene::SceneUpdate(void)
{
	_rsm->Move();
	_modelRenderer->Move();
	_rsm->Update();
	_modelRenderer->Update();
}

void TitleScene::SceneRender(void)
{
	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();
	
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
}

void TitleScene::SceneFinal(void)
{
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
