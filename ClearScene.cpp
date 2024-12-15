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

//クリアシーンクラス
ClearScene::ClearScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

ClearScene::~ClearScene()
{
}

//シーンの初期化
bool ClearScene::SceneInit()
{
	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	ModelReset();
	KeyboardInit();
	if (!RendererBuffInit()) return false;
	if (!TextureInit()) return false;
	if (!ModelInit()) return false;
	if (!RendererDrawInit()) return false;
	ButtonInit();
	return true;
}

//シーンの更新
void ClearScene::SceneUpdate(void)
{
	//カメラの自動回転
	_keyboard->AutoRotateCamera();
	//カメラの更新
	_camera->CalcSceneTrans();
	//クレジットを流す
	_peraRenderer->creditRoll();
	//フェードイン白
	_peraRenderer->GameOverFadeOut();
	//ポーズ判定と初めのフェードイン
	_peraRenderer->FadeIn();
	//モデルの動き更新
	_rsm->Update(false);
	_modelRenderer->Update(false);
	//ボタンの更新
	ButtonUpdate();
}

//シーンの描画
void ClearScene::SceneRender(void)
{
	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	//リスタートボタンが押されたら
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

	//タイトルボタンが押されたら
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

//シーンの終了
void ClearScene::SceneFinal(void)
{
	_restartButton->Destroy();
	_titleButton->Destroy();
}

//シーンのリサイズ
void ClearScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* ClearScene::GetSceneName(void)
{
	return "ClearScene";
}

bool ClearScene::PeraInit()
{
	//ペラポリゴンの初期化
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}
	return true;
}

bool ClearScene::CameraInit()
{
	//カメラの初期化
	_camera.reset(new Camera(Application::_dx, _pera));
	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("カメラの初期化エラー\n ");
		return false;
	}
	_camera->SetEyePos(DirectX::XMFLOAT3(0, -10, -40));
	return true;
}

bool ClearScene::TextureInit()
{
	_textures.resize(5);
	_textures[0].reset(new Texture(Application::_dx, L"texture/clear.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/restart.png"));
	_textures[2].reset(new Texture(Application::_dx, L"texture/BackToTitle.png"));
	_textures[3].reset(new Texture(Application::_dx, L"texture/credit1.png"));
	_textures[4].reset(new Texture(Application::_dx, L"texture/credit2.png"));
	for (auto tex : _textures)
	{
		if (!tex->Init())
		{
			Application::DebugOutputFormatString("テクスチャの初期化エラー\n ");
			return false;
		}
		_pera->SetSRV(tex->GetTexBuff(), tex->GetMetadata().format);
	}
	return true;
}

bool ClearScene::ModelReset()
{
	modelNum = 8;
	_models.resize(modelNum);
	_models[0].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/floor/floor_circle.obj"));
	if (!_models[0]->Load()) return false;
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
		if (!_models[i + 1]->Load()) return false;
		_models[1 + i]->Move(3, -10.5, 10);
		_models[1 + i]->Rotate(0, 0.5, 0);
	}
	_models[7] = std::make_shared<PmxModel>(Application::_dx, _camera, "modelData/八重沢なとり/YaezawaNatori.pmx",
		L"vmdData\\腕上げ.vmd", false);
	if (!_models[7]->Load()) return false;
	_models[7]->Move(-15, -10.5, -10);
	_models[7]->Rotate(0, 0.5, 0);


	return true;
}

void ClearScene::KeyboardInit()
{
	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));
	_keyboard->Init();
}

bool ClearScene::RendererBuffInit()
{
	//レンダラーのバッファー初期化
	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_rsm->SetClearValue(0.8f, 0.8f, 1.0f, 1.0f);
	if (!_rsm->Init())
	{
		Application::DebugOutputFormatString("RSMのバッファー初期化エラー\n ");
		return false;
	}
	_modelRenderer->SetClearValue(0.8f, 0.8f, 1.0f, 1.0f);
	if (!_modelRenderer->Init())
	{
		Application::DebugOutputFormatString("モデルレンダラーのバッファー初期化エラー\n ");
		return false;
	}
	if (!_ssao->Init())
	{
		Application::DebugOutputFormatString("SSAOのバッファー初期化エラー\n ");
		return false;
	}
	if (!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRendererのバッファー初期化エラー\n ");
		return false;
	}
	return true;
}

bool ClearScene::RendererDrawInit()
{
	//各種レンダラークラスの描画部分の初期化
	if (!_rsm->RendererInit(L"VertexShader.hlsl", "rsmVS", L"PixelShader.hlsl", "rsmPS"))
	{
		Application::DebugOutputFormatString("RSMのレンダラー初期化エラー\n ");
		return false;
	}
	if (!_modelRenderer->RendererInit(L"VertexShader.hlsl", "VS", L"PixelShader.hlsl", "PS"))
	{
		Application::DebugOutputFormatString("モデルレンダラーのレンダラー初期化エラー\n ");
		return false;
	}
	if (!_ssao->RendererInit(L"SSAOVertexShader.hlsl", "ssaoVS", L"SSAOPixelShader.hlsl", "ssaoPS"))
	{
		Application::DebugOutputFormatString("SSAOのレンダラー初期化エラー\n ");
		return false;
	}
	if (!_peraRenderer->RendererInit(L"ClearPeraVertexShader.hlsl", "VS", L"ClearPeraPixelShader.hlsl", "PS"))
	{
		Application::DebugOutputFormatString("PeraRendererのレンダラー初期化エラー\n ");
		return false;
	}
	return true;
}

bool ClearScene::ModelInit()
{
	for (auto model : _models)
	{
		if (!model->Init())
		{
			Application::DebugOutputFormatString("モデルの初期化エラー\n ");
			return false;
		}
	}
	return true;
}

void ClearScene::ButtonInit()
{
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_restartButton.reset(new Button("Restart"));
	_restartButton->Create(_T("Restart"), (int)(dx * 0.2), (int)(dy * 0.8), (int)(dx * 0.2), (int)(dy * 0.1), (HMENU)7);
	_titleButton.reset(new Button("Title"));
	_titleButton->Create(_T("Back to Title"), (int)(dx * 0.6), (int)(dy * 0.8), (int)(dx * 0.2), (int)(dy * 0.1), (HMENU)8);
}

void ClearScene::ButtonUpdate()
{
	//クリック判定
	_restartButton->Update();
	_titleButton->Update();
	//ボタンが押されていない
	//ホバー時にボタンがフェードイン・アウトする
	if (!_restartButton->IsActive() && !_titleButton->IsActive())
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
	//押された
	else
	{
		_peraRenderer->HoverCntReset();
	}
}