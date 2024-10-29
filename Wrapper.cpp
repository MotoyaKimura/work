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
	CreateSSAOBuff();
	CreateSSAORTVAndSRV();
	_depthBuff.Reset();
	DepthBuffInit();
	CreateDepthView();
	_sceneTransBuff.Reset();
	SceneTransBuffInit();
}


bool Wrapper::CreateBackBuffRTV()
{
	
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
	viewport.Width = (float)winSize.cx;
	viewport.Height = (float)winSize.cy;
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


	

	auto handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_sceneTransBuff"];
	cbvDesc.BufferLocation = _sceneTransBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_sceneTransBuff->GetDesc().Width);

	_dev->CreateConstantBufferView(&cbvDesc, handle);
	
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
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
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

	

	return true;
}

bool Wrapper::CreateDepthHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	auto result = _dev->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}


bool Wrapper::CreateDepthView()
{

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	_dev->CreateDepthStencilView(
		_depthBuff.Get(),
		&dsvDesc,
		_dsvHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	auto handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_depthBuff"];

	_dev->CreateShaderResourceView(
		_depthBuff.Get(),
		&srvDesc,
		handle);
	return true;
}


bool Wrapper::RSMBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = backBuffers[0]->GetDesc();
	resDesc.Width = shadow_difinition;
	resDesc.Height = shadow_difinition;
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	for (auto& res : _RSMBuff) {
		auto result = _dev->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));
		if (FAILED(result)) return false;
	}

	auto heapDesc = rtvHeaps->GetDesc();
	heapDesc.NumDescriptors = 3;

	auto result = _dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_RSMRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = _RSMRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _RSMBuff) {
		_dev->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	

	int cnt = _peraViewMap["_RSMBuff"];
	for (auto& res : _RSMBuff) {
		handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * cnt++;
		_dev->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}

	return true;
}


bool Wrapper::LightDepthBuffInit()
{
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = shadow_difinition;
	depthResDesc.Height = shadow_difinition;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
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
		IID_PPV_ARGS(_lightDepthBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	auto handle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_dev->CreateDepthStencilView(
		_lightDepthBuff.Get(),
		&dsvDesc,
		handle);


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	

	handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_lightDepthBuff"];
	_dev->CreateShaderResourceView(
		_lightDepthBuff.Get(),
		&srvDesc,
		handle);

	return true;
}


bool Wrapper::CreateSSAOBuff()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = backBuffers[0]->GetDesc();
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	float clsClr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R32_FLOAT, clsClr);
	auto result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(_ssaoBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}

