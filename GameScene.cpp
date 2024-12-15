#include "Application.h"
#include "GameScene.h"
#include <tchar.h>
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
#include "MenuScene.h"
#include "AssimpModel.h"
#include "PmxModel.h"
#include  "GameOverScene.h"
#include "ClearScene.h"
#include "HowToPlayScene.h"

GameScene::GameScene(SceneManager& controller)
	: Scene(controller), _controller(controller)
{
}

GameScene::~GameScene()
{
}

//�V�[���̏�����
bool GameScene::SceneInit()
{
	if (!PeraInit()) return false;
	if (!CameraInit()) return false;
	ModelReset();
	KeyboardInit();
	if (!RendererBuffInit()) return false;
	if (!TextureInit()) return false;
	if (!ModelInit()) return false;
	if (!RendererDrawInit()) return false;
	
	return true;
}

//�V�[���̍X�V
void GameScene::SceneUpdate(void)
{
	_peraRenderer->FadeIn();
	//���߂Ƀ��m�N������J���[�ɕϊ��A�����Ēǉ��ŃV���v���b�N�X�m�C�Y��������
	_peraRenderer->MonochromeToColor();
	//�����オ������A���|�[�Y��ʂłȂ��Ƃ����f���̑���\
	if (!_peraRenderer->IsPause() && !_peraRenderer->IsWipeOpen())
	{
		isStart = true;
		_keyboard->Update();
		_camera->CalcSceneTrans();
	}

	//���[�V�����̓|�[�Y��ʂłȂ��Ƃ��̂ݍX�V
	if(_peraRenderer->IsPause())
	{
		_keyboard->SetIsPause(Application::GetPause());
		_peraRenderer->TimeStop();
	}
	else
	{
		_rsm->Update(isStart);
		_modelRenderer->Update(isStart);
	}

	//�V�ѕ��V�[������߂�܂Ŏ��Ԃ͎~�߂Ă���
	if(isBackFromHowToPlay)
	{
		isBackFromHowToPlay = false;
		_peraRenderer->TimeStop();
	}
	//���j���[����߂�܂Ŏ��Ԃ͎~�߂Ă���
	if(isMenu)
	{
		isMenu = false;
		_peraRenderer->TimeStop();
	}
	//�����オ��܂Ŏ��Ԃ͎~�߂Ă���
	if (isStart)
	{
		_peraRenderer->CalcTime();
	}
	else
	{
		_peraRenderer->TimeStop();
	}
	//�����オ������܂��V�ѕ��V�[���֑J�ځi��x����j
	if (isStart && !Application::GetIsShowHowToPlay())
	{
		_keyboard->SetIsHowToPlay(true);
		isBackFromHowToPlay = true;
		_controller.PushScene(new HowToPlayScene(_controller));
		return;
	}

	//��������Q�[���I�[�o�[�V�[���֑J��
	//�������Ԃ��߂�����Q�[���I�[�o�[�V�[���֑J��
	if ((_camera->GetEyePos()->y <= -100 || _peraRenderer->TimeLimit()) && !isClear)
	{
		if (_peraRenderer->GameOverFadeOut())
		{
			_controller.ChangeScene(new GameOverScene(_controller));
			return;
		}
	}

	//�v���C���[�����郉�C���𒴂�����N���A�V�[���֑J��
	if (_models[0]->GetPos()->x > -15 && _models[0]->GetPos()->x < 15 &&
		_models[0]->GetPos()->y >= 27 &&
		_models[0]->GetPos()->z >= 5 * 14 - 10 || isClear)
	{
		isClear = true;
		if (_peraRenderer->ClearFadeOut())
		{
			_controller.ChangeScene(new ClearScene(_controller));
			return;
		}
		return;
	}

	//�G�X�P�[�v�������ꂽ�烁�j���[�V�[���֑J��
	if (Application::GetMenu())
	{
		if (_peraRenderer->FadeOut())
		{
			isMenu = true;
			_keyboard->SetIsMenu(true);
			_controller.PushScene(new MenuScene(_controller));
			return;
		}
	}
}

//�V�[���̕`��
void GameScene::SceneRender(void)
{

	_rsm->Draw();
	_modelRenderer->Draw();
	_ssao->Draw();
	_peraRenderer->Draw();

	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();

}

//�V�[���̏I��
void GameScene::SceneFinal(void)
{
	_renderer.reset();
	_pera.reset();
	_keyboard.reset();
	_models.clear();
}

//�V�[���̃��T�C�Y
void GameScene::SceneResize(void)
{
	Application::_dx->ResizeBackBuffers();
}

const char* GameScene::GetSceneName(void)
{
	return "GameScene";
}

