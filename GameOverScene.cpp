#include "GameOverScene.h"
#include "Application.h"
#include "GameScene.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Keyboard.h"
#include "PeraRenderer.h"
#include "Camera.h"
#include "Texture.h"
#include "Button.h"
#include <tchar.h>
#include "TitleScene.h"

GameOverScene::GameOverScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

GameOverScene::~GameOverScene()
{
}

//シーンの初期化
bool GameOverScene::SceneInit()
{
	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	if (!TextureInit()) return false;
	if (!RendererBuffInit()) return false;
	if (!RendererDrawInit()) return false;
	ButtonInit();
	return true;
}

//シーンの更新
void GameOverScene::SceneUpdate(void)
{
	//ポーズ判定と初めのフェードイン
	_peraRenderer->FadeIn();
	ButtonUpdate();
}

//シーンの描画
void GameOverScene::SceneRender(void)
{
	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
}

//シーンの終了
void GameOverScene::SceneFinal(void)
{
	Application::SetMenu();
	_restartButton->Destroy();
	_titleButton->Destroy();
}

//シーンのリサイズ
void GameOverScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* GameOverScene::GetSceneName(void)
{
	return "GameOverScene";
}

bool GameOverScene::PeraInit()
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

bool GameOverScene::CameraInit()
{
	//カメラの初期化
	_camera.reset(new Camera(Application::_dx, _pera));
	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("カメラの初期化エラー\n ");
		return false;
	}
}

bool GameOverScene::RendererBuffInit()
{
	//レンダラーのバッファー初期化
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	if (!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRendererのバッファー初期化エラー\n ");
		return false;
	}
	return true;
}

bool GameOverScene::TextureInit()
{
	//テクスチャの初期化
	_textures.resize(3);
	_textures[0].reset(new Texture(Application::_dx, L"texture/GameOver.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/restart.png"));
	_textures[2].reset(new Texture(Application::_dx, L"texture/BackToTitle.png"));
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

bool GameOverScene::RendererDrawInit()
{
	//各種レンダラークラスの描画部分の初期化
	if (!_peraRenderer->RendererInit(
		L"GameOverPeraVertexShader.hlsl", "VS",
		L"GameOverPeraPixelShader.hlsl", "PS"
	))
	{
		Application::DebugOutputFormatString("PeraRendererのレンダラー初期化エラー\n ");
		return false;
	}
	return true;
}

void GameOverScene::ButtonInit()
{
	//ボタンの初期化
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_restartButton.reset(new Button("Restart"));
	_restartButton->Create(
		_T("Restart"),
		(int)(dx * 0.45), (int)(dy * 0.4),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)5
	);
	_titleButton.reset(new Button("Title"));
	_titleButton->Create(
		_T("Back to Title"),
		(int)(dx * 0.45), (int)(dy * 0.6),
		(int)(dx * 0.1), (int)(dy * 0.1),
		(HMENU)6
	);
}

//ボタンの更新
void GameOverScene::ButtonUpdate()
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
	else {
		_peraRenderer->HoverCntReset();
		//リスタートボタンが押されたら
		if (_restartButton->IsActive())
		{
			_restartButton->Hide();
			_titleButton->Hide();
			if (_peraRenderer->FadeOut())
			{
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
			if (_peraRenderer->FadeOut())
			{
				SceneFinal();
				_controller.ChangeScene(new TitleScene(_controller));
				return;
			}
		}
	}
	
}


