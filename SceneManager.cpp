#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"


// �V�[���Ǘ��̏�����
bool SceneManager::InitializeSceneManager(void)
{
	_scene->SceneInit();
	return true;
}

// �V�[���Ǘ��̉��
void SceneManager::FinalizeSceneManager(void)
{
	_scene->SceneFinal();
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

void SceneManager::ResizeSceneManager(void)
{
	_scene->SceneResize();
}

// �V�[���̑J��
void SceneManager::ChangeScene(Scene* scene)
{
	_scene.reset(scene);
	InitializeSceneManager();
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