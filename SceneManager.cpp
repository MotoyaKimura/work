#include "SceneManager.h"




// シーン管理の初期化
void SceneManager::InitializeSceneManager(void)
{
	ZeroMemory(&g_currentScene, sizeof(g_currentScene));
}

// シーン管理の解放
void SceneManager::FinalizeSceneManager(void)
{
	// 最後は解放して終わる
	if (g_currentScene.Final) g_currentScene.Final();

	ZeroMemory(&g_currentScene, sizeof(g_currentScene));
}

// シーンの更新
void SceneManager::UpdateSceneManager(void)
{
	if (g_currentScene.Update) g_currentScene.Update();
}

// シーンの描画
void SceneManager::RenderSceneManager(void)
{
	if (g_currentScene.Render) g_currentScene.Render();
}

// シーンの遷移
bool SceneManager::JumpScene(SetupFunc func)
{
	// 現在のシーンを解放する
	if (g_currentScene.Final) g_currentScene.Final();

	ZeroMemory(&g_currentScene, sizeof(g_currentScene));

	bool ret = true;
	// 次のシーンがあれば初期化する
	if (func)
	{
		g_currentScene = func();
		if (g_currentScene.Init) ret = g_currentScene.Init();
	}
	return ret;
}

// シーン名の取得
const char* SceneManager::GetSceneName(void)
{
	if (g_currentScene.Name) return g_currentScene.Name;
	return "Unknown";
}

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}