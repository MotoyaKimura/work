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

	Microsoft::WRL::ComPtr<ID3D12Resource> _peraBuff;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraRTVHeap = nullptr;

	bool PeraBuffInit();

	void SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer) const;
	void SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const;
	void SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer) const;
	void SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const;
	void SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs);

public:
	bool Init();
	void BeginDraw();
	void Draw();
	void EndDraw();
	
	PeraRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard);
	~PeraRenderer();
};

