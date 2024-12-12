#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Wrapper.h"

class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;
class RSM;
class ModelRenderer;
class SSAO;
class PeraRenderer;
class Camera;
class Texture;
class Button;
class GameScene : public Scene
{
private:
	std::shared_ptr<RSM> _rsm;
	std::shared_ptr<ModelRenderer> _modelRenderer;
	std::shared_ptr<SSAO> _ssao;
	std::shared_ptr<PeraRenderer> _peraRenderer;
	std::shared_ptr<Camera> _camera;
	SceneManager& _controller;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Renderer> _renderer;
	std::vector<std::shared_ptr<Model>> _models;
	UINT modelNum = 0;
	bool isStart = false;
	bool isClear = false;
	bool isMenu = false;
	bool isBackFromHowToPlay = false;
	std::shared_ptr<Keyboard> _keyboard;
	std::shared_ptr<Texture> _pauseTex;
	std::shared_ptr<Texture> _zeroTex;
	std::shared_ptr<Texture> _oneTex;
	std::shared_ptr<Texture> _twoTex;
	std::shared_ptr<Texture> _threeTex;
	std::shared_ptr<Texture> _fourTex;
	std::shared_ptr<Texture> _fiveTex;
	std::shared_ptr<Texture> _sixTex;
	std::shared_ptr<Texture> _sevenTex;
	std::shared_ptr<Texture> _eightTex;
	std::shared_ptr<Texture> _nineTex;
	std::shared_ptr<Texture> _timerTex;
	std::shared_ptr<Texture> _colonTex;

	std::shared_ptr<Button> _button;

	void SceneUpdate(void) override;
	bool SceneInit(void) override;
	void SceneFinal(void) override;
	void SceneRender(void) override;
	void SceneResize(void) override;
	const char* GetSceneName(void) override;
public:
	GameScene(SceneManager& controller);
	~GameScene() override;

};


