#pragma once
#include <Windows.h>


class Scene;

// 初期化用関数
using InitFunc = bool (*)(void);
// 解放用関数
using FinalFunc = void (*)(void);
// 更新用関数
using UpdateFunc = void (*)(void);
// 描画用関数
using RenderFunc = void (*)(void);
// シーン遷移用の関数型

// シーン処理設定用構造体
struct SceneProc
{
	const char* Name;
	InitFunc		Init;
	FinalFunc		Final;
	UpdateFunc		Update;
	RenderFunc		Render;
};


class SceneManager {
private:
	using SetupFunc = SceneProc(*)(void);
public:


	// 現在のシーン情報保持用
	SceneProc g_currentScene;
	// シーン管理の初期化
	void InitializeSceneManager(void);
	// シーン管理の解放
	void FinalizeSceneManager(void);
	// シーンの更新
	void UpdateSceneManager(void);
	// シーンの描画
	void RenderSceneManager(void);

	// シーンの遷移
	bool JumpScene(SetupFunc func);
	// シーン名の取得
	const char* GetSceneName(void);
	SceneManager();
	~SceneManager();
};