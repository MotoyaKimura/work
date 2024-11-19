#include "PeraRenderer.h"
#include "Application.h"
#include "Model.h"
#include "Wrapper.h"
#include "Pera.h"

bool PeraRenderer::Init(void)
{
	wipeBuffInit();
	_pera->SetCBV(_wipeBuff);
	if (!TextureInit()) return false;
	_pera->SetSRV(_texBuff, metadata.format);
	
	return true;
}

bool PeraRenderer::RendererInit(std::wstring VShlslFile, std::string VSEntryPoint, std::wstring PShlslFile, std::string PSEntryPoint)
{
	if (FAILED(!CompileShaderFile(VShlslFile, VSEntryPoint, "vs_5_0", vsBlob))) return false;
	if (FAILED(!CompileShaderFile(PShlslFile, PSEntryPoint, "ps_5_0", psBlob))) return false;
	SetRootSigParam();
	if (!RootSignatureInit()) return false;
	AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	SetNumBuffers(1);
	SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	SetClearValue(0.5, 0.5, 0.5, 1.0);
	if (!PipelineStateInit()) return false;

	return true;
}

void PeraRenderer::Draw()
{
	_pera->SetViews();
	SetBarrierState(_dx->GetBackBuff(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetRenderTargets(_dx->GetRTVHeap(), nullptr, true);
	SetVPAndSR(Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	BeforeDraw(_pipelinestate.Get(), rootsignature.Get());
	DrawPera();
	SetBarrierState(_dx->GetBackBuff(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void PeraRenderer::SetRootSigParam()
{
	CD3DX12_DESCRIPTOR_RANGE descTblRange;
	//ペラポリゴン用テクスチャ、視点深度テクスチャ
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, _pera->GetSrvDescs(), 0);
	ranges.emplace_back(descTblRange);
	descTblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, _pera->GetCbvDescs(), 0);
	ranges.emplace_back(descTblRange);
	
	rootParam.InitAsDescriptorTable(2, ranges.data());
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);
	samplers.emplace_back(samplerDesc);
}

bool PeraRenderer::TextureInit()
{
	

	auto result = LoadFromWICFile(
		L"texture/start.png", DirectX::WIC_FLAGS_NONE,
		&metadata, scratchImg);

	if (FAILED(result)) {
		printf("WICテクスチャがロードできません\n");
		return false;
	}

	auto img = scratchImg.GetImage(0, 0, 0);				//生データ抽出

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(
		AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height,
		D3D12_RESOURCE_FLAG_NONE
	);
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuff = nullptr;
	result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		metadata.height,
		metadata.arraySize,
		metadata.mipLevels
	);

	result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(_texBuff.ReleaseAndGetAddressOf())
	);

	uint8_t* mapforImg = nullptr;
	result = uploadBuff->Map(0, nullptr, (void**)&mapforImg);
	auto srcAddress = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < img->height; y++)
	{
		std::copy_n(srcAddress, img->rowPitch, mapforImg);
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}
	uploadBuff->Unmap(0, nullptr);

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadBuff.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = metadata.width;
	src.PlacedFootprint.Footprint.Height = metadata.height;
	src.PlacedFootprint.Footprint.Depth = metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = _texBuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	_dx->GetCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	SetBarrierState(_texBuff, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	_dx->ExecuteCommand();

	return true;
}

bool PeraRenderer::wipeBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(wipeBuffData) + 0xff) & ~0xff);

	_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_wipeBuff.ReleaseAndGetAddressOf())
	);
	auto result = _wipeBuff->Map(0, nullptr, (void**)&_wipeBuffData);
	if (FAILED(result)) return false;
	_wipeBuffData->_startWipeRight = Application::GetWindowSize().cx;
	_wipeBuffData->_endWipeRight = 0.0f;
	_wipeBuffData->_endWipeDown = 0.0f;
	_wipeBuffData->_isPause = Application::GetPause();
	SetCBVToHeap(_pera->GetHeap(), 10);
	return true;
}

void PeraRenderer::SetCBVToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs) const
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs;
	cbvDesc.BufferLocation = _wipeBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_wipeBuff->GetDesc().Width);

	_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
}

bool PeraRenderer::Update()
{
	_wipeBuffData->_isPause = Application::GetPause();
	if (_wipeBuffData->_isPause)
		while (ShowCursor(true) < 0);

	return _wipeBuffData->_isPause;
}

bool PeraRenderer::LinearWipe()
{
	BYTE keyCode[256];
	GetKeyboardState(keyCode);
	if (keyCode['J'] & 0x80) isWipe = true;
	if (isWipe)
	{
		_wipeBuffData->_endWipeDown += 20;
		if (_wipeBuffData->_endWipeDown < Application::GetWindowSize().cy) return false;
		_wipeBuffData->_endWipeRight++;
		if (_wipeBuffData->_endWipeRight > 64.0f) return true;
	}
	else
	{
		if (_wipeBuffData->_startWipeRight <= 0) return false;
		_wipeBuffData->_startWipeRight -= 30;
	}
	
	return false;
}

PeraRenderer::PeraRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> keyboard, std::vector<std::shared_ptr<Model>> models, std::shared_ptr<Camera> camera)
	: Renderer(dx, pera, keyboard, models, camera), _dx(dx), _pera(pera), _keyboard(keyboard), _models(models), _camera(camera)
{
}

PeraRenderer::~PeraRenderer()
{
}