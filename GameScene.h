#pragma once
#include "Scene.h"
#include "SceneManager.h"

class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;

class GameScene : public Scene
{
private:
	static std::shared_ptr<Pera> _pera;
	static std::shared_ptr<Renderer> _renderer;
	static std::shared_ptr<Model> _model;
	static std::shared_ptr<Model> _model2;
	static std::shared_ptr<Model> _model3;
	static std::shared_ptr<Model> _model4;

	static std::shared_ptr<Keyboard> _keyboard;


	void FadeoutUpdate();
public:
	GameScene(SceneManager& controller);
	~GameScene();

	SceneManager _controller;
	void SceneUpdate(void);
	bool SceneInit(void);
	void SceneFinal(void);
	void SceneRender(void);
	const char* GetSceneName(void);

};


