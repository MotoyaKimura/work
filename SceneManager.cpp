#include "SceneManager.h"




// �V�[���Ǘ��̏�����
void SceneManager::InitializeSceneManager(void)
{
	ZeroMemory(&g_currentScene, sizeof(g_currentScene));
}

// �V�[���Ǘ��̉��
void SceneManager::FinalizeSceneManager(void)
{
	// �Ō�͉�����ďI���
	if (g_currentScene.Final) g_currentScene.Final();

	ZeroMemory(&g_currentScene, sizeof(g_currentScene));
}

// �V�[���̍X�V
void SceneManager::UpdateSceneManager(void)
{
	if (g_currentScene.Update) g_currentScene.Update();
}

// �V�[���̕`��
void SceneManager::RenderSceneManager(void)
{
	if (g_currentScene.Render) g_currentScene.Render();
}

// �V�[���̑J��
bool SceneManager::JumpScene(SetupFunc func)
{
	// ���݂̃V�[�����������
	if (g_currentScene.Final) g_currentScene.Final();

	ZeroMemory(&g_currentScene, sizeof(g_currentScene));

	bool ret = true;
	// ���̃V�[��������Ώ���������
	if (func)
	{
		g_currentScene = func();
		if (g_currentScene.Init) ret = g_currentScene.Init();
	}
	return ret;
}

// �V�[�����̎擾
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