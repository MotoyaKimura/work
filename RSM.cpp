#include "RSM.h"
#include "Application.h"
#include "Wrapper.h"


bool RSM::Init()
{
	SetNumBuffers(3);
	SetResSize(rsm_difinition, rsm_difinition);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!CreateBuffers()) return false;
	if (!CreateDepthBuffer()) return false;
	if (FAILED(!CompileShaderFile(L"VertexShader.hlsl", "shadeVS", "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(L"PixelShader.hlsl", "RSMPS", "ps_5_0", psBlob))) return false;
	SetRootSigParam();
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	AddElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
	if (!PipelineStateInit()) return false;
	
	return true;
}

void RSM::SetDepthBuffToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto handle = heap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs++;
	_dx->GetDevice()->CreateShaderResourceView(
		_depthBuffer.Get(),
		&srvDesc,
		handle);
}

void RSM::Draw()
{
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_rtvHeap, _dsvHeap,false);
	SetVPAndSR(rsm_difinition, rsm_difinition);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawModel();
	SetBarrierState(_buffers, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void RSM::SetRootSigParam()
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

RSM::RSM(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard)
	: Renderer(dx, pera, _keyboard), _dx(dx), _pera(pera), _keyboard(_keyboard)
{
}

RSM::~RSM()
{
}