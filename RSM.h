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
	unsigned int rsm_difinition = 1024;
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Keyboard> _keyboard;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _RSMBuff;
	Microsoft::WRL::ComPtr<ID3D12Resource> _lightDepthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _RSMRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _DSVHeap = nullptr;

	bool RSMBuffInit();
	bool LightDepthBuffInit();
	void SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer) const;
	void SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const;
	void SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer) const;
	void SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const;
	void SetRenderTargets(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[], Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs);

public:
	bool Init();
	void BeginDraw();
	void Draw();
	void EndDraw();
	void SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs);
	void SetDepthBuffToHeap( Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs);
	
	RSM(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard);
	~RSM();
};
