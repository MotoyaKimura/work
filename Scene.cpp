#include "Scene.h"

// ������
bool Scene::SceneInit(void)
{

	return true;
}

// ���
void Scene::SceneFinal(void)
{
}

// �X�V
void Scene::SceneUpdate(void)
{
}

// �`��
void Scene::SceneRender(void)
{
}

// �V�[�������̐ݒ�
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