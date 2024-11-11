#pragma once
#include "Renderer.h"
#include <d3d12.h>
#include <d3dx12.h>

class Camera;
class Wrapper;
class Pera;
class Model;
class Keyboard;
class Renderer;
class ModelRenderer : public Renderer
{
private:
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Keyboard> _keyboard;
	std::shared_ptr<Camera> _camera; 
public:
	bool Init();
	void Draw();
	void SetRootSigParam();
	ModelRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard, std::vector<std::shared_ptr<Model>> models, std::shared_ptr<Camera> camera);
	~ModelRenderer();
}; 
