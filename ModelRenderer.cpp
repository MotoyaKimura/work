#include "ModelRenderer.h"
#include "Application.h"
#include "Wrapper.h"


bool ModelRenderer::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!RootSignatureInit()) return false;
	if (!PipelineStateInit()) return false;
	SetNumBuffers(2);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;

	//if (!BuffInit()) return false;
	//if (!DepthBuffInit()) return false;
	return true;
}

bool ModelRenderer::BuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = _dx->GetBackBuff()->GetDesc();
	CD3DX12_CLEAR_VALUE clearValue(resDesc.Format, clsClr);
	_Buff.resize(2);

	for (auto& res : _Buff) {
		auto result = _dx->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));
		if (FAILED(result)) return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_RTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	
	auto handle = _RTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _Buff) {
		_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	return true;
}

bool ModelRenderer::DepthBuffInit()
{
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = Application::GetWindowSize().cx;
	depthResDesc.Height = Application::GetWindowSize().cy;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = _dx->GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_DepthBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = _dx->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(_DSVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	auto handle = _DSVHeap->GetCPUDescriptorHandleForHeapStart();
	_dx->GetDevice()->CreateDepthStencilView(
		_DepthBuff.Get(),
		&dsvDesc,
		handle);

	return true;
}

void ModelRenderer::BeginDraw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);

	/*SetBarrierStateToRT(_Buff);
	CD3DX12_CPU_DESCRIPTOR_HANDLE peraRTVHs[2] = {};
	SetRenderTargets(peraRTVHs, _RTVHeap, 2, _DSVHeap, 0);
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	CD3DX12_RECT rc(0, 0, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	_dx->GetCommandList()->RSSetViewports(1, &vp);
	_dx->GetCommandList()->RSSetScissorRects(1, &rc);*/
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
}

void ModelRenderer::EndDraw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void ModelRenderer::Draw()
{
	BeginDraw();
	DrawModel();
	EndDraw();
}


void ModelRenderer::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _buffers) {
		handle = heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
		_dx->GetDevice()->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}

	handle = heap->GetCPUDescriptorHandleForHeapStart();
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
	_dx->GetDevice()->CreateShaderResourceView(
		_depthBuffer.Get(),
		&srvDesc,
		handle);

}

void ModelRenderer::SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void ModelRenderer::SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToRT(res);
}

void ModelRenderer::SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void ModelRenderer::SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToSR(res);
}

//void ModelRenderer::SetRenderTargets(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[],
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs)
//{
//	auto baseHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
//	auto rtvIncSize = _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	uint32_t offset = 0;
//	for (int i = 0; i < numRTDescs; ++i) {
//		rtvHandle[i].InitOffsetted(baseHandle, offset);
//		offset += rtvIncSize;
//	}
//
//	if (dsvHeap)
//	{
//		auto dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
//		dsvHandle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * numDSDescs;
//		_dx->GetCommandList()->OMSetRenderTargets(numRTDescs, rtvHandle, true, &dsvHandle);
//		_dx->GetCommandList()->ClearDepthStencilView(
//			dsvHandle,
//			D3D12_CLEAR_FLAG_DEPTH,
//			1.0f,
//			0,
//			0,
//			nullptr);
//	}
//	else
//	{
//		_dx->GetCommandList()->OMSetRenderTargets(numRTDescs, rtvHandle, true, nullptr);
//	}
//	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
//	for (int i = 0; i < numRTDescs; ++i)
//	{
//		_dx->GetCommandList()->ClearRenderTargetView(rtvHandle[i], clearColor, 0, nullptr);
//	}
//}

ModelRenderer::ModelRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> keyboard)
	: Renderer(dx, pera, keyboard), _dx(dx), _pera(pera), _keyboard(keyboard)
{
}

ModelRenderer::~ModelRenderer()
{
}