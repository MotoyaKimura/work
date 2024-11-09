#include "PeraRenderer.h"
#include "Application.h"
#include "Wrapper.h"

bool PeraRenderer::Init(void)
{
	if (FAILED(!CompileShaderFile(L"PeraVertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PeraPixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	if (!PeraRootSignatureInit()) return false;
	if (!PeraPipelineStateInit()) return false;
	if (!PeraBuffInit()) return false;
	return true;
}

bool PeraRenderer::PeraBuffInit()
{

	return true;
}

void PeraRenderer::BeginDraw()
{
	SetBarrierStateToRT(_dx->GetBackBuff());
	SetRenderTargets(_dx->GetRTVHeap(), _dx->GetSwapChain()->GetCurrentBackBufferIndex(), nullptr, 0);
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	CD3DX12_RECT rc(0, 0, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	_dx->GetCommandList()->RSSetViewports(1, &vp);
	_dx->GetCommandList()->RSSetScissorRects(1, &rc);
	BeforeDraw(_peraPipelinestate.Get(), peraRootsignature.Get());
	
}

void PeraRenderer::EndDraw()
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		_dx->GetBackBuff().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void PeraRenderer::Draw()
{
	BeginDraw();
	DrawPera();
	EndDraw();
}


void PeraRenderer::SetBarrierStateToRT(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void PeraRenderer::SetBarrierStateToRT(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToRT(res);
}

void PeraRenderer::SetBarrierStateToSR(Microsoft::WRL::ComPtr<ID3D12Resource>const& buffer) const
{
	CD3DX12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	_dx->GetCommandList()->ResourceBarrier(1, &BarrierDesc);
}

void PeraRenderer::SetBarrierStateToSR(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers) const
{
	for (auto res : buffers)
		SetBarrierStateToSR(res);
}

void PeraRenderer::SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, int numRTDescs,
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
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	_dx->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

}

PeraRenderer::PeraRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> keyboard)
	: Renderer(dx, pera, keyboard), _dx(dx), _pera(pera), _keyboard(keyboard)
{
}

PeraRenderer::~PeraRenderer()
{
}