#include "Application.h"
#include "GameScene.h"
#include <tchar.h>
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
#include  "Button.h"
#include "MenuScene.h"
#include "AssimpModel.h"
#include "PmxModel.h"
#include  "GameOverScene.h"
#include "ClearScene.h"
#include "HowToPlayScene.h"

bool GameScene::SceneInit()
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
	modelNum = 21;
	_models.resize(modelNum);
	_models[0].reset(new PmxModel(Application::_dx, _camera, "modelData/MiraikomachiPMX-master/Miraikomachi.pmx",
		L"vmdData\\1.ぼんやり待ち_(490f_移動なし).vmd", true));
	_models[0]->Move(0, 0.2, 0);
	
	_models[1] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/floor/floor_yellow.obj");
	_models[2] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/piller/piller.obj");
	_models[2]->Move(9, 0, 9);
	_models[3] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/piller/piller.obj");
	_models[3]->Move(-9, 0, 9);
	for (int i = 0; i < 10; i++)
	{
		_models[i + 4] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/floor/floor_white.obj");
		_models[i + 4]->Move(0, 2.5 * (i + 1), 5 * (i + 1));
	}
	_models[14] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/floor/floor_lightBlue.obj");
	_models[14]->Move(0, 2.5 * 11, 5 * 15);

	//家モデル（一遍にできないので１つずつ、モデル数：６）
	_models[15] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/cafe.obj");
	_models[16] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/chimney.obj");
	_models[17] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/roof.obj");
	_models[18] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/knob.obj");
	_models[19] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/door.obj");
	_models[20] = std::make_shared<AssimpModel>(Application::_dx, _camera, "modelData/RSMScene/house/base.obj");
	for (int i = 0; i < 6; i++)
	{
		_models[15 + i]->Move(0, 2.5 * 11 + 0.5, 5 * 18);
	}



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

	_pauseTex.reset(new Texture(Application::_dx));
	_pauseTex->Init(L"texture/pause.png");
	_pera->SetSRV(_pauseTex->GetTexBuff(), _pauseTex->GetMetadata().format);

	_zeroTex.reset(new Texture(Application::_dx));
	_zeroTex->Init(L"texture/zero.png");
	_pera->SetSRV(_zeroTex->GetTexBuff(), _zeroTex->GetMetadata().format);
	_oneTex.reset(new Texture(Application::_dx));
	_oneTex->Init(L"texture/one.png");
	_pera->SetSRV(_oneTex->GetTexBuff(), _oneTex->GetMetadata().format);
	_twoTex.reset(new Texture(Application::_dx));
	_twoTex->Init(L"texture/two.png");
	_pera->SetSRV(_twoTex->GetTexBuff(), _twoTex->GetMetadata().format);
	_threeTex.reset(new Texture(Application::_dx));
	_threeTex->Init(L"texture/three.png");
	_pera->SetSRV(_threeTex->GetTexBuff(), _threeTex->GetMetadata().format);
	_fourTex.reset(new Texture(Application::_dx));
	_fourTex->Init(L"texture/four.png");
	_pera->SetSRV(_fourTex->GetTexBuff(), _fourTex->GetMetadata().format);
	_fiveTex.reset(new Texture(Application::_dx));
	_fiveTex->Init(L"texture/five.png");
	_pera->SetSRV(_fiveTex->GetTexBuff(), _fiveTex->GetMetadata().format);
	_sixTex.reset(new Texture(Application::_dx));
	_sixTex->Init(L"texture/six.png");
	_pera->SetSRV(_sixTex->GetTexBuff(), _sixTex->GetMetadata().format);
	_sevenTex.reset(new Texture(Application::_dx));
	_sevenTex->Init(L"texture/seven.png");
	_pera->SetSRV(_sevenTex->GetTexBuff(), _sevenTex->GetMetadata().format);
	_eightTex.reset(new Texture(Application::_dx));
	_eightTex->Init(L"texture/eight.png");
	_pera->SetSRV(_eightTex->GetTexBuff(), _eightTex->GetMetadata().format);
	_nineTex.reset(new Texture(Application::_dx));
	_nineTex->Init(L"texture/nine.png");
	_pera->SetSRV(_nineTex->GetTexBuff(), _nineTex->GetMetadata().format);
	_timerTex.reset(new Texture(Application::_dx));
	_timerTex->Init(L"texture/timer.png");
	_pera->SetSRV(_timerTex->GetTexBuff(), _timerTex->GetMetadata().format);
	_colonTex.reset(new Texture(Application::_dx));
	_colonTex->Init(L"texture/colon.png");
	_pera->SetSRV(_colonTex->GetTexBuff(), _colonTex->GetMetadata().format);

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
	_peraRenderer->RendererInit(L"PeraVertexShader.hlsl", "VS", L"PeraPixelShader.hlsl", "PS");

	
	return true;
}

void GameScene::SceneUpdate(void)
{
	_peraRenderer->MonochromeToColor();
	if (_peraRenderer->Update() || _peraRenderer->WipeStart())
	{}
	else {
		isStart = true;
		_keyboard->Move();
		_camera->CalcSceneTrans();
	}
	if(!_peraRenderer->IsPause())
	{
		_rsm->Update(isStart);
		_modelRenderer->Update(isStart);
	}
	
	if(isBackFromHowToPlay)
	{
		isBackFromHowToPlay = false;
		_peraRenderer->TimeStart();
	}
	if(isMenu)
	{
		isMenu = false;
		_peraRenderer->TimeStop();
	}
	if (isStart)
	{
		_peraRenderer->CalcTime();
	}
	else
	{
		_peraRenderer->TimeStart();
	}
}

void GameScene::SceneRender(void)
{

	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();

	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	if(isStart && !Application::GetIsShowHowToPlay())
	{
		_keyboard->SetIsHowToPlay(true);
		isBackFromHowToPlay = true;
		_controller.PushScene(new HowToPlayScene(_controller));
		return;
	}

	
	if(_camera->GetEyePos()->y <= -100 && !isClear)
	{
		if (_peraRenderer->GameOverFadeOut())
		{
			_controller.ChangeScene(new GameOverScene(_controller));
			return;
		}
	}

	if (_peraRenderer->TimeLimit() && !isClear)
	{
		if (_peraRenderer->GameOverFadeOut())
		{
			_controller.ChangeScene(new GameOverScene(_controller));
			return;
		}
		return;
	}

	if (_camera->GetEyePos()->x > -15 && _camera->GetEyePos()->x < 15 &&
		_camera->GetEyePos()->y >= 27.5 &&
		_camera->GetEyePos()->z >= 5 * 14 - 10 || isClear)
	{
		isClear = true;
		if (_peraRenderer->ClearFadeOut())
		{
			_controller.ChangeScene(new ClearScene(_controller));
			return;
		}
		return;
	}

	
	_keyboard->SetIsPause(Application::GetPause());
	

	if(Application::GetMenu())
	{
		if (_peraRenderer->FadeOut())
		{
			isMenu = true;
			_keyboard->SetIsMenu(true);
			//_peraRenderer->DataReset();
			_controller.PushScene(new MenuScene(_controller));
			return;
		}
	}

}

void GameScene::SceneFinal(void)
{
	_renderer.reset();
	_pera.reset();
	_keyboard.reset();
	_models.clear();
}

void GameScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
	//_modelRenderer->CreateDepthBuffer();
	//_ssao->CreateBuffers();
	//_rsm->ResizeBuffers();
	//_modelRenderer->ResizeBuffers();

}

const char* GameScene::GetSceneName(void)
{
	return "GameScene";
}


GameScene::GameScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

GameScene::~GameScene()
{
}
