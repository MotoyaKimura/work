#pragma once
#include "Scene.h"
#include <d3dx12.h>
#include <stack>

class Scene;
class SceneManager {
private:
	std::stack<std::shared_ptr<Scene>> _scene;
public:

	// シーン管理の初期化
	bool InitializeSceneManager(void);
	// シーン管理の解放
	void FinalizeSceneManager(void);
	// シーンの更新
	void UpdateSceneManager(void);
	// シーンの描画
	void RenderSceneManager(void);

	void ResizeSceneManager(void);

	// シーンの遷移
	void ChangeScene(Scene* scene);

	void PushScene(Scene* scene);

	void PopScene(void);

	// シーン名の取得
	const char* GetSceneName(void);
	SceneManager();
	~SceneManager();
};