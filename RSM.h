#pragma once
#include "Renderer.h"
#include <d3d12.h>
#include <d3dx12.h>

class Wrapper;
class Pera;
class Model;
class Keyboard;
class Renderer;
class RSM : public Renderer
{
private:
	UINT rsm_difinition = 1024;
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Keyboard> _keyboard;

public:
	bool Init();
	void Draw();
	void SetRootSigParam();
	void SetDepthBuffToHeap( Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs);
	
	RSM(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard, std::vector<std::shared_ptr<Model>> models);
	~RSM();
};
