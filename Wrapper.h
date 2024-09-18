#pragma once
#include <Windows.h>
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <vector>

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
	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorrect = {};
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> _sceneTransBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _sceneTransHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr;

	struct SceneTransMatrix {
		DirectX::XMMATRIX view;//ビュー
		DirectX::XMMATRIX projection;//プロジェクション
	};
	SceneTransMatrix* _sceneTransMatrix;
	
	DirectX::XMFLOAT3 eye;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 up;

	bool DXGIInit();
	void DeviceInit();
	bool CMDInit();
	bool SwapChainInit();
	bool CreateRTV();
	void ViewportInit();
	void ScissorrectInit();
	bool SceneTransBuffInit();
	bool DepthBuffInit();

public:
	Wrapper(HWND hwnd);
	bool Init();
	void Update();
	void BeginDraw();
	void Draw();
	void EndDraw();
	void Flip();
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const;
	~Wrapper();
};