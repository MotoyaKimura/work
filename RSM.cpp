#include "RSM.h"
#include "Application.h"
#include "Wrapper.h"


bool RSM::Init()
{
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "shadeVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "RSMPS", "ps_5_0", psBlob))) return false;
	if (!RootSignatureInit()) return false;
	if (!ShadowPipelineStateInit()) return false;
	if (!RSMBuffInit()) return false;
	if (!LightDepthBuffInit()) return false;
	return true;
}

bool RSM::RSMBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = _dx->GetBackBuff()->GetDesc();
	resDesc.Width = rsm_difinition;
	resDesc.Height = rsm_difinition;

	CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	_RSMBuff.resize(3);
	for (auto& res : _RSMBuff) {
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
	heapDesc.NumDescriptors = 3;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_RSMRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = _RSMRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _RSMBuff) {
		_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	return true;
}

bool RSM::LightDepthBuffInit()
{
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = rsm_difinition;
	depthResDesc.Height = rsm_difinition;
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
		IID_PPV_ARGS(_lightDepthBuff.ReleaseAndGetAddressOf()));
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
		_lightDepthBuff.Get(),
		&dsvDesc,
		handle);
	return true;
}

void RSM::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _RSMBuff) {
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
		_lightDepthBuff.Get(),
		&srvDesc,
		handle);

}

void RSM::SetDepthBuffToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();

	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
	_dx->GetDevice()->CreateShaderResourceView(
		_lightDepthBuff.Get(),
		&srvDesc,
		handle);

}

void RSM::SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void RSM::SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToRT(res);
}

void RSM::SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void RSM::SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToSR(res);
}

void RSM::SetRenderTargets(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[],
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs)
{
	auto baseHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize = _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	uint32_t offset = 0;
	for (int i = 0; i < numRTDescs; ++i) {
		rtvHandle[i].InitOffsetted(baseHandle, offset);
		offset += rtvIncSize;
	}

	if (dsvHeap)
	{
		auto dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		dsvHandle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * numDSDescs;
		_dx->GetCommandList()->OMSetRenderTargets(numRTDescs, rtvHandle, true, &dsvHandle);
		_dx->GetCommandList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr);
	}
	else
	{
		_dx->GetCommandList()->OMSetRenderTargets(numRTDescs, rtvHandle, true, nullptr);
	}
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	for (int i = 0; i < numRTDescs; ++i)
	{
		_dx->GetCommandList()->ClearRenderTargetView(rtvHandle[i], clearColor, 0, nullptr);
	}
}

void RSM::BeginDraw()
{
	SetBarrierStateToRT(_RSMBuff);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rsmRTVH[3] = {};
	SetRenderTargets(rsmRTVH, _RSMRTVHeap, 3, _DSVHeap, 0);
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, rsm_difinition, rsm_difinition);
	CD3DX12_RECT rc(0, 0, rsm_difinition, rsm_difinition);
	_dx->GetCommandList()->RSSetViewports(1, &vp);
	_dx->GetCommandList()->RSSetScissorRects(1, &rc);

	_dx->GetCommandList()->SetPipelineState(_shadowPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(rootsignature.Get());
}

void RSM::EndDraw()
{
	SetBarrierStateToSR(_RSMBuff);
}

void RSM::Draw()
{
	BeginDraw();
	DrawRSM();
	EndDraw();
}


RSM::RSM(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard)
	: Renderer(dx, pera, _keyboard), _dx(dx), _pera(pera), _keyboard(_keyboard)
{
}

RSM::~RSM()
{
}
