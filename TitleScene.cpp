#include "TitleScene.h"
#include "Application.h"
#include "GameScene.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Keyboard.h"
#include "RSM.h"
#include "ModelRenderer.h"
#include "SSAO.h"
#include "PeraRenderer.h"
#include "Camera.h"
#include "Texture.h"
#include "Button.h"
#include "PmxModel.h"
#include <tchar.h>

//�^�C�g���V�[���N���X
TitleScene::TitleScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

TitleScene::~TitleScene()
{
}

//�V�[���̏�����
bool TitleScene::SceneInit()
{
	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	ModelReset();
	KeyboardInit();
	if (!RendererBuffInit()) return false;
	if (!TextureInit()) return false;
	if (!ModelInit()) return false;
	if (!RendererDrawInit()) return false;
	ButtonInit();
	return true;
}

//�V�[���̍X�V
void TitleScene::SceneUpdate(void)
{
	_StartButton->Update();
	_peraRenderer->FadeIn();
	_rsm->Update(true);
	_modelRenderer->Update(true);
	ButtonUpdate();
}

//�V�[���̕`��
void TitleScene::SceneRender(void)
{
	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();
	
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

	if (_StartButton->IsActive())
	{
		//_peraRenderer->HoverCntReset();
		_StartButton->Hide();
		_peraRenderer->FadeOut();
		if (_peraRenderer->IsWipeClose()) {
			_StartButton->SetInActive();
			SceneFinal();
			_controller.ChangeScene(new GameScene(_controller));
			return;
		}
	}
}

//�V�[���̏I��
void TitleScene::SceneFinal(void)
{
	_StartButton->Destroy();
}

//�V�[���̃��T�C�Y
void TitleScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* TitleScene::GetSceneName(void)
{
	return "TitleScene";
}

//�y���|���S���̏�����
bool TitleScene::PeraInit()
{
	_pera.reset(new Pera(Application::_dx));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("�y���|���S���̏������G���[\n ");
		return false;
	}
	return true;
}

//�J�����̏�����
bool TitleScene::CameraInit()
{
	_camera.reset(new Camera(Application::_dx, _pera));

	if (!_camera->Init())
	{
		Application::DebugOutputFormatString("�J�����̏������G���[\n ");
		return false;
	}
	return true;
}

//���f���̃��Z�b�g
bool TitleScene::ModelReset()
{
	modelNum = 1;
	_models.resize(modelNum);
	_models[0].reset(new PmxModel(Application::_dx, _camera, "modelData/���d��ȂƂ�/YaezawaNatori.pmx",
		L"vmdData\\a.vmd", false));
	if (!_models[0]->Load()) return false;
	_models[0]->Move(0, 0, 0);


	return true;
}

//�L�[�{�[�h�̏�����
void TitleScene::KeyboardInit()
{
	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));
	_keyboard->Init();
}

//�����_���[�̃o�b�t�@�[������
bool TitleScene::RendererBuffInit()
{
	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));

	_rsm->SetClearValue(0.5, 0.5, 0.5, 1.0);
	_modelRenderer->SetClearValue(0.5, 0.5, 0.5, 1.0);

	if (!_rsm->Init())
	{
		Application::DebugOutputFormatString("RSM�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	if (!_modelRenderer->Init())
	{
		Application::DebugOutputFormatString("���f�������_���[�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	if (!_ssao->Init())
	{
		Application::DebugOutputFormatString("SSAO�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	if (!_peraRenderer->Init())
	{
		Application::DebugOutputFormatString("PeraRenderer�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	return true;
}

//�e�N�X�`���̏�����
bool TitleScene::TextureInit()
{
	_textures.resize(2);
	_textures[0].reset(new Texture(Application::_dx, L"texture/start.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/yabai.png"));
	for (auto tex : _textures)
	{
		if (!tex->Init())
		{
			Application::DebugOutputFormatString("�e�N�X�`���̏������G���[\n ");
			return false;
		}
		_pera->SetSRV(tex->GetTexBuff(), tex->GetMetadata().format);
	}
	return true;
}

//���f���̏�����
bool TitleScene::ModelInit()
{
	for (auto model : _models)
	{
		if (!model->Init())
		{
			Application::DebugOutputFormatString("���f���̏������G���[\n ");
			return false;
		}
	}
	return true;
}

//�����_���[�̕`�敔���̏�����
bool TitleScene::RendererDrawInit()
{
	if (!_rsm->RendererInit(L"VertexShader.hlsl", "rsmVS", L"PixelShader.hlsl", "rsmPS"))
	{
		Application::DebugOutputFormatString("RSM�̃����_���[�������G���[\n ");
		return false;
	}
	if (!_modelRenderer->RendererInit(L"VertexShader.hlsl", "VS", L"PixelShader.hlsl", "PS"))
	{
		Application::DebugOutputFormatString("���f�������_���[�̃����_���[�������G���[\n ");
		return false;
	}
	if (!_ssao->RendererInit(L"SSAOVertexShader.hlsl", "ssaoVS", L"SSAOPixelShader.hlsl", "ssaoPS"))
	{
		Application::DebugOutputFormatString("SSAO�̃����_���[�������G���[\n ");
		return false;
	}
	if (!_peraRenderer->RendererInit(L"TitlePeraVertexShader.hlsl", "VS", L"TitlePeraPixelShader.hlsl", "PS"))
	{
		Application::DebugOutputFormatString("PeraRenderer�̃����_���[�������G���[\n ");
		return false;
	}
	return true;
}

//�{�^���̏�����
void TitleScene::ButtonInit()
{
	_StartButton.reset(new Button("Start"));
	int dx = Application::GetWindowSize().cx;
	int dy = Application::GetWindowSize().cy;
	_StartButton->Create(_T("Start"), (int)(dx * 0.45), (int)(dy * 0.9), (int)(dx * 0.1), (int)(dy * 0.1), (HMENU)1);
}

//�{�^���̍X�V
void TitleScene::ButtonUpdate()
{
	if (!_StartButton->IsActive())
	{
		if (_StartButton->IsHover())
		{
			_peraRenderer->HoverButton(_StartButton->GetName());
		}
		else
		{
			_peraRenderer->HoverCntReset();
		}
	}
	
}
