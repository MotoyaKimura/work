#include "SceneManager.h"
#include <cassert>
#include "TitleScene.h"
#include "GameScene.h"
#include "ClearScene.h"

// �V�[���Ǘ��̏�����
bool SceneManager::InitializeSceneManager(void)
{
	_scene.top()->SceneInit();
	return true;
}

// �V�[���Ǘ��̉��
void SceneManager::FinalizeSceneManager(void)
{
	_scene.top()->SceneFinal();
}

// �V�[���̍X�V
void SceneManager::UpdateSceneManager(void)
{
	_scene.top()->SceneUpdate();
}

// �V�[���̕`��
void SceneManager::RenderSceneManager(void)
{
	_scene.top()->SceneRender();
}

void SceneManager::ResizeSceneManager(void)
{
	_scene.top()->SceneResize();
}

// �V�[���̑J��
void SceneManager::ChangeScene(Scene* scene)
{
	_scene.pop();
	_scene.emplace(scene);
	InitializeSceneManager();
}

// �V�[�����̎擾
const char* SceneManager::GetSceneName(void)
{
	return _scene.top()->GetSceneName();
}

//�V�[���̒ǉ�
void SceneManager::PushScene(Scene* scene)
{
	_scene.emplace(scene);
	InitializeSceneManager();
}

//�V�[���̍폜
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