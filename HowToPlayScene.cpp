#include "HowToPlayScene.h"
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

//遊び方シーンクラス
HowToPlayScene::HowToPlayScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

HowToPlayScene::~HowToPlayScene()
{
}

//シーンの初期化
bool HowToPlayScene::SceneInit()
{
	Application::SetIsShowHowToPlay(true);

	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	if (!TextureInit()) return false;
	if (!RendererBuffInit()) return false;
	if (!RendererDrawInit()) return false;
	ButtonInit();

	return true;
}

//シーンの更新
void HowToPlayScene::SceneUpdate(void)
{
	_peraRenderer->FadeIn();
	ButtonUpdate();
}

//シーンの描画
void HowToPlayScene::SceneRender(void)
{

	_peraRenderer->Draw();
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	//ボタンが押された
	if (_backButton->IsActive()) {
		_backButton->Hide();
		if (_peraRenderer->FadeOut())
		{
			SceneFinal();
			SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);
			_controller.PopScene();
			return;
		}
	}
}

//シーンの終了
void HowToPlayScene::SceneFinal(void)
{
	_backButton->Destroy();
}

//シーンのリサイズ
void HowToPlayScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* HowToPlayScene::GetSceneName(void)
{
	return "HowToPlayScene";
}

//ペラポリゴンの初期化
bool HowToPlayScene::PeraInit()
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
bool HowToPlayScene::CameraInit()
{
	_camera.reset(new Camera(Application::_dx, _pera));

	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("カメラの初期化エラー\n ");
		return false;
	}
	return true;
}

//テクスチャの初期化
bool HowToPlayScene::TextureInit()
{
	_howToPlayTex.reset(new Texture(Application::_dx, L"texture/asobikata.png"));
	if (!_howToPlayTex->Init())
	{
		Application::DebugOutputFormatString("テクスチャの初期化エラー\n ");
		return false;
	}
	_pera->SetSRV(_howToPlayTex->GetTexBuff(), _howToPlayTex->GetMetadata().format);
	return true;
}

//レンダラーのバッファー初期化
bool HowToPlayScene::RendererBuffInit()
{
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	if (!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRendererのバッファー初期化エラー\n ");
		return false;
	}
	return true;
}

//レンダラーの描画初期化
bool HowToPlayScene::RendererDrawInit()
{
	if(!_peraRenderer->RendererInit(
		L"HowToPlayPeraVertexShader.hlsl", "VS",
		L"HowToPlayPeraPixelShader.hlsl", "PS"
	))
	{
		Application::DebugOutputFormatString("PeraRendererのレンダラー初期化エラー\n ");
		return false;
	}
	return true;
}

//ボタンの初期化
void HowToPlayScene::ButtonInit()
{
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_backButton.reset(new Button("Back"));
	_backButton->Create(
		_T("←"),
		(int)(dx * 0.025f), (int)(dy * 0.1f),
		(int)(dx * 0.05), (int)(dy * 0.05),
		(HMENU)9
	);

}

//ボタンの更新
void HowToPlayScene::ButtonUpdate()
{
	//クリック判定
	_backButton->Update();

	
}
