#include "Texture.h"
#include "Pera.h"
#include "Wrapper.h"

std::wstring Texture::GetWideStringFromString(const std::string& str)
{
	auto num1 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		nullptr,
		0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0],
		num1);

	assert(num1 == num2);
	return wstr;
}

std::string Texture::GetExtension(const std::string& path)
{
	return path.substr(path.find_last_of('.') + 1);
}

void Texture::DefineLambda()
{
	loadLambdaTable["sph"]
		= loadLambdaTable["bmp"]
		= loadLambdaTable["png"]
		= loadLambdaTable["jpg"]
		= [](const std::wstring& path, DirectX::TexMetadata* metadata, DirectX::ScratchImage& scratchImg)
		-> HRESULT
	{
		return DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, metadata, scratchImg);
	};

	loadLambdaTable["tga"]
		= [](const std::wstring& path, DirectX::TexMetadata* metadata, DirectX::ScratchImage& scratchImg)
		-> HRESULT
		{
			return DirectX::LoadFromTGAFile(path.c_str(), metadata, scratchImg);
		};

	loadLambdaTable["dds"]
		= [](const std::wstring& path, DirectX::TexMetadata* metadata, DirectX::ScratchImage& scratchImg)
		-> HRESULT
		{
			return DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, metadata, scratchImg);
		};
}

bool Texture::Init(std::string fileName)
{
	DefineLambda();
	auto wtexpath = GetWideStringFromString(fileName);
	_texBuff = _dx->GetTextureByPath(wtexpath);
	if (_texBuff == nullptr) {
		if (!LoadTexture(fileName)) return false;
		if (!UploadBufferInit()) return false;
		if (!TexBufferInit()) return false;
		if (!CopyBuffer()) return false;
		_dx->GetResourceTable()[wtexpath] = _texBuff;
	}
	ChangeBarrier();
	_dx->ExecuteCommand();
	return true;
}

void Texture::SetSRV()
{
	_pera->SetSRV(_texBuff, metadata.format);
}

bool Texture::LoadTexture(std::string fileName)
{
	auto wtexpath = GetWideStringFromString(fileName);
	auto ext = GetExtension(fileName);
	auto result = loadLambdaTable[ext](wtexpath, &metadata, scratchImg);
	

	if (FAILED(result)) {
		printf("テクスチャがロードできません\n");
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