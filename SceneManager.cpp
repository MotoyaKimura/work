#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"


// シーン管理の初期化
bool SceneManager::InitializeSceneManager(void)
{
	_scene->SceneInit();
	return true;
}

// シーン管理の解放
void SceneManager::FinalizeSceneManager(void)
{
	_scene->SceneFinal();
}

// シーンの更新
void SceneManager::UpdateSceneManager(void)
{
	_scene->SceneUpdate();
}

// シーンの描画
void SceneManager::RenderSceneManager(void)
{
	_scene->SceneRender();
}

void SceneManager::ResizeSceneManager(void)
{
	_scene->SceneResize();
}

// シーンの遷移
void SceneManager::ChangeScene(Scene* scene)
{
	_scene.reset(scene);
	InitializeSceneManager();
}

// シーン名の取得
const char* SceneManager::GetSceneName(void)
{
	return _scene->GetSceneName();
}

SceneManager::SceneManager()
{
	_scene.reset(new TitleScene(*this));
}

SceneManager::~SceneManager()
{
}