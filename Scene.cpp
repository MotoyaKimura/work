#include "Scene.h"

// 初期化
bool Scene::SceneInit(void)
{

	return true;
}

// 解放
void Scene::SceneFinal(void)
{
}

// 更新
void Scene::SceneUpdate(void)
{
}

// 描画
void Scene::SceneRender(void)
{
}

// シーン処理の設定
SceneProc Scene::SetupTestScene(void)
{
	SceneProc proc =
	{
		"Test",
		SceneInit,
		SceneFinal,
		SceneUpdate,
		SceneRender,
	};
	return proc;
}

Scene::Scene()
{
}

Scene::~Scene()
{
}