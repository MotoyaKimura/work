#pragma once
#include "Renderer.h"
#include <d3d12.h>
#include <d3dx12.h>

class RSM : public Renderer
{
private:
	UINT rsm_difinition = 2048;
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Keyboard> _keyboard;
	std::shared_ptr<Camera> _camera; 

public:
	bool Init() override;
	bool RendererInit(
		std::wstring VShlslFile, 
		std::string VSEntryPoint,
		std::wstring PShlslFile,
		std::string PSEntryPoint
	) override;
	void Draw() override;
	RSM(std::shared_ptr<Wrapper> dx,
		std::shared_ptr<Pera> pera,
		std::shared_ptr<Keyboard> _keyboard,
		std::vector<std::shared_ptr<Model>> models, 
		std::shared_ptr<Camera> camera);
	~RSM() override;
};
