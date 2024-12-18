#pragma once
#include <Windows.h>
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <map>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "DirectXTex.lib")

class Wrapper
{
private:
	HWND _hwnd;
	SIZE winSize;

	Microsoft::WRL::ComPtr<ID3D12Device> _dev = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	std::vector< Microsoft::WRL::ComPtr<IDXGIAdapter>> adapters;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D12Resource>> mResourceTableW;


	bool DXGIInit();
	void DeviceInit();
	bool CMDInit();
	bool SwapChainInit();
	bool CreateBackBuffRTV();
	bool FenceInit();

public:
	bool Init();
	void ExecuteCommand();
	void Flip();
	void ResizeBackBuffers();
	std::wstring GetWideStringFromString(const std::string& str);
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTextureByPath(const std::wstring& path);
	std::string GetStringFromWideString(const std::wstring& wstr);

	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const {return _dev.Get();}
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const {return  _cmdList.Get();}
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const { return _swapchain.Get();}
	Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBuff() const { return backBuffers[_swapchain->GetCurrentBackBufferIndex()].Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeap() const { return rtvHeaps.Get(); }
	std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D12Resource>> GetResourceTable() const { return mResourceTableW; }

	Wrapper(HWND hwnd);
	~Wrapper();
};