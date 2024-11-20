#pragma once
#include "Renderer.h"
#include <d3dx12.h>

class SSAO : public Renderer
{
protected:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Keyboard> _keyboard;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Camera> _camera; 
public:
	bool Init() override;
	bool RendererInit(std::wstring VShlslFile, std::string VSEntryPoint, std::wstring PShlslFile, std::string PSEntryPoint) override;
	void Draw() override;
	
	SSAO(
		std::shared_ptr<Wrapper> dx, 
		std::shared_ptr<Pera> pera, 
		std::shared_ptr<Keyboard> _keyboard, 
		std::vector<std::shared_ptr<Model>> models, 
		std::shared_ptr<Camera> camera
	);
	~SSAO() override;
};

