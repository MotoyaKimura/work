#pragma once
#include "Scene.h"
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
class HowToPlayScene : public Scene
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
	std::vector<std::shared_ptr<Model>> _models = {};
	UINT modelNum = 0;
	std::shared_ptr<Keyboard> _keyboard;
	std::shared_ptr<Texture> _howToPlayTex;
	
	std::shared_ptr<Button> _backButton;
	

	void SceneUpdate(void) override;
	bool SceneInit(void) override;
	void SceneFinal(void) override;
	void SceneRender(void) override;
	void SceneResize(void) override;
	const char* GetSceneName(void) override;

	bool PeraInit();
	bool CameraInit();
	bool TextureInit();
	bool RendererBuffInit();
	bool RendererDrawInit();
	void ButtonInit();
	void ButtonUpdate();
public:
	HowToPlayScene(SceneManager& controller);
	~HowToPlayScene() override;

};


