#include "SceneManager.h"
#include <cassert>
#include "TitleScene.h"
#include "GameScene.h"
#include "ClearScene.h"

// シーン管理の初期化
bool SceneManager::InitializeSceneManager(void)
{
	_scene.top()->SceneInit();
	return true;
}

// シーン管理の解放
void SceneManager::FinalizeSceneManager(void)
{
	_scene.top()->SceneFinal();
}

// シーンの更新
void SceneManager::UpdateSceneManager(void)
{
	_scene.top()->SceneUpdate();
}

// シーンの描画
void SceneManager::RenderSceneManager(void)
{
	_scene.top()->SceneRender();
}

void SceneManager::ResizeSceneManager(void)
{
	_scene.top()->SceneResize();
}

// シーンの遷移
void SceneManager::ChangeScene(Scene* scene)
{
	_scene.pop();
	_scene.emplace(scene);
	InitializeSceneManager();
}

// シーン名の取得
const char* SceneManager::GetSceneName(void)
{
	return _scene.top()->GetSceneName();
}

//シーンの追加
void SceneManager::PushScene(Scene* scene)
{
	_scene.emplace(scene);
	InitializeSceneManager();
}

//シーンの削除
void SceneManager::PopScene(void)
{
	_scene.pop();
	assert(!_scene.empty());
}

SceneManager::SceneManager()
{
	//_scene.emplace(new ClearScene(*this));
	_scene.emplace(new GameScene(*this));
}

SceneManager::~SceneManager()
{
}