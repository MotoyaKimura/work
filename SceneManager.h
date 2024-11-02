#pragma once
#include "Scene.h"
#include <d3dx12.h>

class Scene;
class SceneManager {
private:
	std::shared_ptr<Scene> _scene;
public:

	// �V�[���Ǘ��̏�����
	void InitializeSceneManager(void);
	// �V�[���Ǘ��̉��
	void FinalizeSceneManager(void);
	// �V�[���̍X�V
	void UpdateSceneManager(void);
	// �V�[���̕`��
	void RenderSceneManager(void);

	// �V�[���̑J��
	void ChangeScene(Scene* scene);

	// �V�[�����̎擾
	const char* GetSceneName(void);
	SceneManager();
	~SceneManager();
};