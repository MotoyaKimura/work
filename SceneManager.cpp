#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"


// �V�[���Ǘ��̏�����
void SceneManager::InitializeSceneManager(void)
{
}

// �V�[���Ǘ��̉��
void SceneManager::FinalizeSceneManager(void)
{
}

// �V�[���̍X�V
void SceneManager::UpdateSceneManager(void)
{
	_scene->SceneUpdate();
}

// �V�[���̕`��
void SceneManager::RenderSceneManager(void)
{
	_scene->SceneRender();
}

// �V�[���̑J��
void SceneManager::ChangeScene(Scene* scene)
{
	_scene.reset(scene);
}

// �V�[�����̎擾
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