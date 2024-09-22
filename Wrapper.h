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

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC _rtvheapDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorrect = {};
	D3D12_RESOURCE_BARRIER backBuffBarrierDesc = {};
	D3D12_RESOURCE_BARRIER peraBuffBarrierDesc = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> _sceneTransBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _sceneTransHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _depthSRVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _peraBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraSRVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _lightDepthBuff = nullptr;
	unsigned int shadow_difinition = 1024;

	struct SceneTransMatrix {
		DirectX::XMMATRIX view;//ビュー
		DirectX::XMMATRIX projection;//プロジェクション
		DirectX::XMMATRIX invProjection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX shadowOffsetY;
		DirectX::XMMATRIX invShadowOffsetY;
		DirectX::XMMATRIX lightCamera;
		DirectX::XMFLOAT3 lightVec;
		DirectX::XMFLOAT3 eye;
	};
	SceneTransMatrix* _sceneTransMatrix;

	DirectX::XMFLOAT3 lightVec;
	DirectX::XMFLOAT3 eye;
	DirectX::XMFLOAT3 target;
	DirectX::XMFLOAT3 up;

	bool DXGIInit();
	void DeviceInit();
	bool CMDInit();
	bool SwapChainInit();
	bool CreateBackBuffRTV();
	void ViewportInit();
	void ScissorrectInit();
	bool SceneTransBuffInit();
	bool DepthBuffInit();
	bool CreatePeraRTVAndSRV();
	bool LightDepthBuffInit();
public:
	Wrapper(HWND hwnd);
	bool Init();
	void Update();
	void BeginDrawShadow();
	void BeginDrawTeapot();
	void EndDrawTeapot();
	void BeginDrawPera();
	void EndDrawPera();
	void Draw();
	void Flip();
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSceneTransHeap() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetPeraSRVHeap() const;
	~Wrapper();
};