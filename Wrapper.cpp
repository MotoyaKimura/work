#include "Wrapper.h"
#include "Application.h"
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "DirectXTex.lib")

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
	return true;
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

	//D3D_FEATURE_LEVEL featureLevel;

	for (auto lv : levels)
	{
		if (D3D12CreateDevice(
			tmpAdapter.Get(), 
			lv,
			IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			//featureLevel = lv;
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

	

	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = _dev->CreateCommandQueue(
		&cmdQueueDesc, 
		IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
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

void Wrapper::ResizeBackBuffers()
{
	ExecuteCommand();

	for (auto& backBuff : backBuffers)
	{
		backBuff.Reset();
	}
	_swapchain->ResizeBuffers(
		2,
		winSize.cx,
		winSize.cy,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);
	CreateBackBuffRTV();
	SceneTransBuffInit();
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
	for (unsigned int idx = 0; idx < swapchainDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(
			idx, 
			IID_PPV_ARGS(backBuffers[idx].ReleaseAndGetAddressOf())
		);
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
	
	return true;
}

void Wrapper::CalcSceneTrans()
{
	_sceneTransMatrix->view = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));;

	_sceneTransMatrix->projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(winSize.cx) / static_cast<float>(winSize.cy),
		1.0f,
		800.0f);
	XMVECTOR det;
	_sceneTransMatrix->invProjection = XMMatrixInverse(&det, _sceneTransMatrix->view * _sceneTransMatrix->projection);
	XMFLOAT4 planeVec = XMFLOAT4(0, 1, 0, 0);
	_sceneTransMatrix->shadow = XMMatrixShadow(
		XMLoadFloat4(&planeVec),
		XMLoadFloat3(&lightVec));
	_sceneTransMatrix->shadowOffsetY = XMMatrixTranslation(0, 15, 0);

	_sceneTransMatrix->invShadowOffsetY = XMMatrixInverse(&det, _sceneTransMatrix->shadowOffsetY);
	_sceneTransMatrix->lightVec = lightVec;
	_sceneTransMatrix->eye = eye;
	auto lightPos = XMLoadFloat3(&target) +
		XMVector3Normalize(XMLoadFloat3(&lightVec)) *
		XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];
	_sceneTransMatrix->lightCamera =
		XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) *
		XMMatrixOrthographicLH(150, 150, 1.0f, 800.0f);
	_sceneTransMatrix->lightView = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up));
}





Wrapper::Wrapper(HWND hwnd) :
	_hwnd(hwnd),
	eye(40, 20, 150),
	rotate(0, 0, 0),
	target(20, 30, 30),
	up(0, 1, 0),
lightVec(30, 30, 60),
_sceneTransMatrix(nullptr),
winSize(Application::Instance().GetWindowSize())
{
}

bool Wrapper::Init()
{
	if(!DXGIInit()) return false;
	DeviceInit();
	if(!CMDInit()) return false;
	if(!SwapChainInit()) return false;
	if(!CreateBackBuffRTV()) return false;

	if(!SceneTransBuffInit()) return false;
	CalcSceneTrans();

	auto result = _dev->CreateFence(
		_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}

void Wrapper::Update()
{
	CalcSceneTrans();
}

void Wrapper::ExecuteCommand()
{
	_cmdList->Close();
	ID3D12CommandList* cmdLists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdLists);
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		if (event == nullptr) return;
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	auto result = _cmdAllocator->Reset();
	if (FAILED(result)) return;
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);
}

void Wrapper::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs;
	cbvDesc.BufferLocation = _sceneTransBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_sceneTransBuff->GetDesc().Width);

	_dev->CreateConstantBufferView(&cbvDesc, handle);
}



void Wrapper::Draw()
{
}

void Wrapper::Flip()
{
	_swapchain->Present(1, 0);
}


Wrapper::~Wrapper()
{
}
