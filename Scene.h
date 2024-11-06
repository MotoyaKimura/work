#pragma once


#ifdef _DEBUG
#include <iostream>
#endif
#include "SceneManager.h"


class SceneManager;
class Scene 
{
private:
protected:
	SceneManager& _controller;

public:
	virtual void SceneUpdate(void) = 0;
	virtual bool SceneInit(void) = 0;
	virtual void SceneFinal(void) = 0;
	virtual void SceneRender(void) = 0;
	virtual void SceneResize(void) = 0;
	virtual const char* GetSceneName(void) = 0;

	Scene(SceneManager& controller);
	virtual ~Scene();
};
