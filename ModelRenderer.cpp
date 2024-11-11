#include "ModelRenderer.h"
#include "Application.h"
#include "Wrapper.h"


bool ModelRenderer::Init()
{
	SetNumBuffers(2);
	SetResSize(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;

	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "VS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "PS", "ps_5_0", psBlob))) return false;
	SetRootSigParam();
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	AddElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
	if (!PipelineStateInit()) return false;

	return true;
}

void ModelRenderer::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap,false);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawModel();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void ModelRenderer::SetRootSigParam()
{
	CD3DX12_DESCRIPTOR_RANGE descTblRange;
	//�J�����A���f�����W�A�}�e���A��
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0);
	ranges.emplace_back(descTblRange);
	//���f���e�N�X�`���i���͎g���Ă��Ȃ��j �A���C�g�[�x�e�N�X�`��
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
	ranges.emplace_back(descTblRange);
	rootParam.InitAsDescriptorTable(ranges.size(), ranges.data());
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc;
	samplerDesc.Init(0);
	samplers.emplace_back(samplerDesc);
	samplerDesc.Init(
		1,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	samplers.emplace_back(samplerDesc);
}

ModelRenderer::ModelRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> keyboard)
	: Renderer(dx, pera, keyboard), _dx(dx), _pera(pera), _keyboard(keyboard)
{
}

ModelRenderer::~ModelRenderer()
{
}