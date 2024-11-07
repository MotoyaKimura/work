#include "SSAO.h"
#include "Application.h"
#include "Wrapper.h"

bool SSAO::Init()
{
	if (FAILED(!CompileShaderFile(L"SSAOVertexShader.hlsl", "ssaoVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"SSAOPixelShader.hlsl", "ssaoPS", "ps_5_0", psBlob))) return false;
	if (!PeraRootSignatureInit()) return false;
	if (!SSAOPipelineStateInit()) return false;
	if (!SSAOBuffInit()) return false;

	return true;
}

bool SSAO::SSAOBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = _dx->GetBackBuff()->GetDesc();
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	float clsClr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R32_FLOAT, clsClr);
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(_SSAOBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;



	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	result = _dx->GetDevice()->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(_SSAORTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	_dx->GetDevice()->CreateRenderTargetView(
		_SSAOBuff.Get(),
		&rtvDesc,
		_SSAORTVHeap->GetCPUDescriptorHandleForHeapStart());


	return true;
}


void SSAO::BeginDraw()
{
	SetBarrierStateToRT(_SSAOBuff.Get());
	SetRenderTargets(_SSAORTVHeap, 0, nullptr, 0);
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	CD3DX12_RECT rc(0, 0, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	_dx->GetCommandList()->RSSetViewports(1, &vp);
	_dx->GetCommandList()->RSSetScissorRects(1, &rc);

	_dx->GetCommandList()->SetPipelineState(_ssaoPipelinestate.Get());
	_dx->GetCommandList()->SetGraphicsRootSignature(peraRootsignature.Get());
}

void SSAO::EndDraw()
{
	SetBarrierStateToSR(_SSAOBuff);
}

void SSAO::Draw()
{
	BeginDraw();
	DrawSSAO();
	EndDraw();
}

void SSAO::SetSRVsToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	auto handle = heap->GetCPUDescriptorHandleForHeapStart();


	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
	_dx->GetDevice()->CreateShaderResourceView(
		_SSAOBuff.Get(),
		&srvDesc,
		handle);
}

void SSAO::SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void SSAO::SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToRT(res);
}

void SSAO::SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void SSAO::SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToSR(res);
}

void SSAO::SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, int numDSDescs)
{
	auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * numRTDescs;
	if (dsvHeap)
	{
		auto dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		dsvHandle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * numDSDescs;

		_dx->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);
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
		_dx->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	}
	float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_dx->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

}

SSAO::SSAO(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard)
	: Renderer(dx, pera, _keyboard), _dx(dx), _pera(pera), _keyboard(_keyboard)
{
}

SSAO::~SSAO()
{
}