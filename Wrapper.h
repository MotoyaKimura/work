#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>


class Wrapper
{
private:

	HWND _hwnd;

	SIZE winSize;

	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;
	std::vector<IDXGIAdapter*> adapters;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	ID3D12Fence* _fence = nullptr;
	
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorrect = {};

	std::vector<ID3D12Resource*> backBuffers;
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	UINT64 _fenceVal = 0;

public:
	Wrapper(HWND hwnd);
	void Init();
	void Update();
	void BeginDraw();
	void EndDraw();
	ID3D12Device* GetDevice() const;
	~Wrapper();
};