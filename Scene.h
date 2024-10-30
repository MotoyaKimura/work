#pragma once

#ifdef _DEBUG
#include <iostream>
#endif
#include "SceneManager.h"

class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;

class Scene 
{
private:

	
	static std::shared_ptr<Pera> _pera;
	static std::shared_ptr<Renderer> _renderer;
	static std::shared_ptr<Model> _model;
	static std::shared_ptr<Model> _model2;
	static std::shared_ptr<Model> _model3;
	static std::shared_ptr<Model> _model4;

	static std::shared_ptr<Keyboard> _keyboard;

	static bool SceneInit(void);
	static void SceneFinal(void);
	static void SceneUpdate(void);
	static void SceneRender(void);
	
public:

	static SceneProc SetupTestScene(void);
	Scene(std::shared_ptr<Wrapper> dx, HWND _hwnd);
	~Scene();
};
