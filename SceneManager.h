#pragma once
#include <Windows.h>


class Scene;

// �������p�֐�
using InitFunc = bool (*)(void);
// ����p�֐�
using FinalFunc = void (*)(void);
// �X�V�p�֐�
using UpdateFunc = void (*)(void);
// �`��p�֐�
using RenderFunc = void (*)(void);
// �V�[���J�ڗp�̊֐��^

// �V�[�������ݒ�p�\����
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


	// ���݂̃V�[�����ێ��p
	SceneProc g_currentScene;
	// �V�[���Ǘ��̏�����
	void InitializeSceneManager(void);
	// �V�[���Ǘ��̉��
	void FinalizeSceneManager(void);
	// �V�[���̍X�V
	void UpdateSceneManager(void);
	// �V�[���̕`��
	void RenderSceneManager(void);

	// �V�[���̑J��
	bool JumpScene(SetupFunc func);
	// �V�[�����̎擾
	const char* GetSceneName(void);
	SceneManager();
	~SceneManager();
};