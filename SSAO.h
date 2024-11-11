#pragma once
#include "Renderer.h"
#include <d3d12.h>
#include <d3dx12.h>

class Wrapper;
class Pera;
class Keyboard;
class Renderer;
class SSAO : public Renderer
{
protected:
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Keyboard> _keyboard;
	std::vector<std::shared_ptr<Model>> _models;
public:
	bool Init();
	void Draw();
	void SetRootSigParam();
	SSAO(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard, std::vector<std::shared_ptr<Model>> models);
	~SSAO();
};

