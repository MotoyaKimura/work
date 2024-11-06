#pragma once
#include <Windows.h>
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "DirectXTex.lib")

class Wrapper
{
private:

	HWND _hwnd;

	SIZE winSize;

	Microsoft::WRL::ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	std::vector< Microsoft::WRL::ComPtr<IDXGIAdapter>> adapters;
	Microsoft::WRL::ComPtr<ID3D12Device> _dev = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC _rtvheapDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	

	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorrect = {};
	
	
	D3D12_RESOURCE_BARRIER depthBuffBarrierDesc = {};
	
	

	
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _depthSRVHeap = nullptr;

	


	

	





	bool DXGIInit();
	void DeviceInit();
	bool CMDInit();
	bool SwapChainInit();
	bool CreateBackBuffRTV();
	void ViewportInit();
	void ScissorrectInit();

	

public:
	Wrapper(HWND hwnd);
	bool Init();
	void Update();
	void ResizeBackBuffers();
	void ExecuteCommand();
	void Flip();
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeap() const;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBuffer() const;
	

	~Wrapper();
};