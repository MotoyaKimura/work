#pragma once

#ifdef _DEBUG
#include <iostream>
#endif
#include "SceneManager.h"

class Scene 
{
private:
	
	static bool SceneInit(void);
	static void SceneFinal(void);
	static void SceneUpdate(void);
	static void SceneRender(void);
	
public:

	static SceneProc SetupTestScene(void);
	Scene();
	~Scene();
};
