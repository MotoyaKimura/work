#pragma once
#include <Windows.h>
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
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
	CD3DX12_VIEWPORT viewport = {};
	CD3DX12_RECT scissorrect = {};
	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _peraBuff;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _RSMBuff;
	Microsoft::WRL::ComPtr<ID3D12Resource> _sceneTransBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _lightDepthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _ssaoBuff = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _RSMRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _ssaoRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _depthSRVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraSRVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _ssaoSRVHeap = nullptr;

	unsigned int peraSRVHeapNum = 0;
	std::map<std::string, unsigned int> _peraViewMap;
	unsigned int shadow_difinition = 1024;

	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

	struct SceneTransMatrix {
		DirectX::XMMATRIX view;//ビュー
		DirectX::XMMATRIX projection;//プロジェクション
		DirectX::XMMATRIX invProjection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX shadowOffsetY;
		DirectX::XMMATRIX invShadowOffsetY;
		DirectX::XMMATRIX lightView;
		DirectX::XMMATRIX lightCamera;
		DirectX::XMFLOAT3 lightVec;
		DirectX::XMFLOAT3 eye;
	};
	SceneTransMatrix* _sceneTransMatrix;

	DirectX::XMFLOAT3 lightVec;
	DirectX::XMFLOAT3 eye;
	DirectX::XMFLOAT3 rotate;
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
	void CalcSceneTrans();
	bool DepthBuffInit();
	bool CreateDepthHeap();
	bool CreateDepthView();
	bool RSMBuffInit();
	bool CreatePeraRTVAndSRV();
	bool LightDepthBuffInit();
	bool CreateSSAOBuff();
	bool CreateSSAOHeap();
	bool CreateSSAORTVAndSRV();
	void SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer) const;
	void SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const;
	void SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer) const;
	void SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const  &buffers) const;
	void SetRenderTargets(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[], Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs);
	void SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs);

public:
	Wrapper(HWND hwnd);
	bool Init();
	void Update();
	void BeginDrawRSM();
	void EndDrawRSM();
	void BeginDrawModel();
	void EndDrawModel();
	void BeginDrawSSAO();
	void EndDrawSSAO();
	void BeginDrawPera();
	void EndDrawPera();
	void ExecuteCommand();
	void Draw();
	void Flip();
	void ResizeBackBuffers();

	void SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs);
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const {return _dev.Get();}
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const {return  _cmdList.Get();}
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const { return _swapchain.Get();}
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetPeraSRVHeap() const { return _peraSRVHeap.Get();}
	Microsoft::WRL::ComPtr<ID3D12Resource> GetSceneTransBuff() const { return _sceneTransBuff.Get();}
	Microsoft::WRL::ComPtr<ID3D12Resource> GetLightDepthBuff() const { return _lightDepthBuff.Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBuff() const { return backBuffers[_swapchain->GetCurrentBackBufferIndex()].Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeap() const { return rtvHeaps.Get(); }
	DirectX::XMFLOAT3* GetEyePos() { return &eye; }
	DirectX::XMFLOAT3* GetTargetPos() { return &target; }
	DirectX::XMFLOAT3* GetEyeRotate() { return &rotate; }
	

	~Wrapper();
};