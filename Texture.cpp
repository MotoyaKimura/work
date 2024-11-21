#include "Texture.h"
#include "Pera.h"
#include "Wrapper.h"

bool Texture::Init(std::wstring fileName)
{
	if (!LoadTexture(fileName)) return false;
	if (!UploadBufferInit()) return false;
	if (!TexBufferInit()) return false;
	if (!CopyBuffer()) return false;
	ChangeBarrier();
	_dx->ExecuteCommand();
	_pera->SetSRV(_texBuff, metadata.format);
	return true;
}

bool Texture::LoadTexture(std::wstring fileName)
{
	auto result = LoadFromWICFile(
		fileName.c_str(), DirectX::WIC_FLAGS_NONE,
		&metadata, scratchImg);

	if (FAILED(result)) {
		printf("WICテクスチャがロードできません\n");
		return false;
	}

	return true;
}

bool Texture::UploadBufferInit()
{
	image = scratchImg.GetImage(0, 0, 0);				//生データ抽出

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(
		AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * image->height,
		D3D12_RESOURCE_FLAG_NONE
	);
	
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	return true;
}

bool Texture::TexBufferInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		metadata.height,
		metadata.arraySize,
		metadata.mipLevels
	);

	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(_texBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	return true;
}

bool Texture::CopyBuffer()
{
	uint8_t* mapforImg = nullptr;
	auto result = uploadBuff->Map(0, nullptr, (void**)&mapforImg);
	if (FAILED(result)) return false;
	auto srcAddress = image->pixels;
	auto rowPitch = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < image->height; y++)
	{
		std::copy_n(srcAddress, image->rowPitch, mapforImg);
		srcAddress += image->rowPitch;
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
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = image->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = _texBuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	_dx->GetCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	return true;
}

void Texture::ChangeBarrier()
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_texBuff.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	_dx->GetCommandList()->ResourceBarrier(1, &barrier);
}


Texture::Texture(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera) : _dx(dx), _pera(pera)
{}

Texture ::~Texture()
{}