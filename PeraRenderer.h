#pragma once
#include "Renderer.h"
#include <d3d12.h>
#include <d3dx12.h>

class Wrapper;
class Pera;
class Keyboard;
class Renderer;
class PeraRenderer : public Renderer
{
private:
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Keyboard> _keyboard;
	std::vector<std::shared_ptr<Model>> _models;

	Microsoft::WRL::ComPtr<ID3D12Resource> _peraBuff;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraRTVHeap = nullptr;

	/*void SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs);*/

public:
	bool Init();
	void BeginDraw();
	void Draw();
	void EndDraw();
	void SetRootSigParam();
	PeraRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard, std::vector<std::shared_ptr<Model>> models);
	~PeraRenderer();
};

