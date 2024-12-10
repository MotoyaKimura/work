#include "ClearScene.h"
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
#include <tchar.h>

#include "TitleScene.h"

bool ClearScene::SceneInit()
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


	modelNum = 3;
	_models.resize(modelNum);
	_models[0].reset(new AssimpModel(Application::_dx, _camera, "modelData/bunny/bunny.obj"));
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
	_rsm->Init();
	_modelRenderer->Init();
	_ssao->Init();
	_peraRenderer->Init();

	_clearTex.reset(new Texture(Application::_dx));
	_clearTex->Init(L"texture/clear.png");
	_pera->SetSRV(_clearTex->GetTexBuff(), _clearTex->GetMetadata().format);
	_restartTex.reset(new Texture(Application::_dx));
	_restartTex->Init(L"texture/restart.png");
	_pera->SetSRV(_restartTex->GetTexBuff(), _restartTex->GetMetadata().format);
	_titleTex.reset(new Texture(Application::_dx));
	_titleTex->Init(L"texture/BackToTitle.png");
	_pera->SetSRV(_titleTex->GetTexBuff(), _titleTex->GetMetadata().format);

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
	_peraRenderer->RendererInit(L"ClearPeraVertexShader.hlsl", "VS", L"ClearPeraPixelShader.hlsl", "PS");

	_rsm->SetClearValue(0.8f, 0.8f, 1.0f, 1.0f);
	_modelRenderer->SetClearValue(0.8f, 0.8f, 0.9f, 1.0f);

	_restartButton.reset(new Button("Restart"));
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_restartButton->Create(_T("Restart"), (int)(dx * 0.2), (int)(dy * 0.8), (int)(dx * 0.2), (int)(dy * 0.1), (HMENU)7);

	_titleButton.reset(new Button("Title"));
	_titleButton->Create(_T("Back to Title"), (int)(dx * 0.6), (int)(dy * 0.8), (int)(dx * 0.2), (int)(dy * 0.1), (HMENU)8);

	return true;
}

void ClearScene::SceneUpdate(void)
{
	if(!_restartButton->IsActive())
	{
		if (_restartButton->IsHover())
		{
			_peraRenderer->HoverButton(_restartButton->GetName());
		}
	}
	if (!_titleButton->IsActive())
	{
		if (_titleButton->IsHover())
		{
			_peraRenderer->HoverButton(_titleButton->GetName());
		}
	}
	else
	{
		_peraRenderer->HoverCntReset();
	}

	_keyboard->AutoRotateCamera();
	_camera->CalcSceneTrans();

	_restartButton->Update();
	_titleButton->Update();
	if (_peraRenderer->Update())
	{
	}
	else {
			_rsm->Update(false);
			_modelRenderer->Update(false);
	}
}

void ClearScene::SceneRender(void)
{

	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();

	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	if (_restartButton->IsActive())
	{
		_restartButton->Hide();
		_titleButton->Hide();

		if (_peraRenderer->FadeOut()) {
			SceneFinal();
			_controller.ChangeScene(new GameScene(_controller));
			return;
		}
	}

	if (_titleButton->IsActive())
	{
		_restartButton->Hide();
		_titleButton->Hide();

		if (_peraRenderer->FadeOut()) {
			SceneFinal();
			_controller.ChangeScene(new TitleScene(_controller));
			return;
		}
	}
}

void ClearScene::SceneFinal(void)
{
	
	_restartButton->Destroy();
	
	_titleButton->Destroy();

}

void ClearScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
	//_modelRenderer->CreateDepthBuffer();
	//_ssao->CreateBuffers();
	//_rsm->ResizeBuffers();
	//_modelRenderer->ResizeBuffers();

}

const char* ClearScene::GetSceneName(void)
{
	return "ClearScene";
}


ClearScene::ClearScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

ClearScene::~ClearScene()
{
}
