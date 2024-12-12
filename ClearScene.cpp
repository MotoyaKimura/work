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
#include "PmxModel.h"
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

	_camera->SetEyePos(DirectX::XMFLOAT3(0, 0, -40));

	modelNum = 8;
	_models.resize(modelNum);
	_models[0].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/floor/floor_circle.obj"));
	_models[0]->Move(0, -11.5, 0);
	_models[0]->Rotate(0, 0.5, 0);
	_models[1] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/cafe.obj");
	_models[2] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/chimney.obj");
	_models[3] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/roof.obj");
	_models[4] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/knob.obj");
	_models[5] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/door.obj");
	_models[6] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/base.obj");
	for (int i = 0; i < 6; i++)
	{
		_models[1 + i]->Move(3, -10.5, 10);
		_models[1 + i]->Rotate(0, 0.5, 0);
	}

	_models[7] = std::make_shared<PmxModel>(Application::_dx, _camera, "modelData/MiraikomachiPMX-master/Miraikomachi.pmx", 
		L"vmdData\\jump.vmd", false);
	_models[7]->Move(-15, -10.5, -10);
	_models[7]->Rotate(0, 0.5, 0);
	
	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));

	_keyboard->Init();

	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));

	_rsm->SetClearValue(0.8f, 0.8f, 1.0f, 1.0f);
	_modelRenderer->SetClearValue(0.8f, 0.8f, 0.9f, 1.0f);

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
	_creditTex1.reset(new Texture(Application::_dx));
	_creditTex1->Init(L"texture/credit1.png");
	_pera->SetSRV(_creditTex1->GetTexBuff(), _creditTex1->GetMetadata().format);
	_creditTex2.reset(new Texture(Application::_dx));
	_creditTex2->Init(L"texture/credit2.png");
	_pera->SetSRV(_creditTex2->GetTexBuff(), _creditTex2->GetMetadata().format);


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
	if(!_restartButton->IsActive() && !_titleButton->IsActive())
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
	}
	_peraRenderer->creditRoll();
	
	_keyboard->AutoRotateCamera();
	_camera->CalcSceneTrans();

	_restartButton->Update();
	_titleButton->Update();
	
	_peraRenderer->GameOverFadeOut();
	_peraRenderer->Update();
	_rsm->Update(false);
	_modelRenderer->Update(false);
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
