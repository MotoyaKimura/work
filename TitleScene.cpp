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
#include "PmxModel.h"
#include <tchar.h>

//タイトルシーンクラス
TitleScene::TitleScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

TitleScene::~TitleScene()
{
}

//シーンの初期化
bool TitleScene::SceneInit()
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
void TitleScene::SceneUpdate(void)
{
	_StartButton->Update();
	_peraRenderer->FadeIn();
	_rsm->Update(true);
	_modelRenderer->Update(true);
	ButtonUpdate();
}

//シーンの描画
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
		//_peraRenderer->HoverCntReset();
		_StartButton->Hide();
		_peraRenderer->FadeOut();
		if (_peraRenderer->IsWipeClose()) {
			_StartButton->SetInActive();
			SceneFinal();
			_controller.ChangeScene(new GameScene(_controller));
			return;
		}
	}
}

//シーンの終了
void TitleScene::SceneFinal(void)
{
	_StartButton->Destroy();
}

//シーンのリサイズ
void TitleScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* TitleScene::GetSceneName(void)
{
	return "TitleScene";
}

//ペラポリゴンの初期化
bool TitleScene::PeraInit()
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}
	return true;
}

//カメラの初期化
bool TitleScene::CameraInit()
{
	_camera.reset(new Camera(Application::_dx, _pera));

	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("カメラの初期化エラー\n ");
		return false;
	}
	return true;
}

//モデルのリセット
bool TitleScene::ModelReset()
{
	modelNum = 1;
	_models.resize(modelNum);
	_models[0].reset(new PmxModel(Application::_dx, _camera, "modelData/八重沢なとり/YaezawaNatori.pmx",
		L"vmdData\\a.vmd", false));
	if (!_models[0]->Load()) return false;
	_models[0]->Move(0, 0, 0);


	return true;
}

//キーボードの初期化
void TitleScene::KeyboardInit()
{
	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));
	_keyboard->Init();
}

//レンダラーのバッファー初期化
bool TitleScene::RendererBuffInit()
{
	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));

	_rsm->SetClearValue(0.5, 0.5, 0.5, 1.0);
	_modelRenderer->SetClearValue(0.5, 0.5, 0.5, 1.0);

	if (!_rsm->Init())
	{
		Application::DebugOutputFormatString("RSMのバッファー初期化エラー\n ");
		return false;
	}
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

//テクスチャの初期化
bool TitleScene::TextureInit()
{
	_textures.resize(2);
	_textures[0].reset(new Texture(Application::_dx, L"texture/start.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/yabai.png"));
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

//モデルの初期化
bool TitleScene::ModelInit()
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

//レンダラーの描画部分の初期化
bool TitleScene::RendererDrawInit()
{
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
	if (!_peraRenderer->RendererInit(L"TitlePeraVertexShader.hlsl", "VS", L"TitlePeraPixelShader.hlsl", "PS"))
	{
		Application::DebugOutputFormatString("PeraRendererのレンダラー初期化エラー\n ");
		return false;
	}
	return true;
}

//ボタンの初期化
void TitleScene::ButtonInit()
{
	_StartButton.reset(new Button("Start"));
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_StartButton->Create(_T("Start"), (int)(dx * 0.45), (int)(dy * 0.9), (int)(dx * 0.1), (int)(dy * 0.1), (HMENU)1);
}

//ボタンの更新
void TitleScene::ButtonUpdate()
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
	
}