bool Wrapper::CreateSSAOHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	auto result = _dev->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(_ssaoRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = _dev->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(_ssaoSRVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	
	return true;
}

bool Wrapper::CreateSSAORTVAndSRV()
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	_dev->CreateRenderTargetView(
		_ssaoBuff.Get(),
		&rtvDesc,
		_ssaoRTVHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	auto handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_ssaoBuff"];
	_dev->CreateShaderResourceView(
		_ssaoBuff.Get(),
		&srvDesc,
		handle);

	return true;
}




bool Wrapper::CreatePeraRTVAndSRV()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = backBuffers[0]->GetDesc();
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	for (auto& res : _peraBuff) {
		auto result = _dev->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));
		if (FAILED(result)) return false;
	}

	auto heapDesc = rtvHeaps->GetDesc();
	heapDesc.NumDescriptors = 2;

	auto result = _dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _peraBuff) {
		_dev->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 9;

	_peraViewMap["_peraBuff"] = peraSRVHeapNum;
	peraSRVHeapNum += _peraBuff.size();
	_peraViewMap["_RSMBuff"] = peraSRVHeapNum;
	peraSRVHeapNum += _RSMBuff.size();
	_peraViewMap["_depthBuff"] = peraSRVHeapNum++;
	_peraViewMap["_lightDepthBuff"] = peraSRVHeapNum++;
	_peraViewMap["_ssaoBuff"] = peraSRVHeapNum++;
	_peraViewMap["_sceneTransBuff"] = peraSRVHeapNum++;

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
	handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	
	int cnt = _peraViewMap["_peraBuff"];
	for (auto& res : _peraBuff) {
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * cnt++;
		_dev->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}
	return true;
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
	if (!CreatePeraRTVAndSRV()) return false;

	ViewportInit();
	ScissorrectInit();
	if (!RSMBuffInit()) return false;
	if (!DepthBuffInit()) return false;
	if (!CreateDepthHeap()) return false;
	if (!CreateDepthView()) return false;
	if (!LightDepthBuffInit()) return false;
	if (!CreateSSAOBuff()) return false;
	if (!CreateSSAOHeap()) return false;
	if (!CreateSSAORTVAndSRV()) return false;
	if(!SceneTransBuffInit()) return false;
	CalcSceneTrans();

	auto result = _dev->CreateFence(
		_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;


	
	return true;
}

void Wrapper::Update()
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

void Wrapper::BeginDrawTeapot()
{
	for (auto& res : _peraBuff) {
		peraBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		peraBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		peraBuffBarrierDesc.Transition.pResource = res.Get();
		peraBuffBarrierDesc.Transition.Subresource = 0;
		peraBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		peraBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		_cmdList->ResourceBarrier(1, &peraBuffBarrierDesc);
	}
	auto handle = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE peraRTVHs[2] = {};
	uint32_t offset = 0;
	for (auto& peraRTVH : peraRTVHs) {
		peraRTVH.InitOffsetted(handle, offset);
		offset += rtvIncSize;
	}


	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	_cmdList->OMSetRenderTargets(
		2,
		peraRTVHs,
		true,
		&dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	for (int i = 0; i < _countof(peraRTVHs); ++i)
	{
		_cmdList->ClearRenderTargetView(peraRTVHs[i], clearColor, 0, nullptr);
	}
	
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
	for (auto& res : _peraBuff) {
		peraBuffBarrierDesc.Transition.pResource = res.Get();
		peraBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		peraBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		_cmdList->ResourceBarrier(1, &peraBuffBarrierDesc);
	}
}

void Wrapper::BeginDrawSSAO()
{
	ssaoBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ssaoBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ssaoBuffBarrierDesc.Transition.pResource = _ssaoBuff.Get();
	ssaoBuffBarrierDesc.Transition.Subresource = 0;
	ssaoBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	ssaoBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &ssaoBuffBarrierDesc);
	auto rtvH = _ssaoRTVHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(
		1, 
		&rtvH, 
		false, 
		nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, winSize.cx, winSize.cy);
	CD3DX12_RECT rc(0, 0, winSize.cx, winSize.cy);
	_cmdList->RSSetViewports(1, &vp);
	_cmdList->RSSetScissorRects(1, &rc);
}

void Wrapper::EndDrawSSAO()
{
	ssaoBuffBarrierDesc.Transition.pResource = _ssaoBuff.Get();
	ssaoBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ssaoBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	_cmdList->ResourceBarrier(1, &ssaoBuffBarrierDesc);
}



void Wrapper::BeginDrawShade()
{
	for (auto& res : _RSMBuff) {
		RSMBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		RSMBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		RSMBuffBarrierDesc.Transition.pResource = res.Get();
		RSMBuffBarrierDesc.Transition.Subresource = 0;
		RSMBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		RSMBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		_cmdList->ResourceBarrier(1, &RSMBuffBarrierDesc);
	}
	auto handle = _RSMRTVHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE RSMRTVHs[3] = {};
	uint32_t offset = 0;
	for (auto& rsmRTVH : RSMRTVHs) {
		rsmRTVH.InitOffsetted(handle, offset);
		offset += rtvIncSize;
	}

	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvH.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_cmdList->OMSetRenderTargets(3, RSMRTVHs, true, &dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	for (int i = 0; i < _countof(RSMRTVHs); ++i)
	{
		_cmdList->ClearRenderTargetView(RSMRTVHs[i], clearColor, 0, nullptr);
	}

	_cmdList->ClearDepthStencilView(
		dsvH,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, shadow_difinition, shadow_difinition);
	CD3DX12_RECT rc(0, 0, shadow_difinition, shadow_difinition);
	_cmdList->RSSetViewports(1, &vp);
	_cmdList->RSSetScissorRects(1, &rc);
}

void Wrapper::EndDrawShade()
{
	for (auto& res : _RSMBuff) {
		RSMBuffBarrierDesc.Transition.pResource = res.Get();
		RSMBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		RSMBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		_cmdList->ResourceBarrier(1, &RSMBuffBarrierDesc);
	}
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

	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);

}

void Wrapper::EndDrawPera()
{
	backBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &backBuffBarrierDesc);
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



Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Wrapper::GetPeraSRVHeap() const
{
	return _peraSRVHeap.Get();
}

Microsoft::WRL::ComPtr<ID3D12Resource> Wrapper::GetSceneTransBuff() const
{
	return _sceneTransBuff.Get();
}

Microsoft::WRL::ComPtr<ID3D12Resource> Wrapper::GetLightDepthBuff() const
{
	return _lightDepthBuff.Get();
}

DirectX::XMFLOAT3* Wrapper::GetEyePos()
{
	return &eye;
}

DirectX::XMFLOAT3* Wrapper::GetTargetPos()
{
	return &target;
}

DirectX::XMFLOAT3* Wrapper::GetEyeRotate()
{
	return &rotate;
}



Wrapper::~Wrapper()
{
}
