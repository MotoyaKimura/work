#include "Wrapper.h"
#include "Application.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;


bool EnableDebugLayer()
{
	ComPtr<ID3D12Debug> debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(debugLayer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

bool Wrapper::DXGIInit()
{
#ifdef _DEBUG
	EnableDebugLayer();
	auto result = CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG, 
		IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	if (FAILED(result)) return false;
	return true;
}

void Wrapper::DeviceInit()
{
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, tmpAdapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter.Get());
	}

	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (auto lv : levels)
	{
		if (D3D12CreateDevice(
			tmpAdapter.Get(), 
			lv,
			IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			featureLevel = lv;
			break;
		}
	}
}

bool Wrapper::CMDInit()
{
	auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	result = _dev->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = _dev->CreateCommandQueue(
		&cmdQueueDesc, 
		IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));

	return true;
}

bool Wrapper::SwapChainInit()
{
	swapchainDesc.Width = winSize.cx;
	swapchainDesc.Height = winSize.cy;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	auto result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		_hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());
	if (FAILED(result)) return false;
	return true;
}

bool Wrapper::CreateBackBuffRTV()
{
	D3D12_DESCRIPTOR_HEAP_DESC _rtvheapDesc = {};
	_rtvheapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	_rtvheapDesc.NodeMask = 0;
	_rtvheapDesc.NumDescriptors = 2;
	_rtvheapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	auto result = _dev->CreateDescriptorHeap(
		&_rtvheapDesc,
		IID_PPV_ARGS(rtvHeaps.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	backBuffers.resize(swapchainDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = 
		rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (int idx = 0; idx < swapchainDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(
			idx, 
			IID_PPV_ARGS(backBuffers[idx].ReleaseAndGetAddressOf()));
		if (FAILED(result)) return false;
		_dev->CreateRenderTargetView(
			backBuffers[idx].Get(),
			nullptr,
			handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	return true;
}

void Wrapper::ViewportInit()
{
	viewport.Width = winSize.cx;
	viewport.Height = winSize.cy;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
}

void Wrapper::ScissorrectInit()
{
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + winSize.cx;
	scissorrect.bottom = scissorrect.top + winSize.cy;
}

bool Wrapper::SceneTransBuffInit()
{

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneTransMatrix) + 0xff) & ~0xff);

	_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_sceneTransBuff.ReleaseAndGetAddressOf())
	);

	auto result = _sceneTransBuff->Map(0, nullptr, (void**)&_sceneTransMatrix);
	if (FAILED(result)) return false;
	_sceneTransMatrix->view = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&tangent),
		XMLoadFloat3(&up));;

	_sceneTransMatrix->projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(winSize.cx) / static_cast<float>(winSize.cy),
		1.0f,
		200.0f);

	XMFLOAT4 planeVec = XMFLOAT4(0, 1, 0, 0);
	_sceneTransMatrix->shadow = XMMatrixShadow(
		XMLoadFloat4(&planeVec),
		XMLoadFloat3(&lightVec));
	_sceneTransMatrix->shadowOffsetY = XMMatrixTranslation(0, 15, 0);
	XMVECTOR det;
	_sceneTransMatrix->invShadowOffsetY = XMMatrixInverse(&det , _sceneTransMatrix->shadowOffsetY);
	_sceneTransMatrix->lightVec = lightVec;
	_sceneTransMatrix->eye = eye;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 3;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = _dev->CreateDescriptorHeap(
		&descHeapDesc, 
		IID_PPV_ARGS(_sceneTransHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	auto handle = _sceneTransHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _sceneTransBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_sceneTransBuff->GetDesc().Width);

	_dev->CreateConstantBufferView(&cbvDesc, handle);

	return true;
}

bool Wrapper::DepthBuffInit()
{
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = winSize.cx;
	depthResDesc.Height = winSize.cy;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_depthBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = _dev->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	_dev->CreateDepthStencilView(
		_depthBuff.Get(),
		&dsvDesc, 
		_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}

bool Wrapper::CreatePeraRTVAndSRV()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = backBuffers[0]->GetDesc();
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	auto result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(_peraBuff.ReleaseAndGetAddressOf()));

	auto heapDesc = rtvHeaps->GetDesc();
	heapDesc.NumDescriptors = 1;

	result = _dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	_dev->CreateRenderTargetView(
		_peraBuff.Get(),
		nullptr,
		_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = _dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraSRVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	_dev->CreateShaderResourceView(
		_peraBuff.Get(),
		&srvDesc,
		_peraSRVHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}


Wrapper::Wrapper(HWND hwnd) :
	_hwnd(hwnd),
	eye(0, 50, -100),
	tangent(0, 0, 0),
	up(0, 1, 0),
lightVec(-1, 1, -1)
{
}

bool Wrapper::Init()
{

	const auto& app = Application::Instance();
	winSize = app.GetWindowSize();

	if(!DXGIInit()) return false;
	DeviceInit();
	if(!CMDInit()) return false;;
	if(!SwapChainInit()) return false;;
	if(!CreateBackBuffRTV()) return false;;
	if (!CreatePeraRTVAndSRV()) return false;;

	ViewportInit();
	ScissorrectInit();
	if(!SceneTransBuffInit()) return false;;
	if (!DepthBuffInit()) return false;;

	auto result = _dev->CreateFence(
		_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;


	
	return true;
}

void Wrapper::Update()
{
}

void Wrapper::BeginDrawTeapot()
{

	peraBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	peraBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	peraBuffBarrierDesc.Transition.pResource = _peraBuff.Get();
	peraBuffBarrierDesc.Transition.Subresource = 0;
	peraBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	peraBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &peraBuffBarrierDesc);

	auto peraRTVH = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(
		1,
		&peraRTVH,
		true,
		&dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	_cmdList->ClearRenderTargetView(peraRTVH, clearColor, 0, nullptr);
	_cmdList->ClearDepthStencilView(
		dsvH,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr);

	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);
}

void Wrapper::EndDrawTeapot()
{
	peraBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	peraBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	_cmdList->ResourceBarrier(1, &peraBuffBarrierDesc);
}


void Wrapper::BeginDrawPera()
{
	auto backBufferIndex = _swapchain->GetCurrentBackBufferIndex();

	backBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBuffBarrierDesc.Transition.pResource = backBuffers[backBufferIndex].Get();
	backBuffBarrierDesc.Transition.Subresource = 0;
	backBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &backBuffBarrierDesc);

	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += backBufferIndex *
		_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(
		1,
		&rtvH,
		true,
		&dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	_cmdList->ClearDepthStencilView(
		dsvH,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr);

	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);

}

void Wrapper::EndDrawPera()
{
	backBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &backBuffBarrierDesc);

	_cmdList->Close();

	ID3D12CommandList* cmdLists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdLists);

	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	auto result = _cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);

}

void Wrapper::Draw()
{
}

void Wrapper::Flip()
{
	_swapchain->Present(1, 0);
}

Microsoft::WRL::ComPtr<ID3D12Device> Wrapper::GetDevice() const
{
	return _dev.Get();
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> Wrapper::GetCommandList() const
{
	return _cmdList.Get();
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Wrapper::GetSceneTransHeap() const
{
	return _sceneTransHeap.Get();
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Wrapper::GetPeraSRVHeap() const
{
	return _peraSRVHeap.Get();
}

Wrapper::~Wrapper()
{
}
