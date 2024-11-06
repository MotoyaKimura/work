#pragma once
#include "Scene.h"
#include "SceneManager.h"

class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;
class TitleScene : public Scene
{
private:

	SceneManager& _controller;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Renderer> _renderer;
	std::vector<std::shared_ptr<Model>> _models;
	UINT modelNum = 0;
	std::shared_ptr<Keyboard> _keyboard;

	void FadeoutUpdate();

	void SceneUpdate(void);
	bool SceneInit(void);
	void SceneFinal(void);
	void SceneRender(void);
	void SceneResize(void);
	const char* GetSceneName(void);
public:
	TitleScene(SceneManager& controller);
	~TitleScene();

};


