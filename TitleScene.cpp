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
#include "AssimpModel.h"
#include "PmxModel.h"
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
	_models[0].reset(new PmxModel(Application::_dx, _camera, "modelData/nico/nico.pmx", 
		L"vmdData\\1.ぼんやり待ち_(490f_移動なし).vmd", true));
	_models[0]->Move(0, 0, 0);
	_models[1] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/wall/wall_green.obj");
	_models[1]->Move(2.5, 2.5, 0);
	_models[2] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/wall/wall_red.obj");
	_models[2]->Move(0, 2.5, 2.5);


	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));
	
	_keyboard->Init();

	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));

	_rsm->SetClearValue(0.5, 0.5, 0.5, 1.0);
	_modelRenderer->SetClearValue(0.5, 0.5, 0.5, 1.0);

	_rsm->Init();
	_modelRenderer->Init();
	_ssao->Init();
	_peraRenderer->Init();

	_texture.reset(new Texture(Application::_dx));
	_texture->Init(L"texture/start.png");
	_pera->SetSRV(_texture->GetTexBuff(), _texture->GetMetadata().format);

	for (auto model : _models)
	{
		if (!model->Init())
		{
			Application::DebugOutputFormatString("モデルの初期化エラー\n ");
			return false;
		}
	}

	for (auto model : _models)
	{
		model->RendererInit();
	}

	_rsm->RendererInit(L"VertexShader.hlsl", "rsmVS", L"PixelShader.hlsl", "rsmPS");
	_modelRenderer->RendererInit(L"VertexShader.hlsl", "VS", L"PixelShader.hlsl", "PS");
	_ssao->RendererInit(L"SSAOVertexShader.hlsl", "ssaoVS", L"SSAOPixelShader.hlsl", "ssaoPS");
	_peraRenderer->RendererInit(L"TitlePeraVertexShader.hlsl", "VS", L"TitlePeraPixelShader.hlsl", "PS");


	_StartButton.reset(new Button("Start"));
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_StartButton->Create(_T("Start"), (int)(dx * 0.45), (int)(dy * 0.9), (int)(dx * 0.1), (int)(dy * 0.1), (HMENU)1);

	return true;
}

void TitleScene::SceneUpdate(void)
{
	if (!_StartButton->IsActive())
	{
		if (_StartButton->IsHover())
		{
			_peraRenderer->HoverButton(_StartButton->GetName());
		}
		else
		{
			_peraRenderer->HoverCntReset();
		}
	}
	
	_StartButton->Update();
	_peraRenderer->Update();
	_rsm->Update(true);
	_modelRenderer->Update(true);
}

void TitleScene::SceneRender(void)
{

	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();
	
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	if (_StartButton->IsActive())
	{
	
		_StartButton->Hide();
		_peraRenderer->FadeOut();
		if (_peraRenderer->WipeEnd()) {
			_StartButton->SetInActive();
			SceneFinal();
			_controller.ChangeScene(new GameScene(_controller));
			return;
		}
	}
}

void TitleScene::SceneFinal(void)
{
	
	_StartButton->Destroy();

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