//�y���|���S���̏�����
bool GameScene::PeraInit()
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
bool GameScene::CameraInit()
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
void GameScene::ModelReset()
{
	modelNum = 22;
	_models.resize(modelNum);
	_models[0].reset(new PmxModel(Application::_dx, _camera, "modelData/���d��ȂƂ�/YaezawaNatori.pmx",
		L"vmdData\\1.�ڂ���҂�_(490f_�ړ��Ȃ�).vmd", true));
	_models[0]->Move(0, 0.0, 0);

	_models[1].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/floor/floor_yellow.obj"));
	_models[2].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/piller/piller.obj"));
	_models[2]->Move(9, 0, 9);
	_models[3].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/piller/piller.obj"));
	_models[3]->Move(-9, 0, 9);
	for (int i = 0; i < 10; i++)
	{
		_models[i + 4].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/floor/floor_white.obj"));
		_models[i + 4]->Move(0, 2.5 * (i + 1), 5 * (i + 1));
	}
	_models[14].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/floor/floor_lightBlue.obj"));
	_models[14]->Move(0, 2.5 * 11, 5 * 15 - 1);

	//�ƃ��f���i��Ղɂł��Ȃ��̂łP���A���f�����F�U�j
	_models[15].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/house/cafe.obj"));
	_models[16].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/house/chimney.obj"));
	_models[17].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/house/roof.obj"));
	_models[18].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/house/knob.obj"));
	_models[19].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/house/door.obj"));
	_models[20].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/house/base.obj"));
	for (int i = 0; i < 6; i++)
	{
		_models[15 + i]->Move(0, 2.5 * 11 + 0.5, 5 * 18);
	}
	_models[21].reset(new AssimpModel(Application::_dx, _camera, "modelData/RSMScene/sky/skySphere.obj"));
	_models[21]->SetAABB(0, 0, 0, 0, 0, 0);
}

//�L�[�{�[�h�̏�����
void GameScene::KeyboardInit()
{
	_keyboard.reset(new Keyboard(Application::GetHwnd(), _camera, _models));
	_keyboard->Init();
}

bool GameScene::RendererBuffInit()
{
	//�����_���[�̃o�b�t�@�[������
	_rsm.reset(new RSM(Application::_dx, _pera, _keyboard, _models, _camera));
	_modelRenderer.reset(new ModelRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_ssao.reset(new SSAO(Application::_dx, _pera, _keyboard, _models, _camera));
	_peraRenderer.reset(new PeraRenderer(Application::_dx, _pera, _keyboard, _models, _camera));
	_rsm->SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!_rsm->Init())
	{
		Application::DebugOutputFormatString("RSM�̃o�b�t�@�[�������G���[\n ");
		return false;
	}
	_modelRenderer->SetClearValue(0.5, 0.5, 0.5, 1.0);
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
bool GameScene::TextureInit()
{
	_textures.resize(13);
	_textures[0].reset(new Texture(Application::_dx, L"texture/pause.png"));
	_textures[1].reset(new Texture(Application::_dx, L"texture/zero.png"));
	_textures[2].reset(new Texture(Application::_dx, L"texture/one.png"));
	_textures[3].reset(new Texture(Application::_dx, L"texture/two.png"));
	_textures[4].reset(new Texture(Application::_dx, L"texture/three.png"));
	_textures[5].reset(new Texture(Application::_dx, L"texture/four.png"));
	_textures[6].reset(new Texture(Application::_dx, L"texture/five.png"));
	_textures[7].reset(new Texture(Application::_dx, L"texture/six.png"));
	_textures[8].reset(new Texture(Application::_dx, L"texture/seven.png"));
	_textures[9].reset(new Texture(Application::_dx, L"texture/eight.png"));
	_textures[10].reset(new Texture(Application::_dx, L"texture/nine.png"));
	_textures[11].reset(new Texture(Application::_dx, L"texture/timer.png"));
	_textures[12].reset(new Texture(Application::_dx, L"texture/colon.png"));
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
bool GameScene::ModelInit()
{
	for (auto model : _models)
	{
		if (!model->Init())
		{
			Application::DebugOutputFormatString("���f���̏������G���[\n ");
			return false;
		}
		if (!model->RendererInit())
		{
			Application::DebugOutputFormatString("���f���̃����_���[�������G���[\n ");
			return false;
		}
	}
	return true;
}

//�e�탌���_���[�N���X�̕`�敔���̏�����
bool GameScene::RendererDrawInit()
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
	if (!_peraRenderer->RendererInit(L"PeraVertexShader.hlsl", "VS", L"PeraPixelShader.hlsl", "PS"))
	{
		Application::DebugOutputFormatString("PeraRenderer�̃����_���[�������G���[\n ");
		return false;
	}
	return true;
}