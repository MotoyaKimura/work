#include "Texture.h"
#include <iostream>
#include "Pera.h"
#include "Wrapper.h"

//テクスチャクラス
Texture::Texture(std::shared_ptr<Wrapper> dx, std::wstring fileName) : _dx(dx), _fileName(fileName)
{
}

Texture ::~Texture()
{
}

//テクスチャの初期化
bool Texture::Init()
{
	DefineLambda();
	_texBuff = _dx->GetTextureByPath(_fileName);
	if (_texBuff == nullptr) {
		if (!LoadTexture(_fileName)) return false;
		if (!UploadBufferInit()) return false;
		if (!TexBufferInit()) return false;
		if (!CopyBuffer()) return false;
		_dx->GetResourceTable()[_fileName] = _texBuff;
	}
	ChangeBarrier();
	_dx->ExecuteCommand();
	return true;
}

//拡張子に対応したロード関数を定義
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

//ワイド文字列から文字列へ変換
std::string Texture::GetStringFromWideString(const std::wstring& wstr)
{
	auto num1 = WideCharToMultiByte(
		CP_ACP,
		0,
		wstr.c_str(),
		-1,
		nullptr,
		0,
		nullptr,
		nullptr);

	std::string str;
	str.resize(num1);

	auto num2 = WideCharToMultiByte(
		CP_ACP,
		0,
		wstr.c_str(),
		-1,
		&str[0],
		num1,
		nullptr,
		nullptr);

	assert(num1 == num2);
	return str;
}

//拡張子の取得
std::string Texture::GetExtension(const std::wstring& path)
{
	auto str = GetStringFromWideString(path);
	return str.substr(str.find_last_of('.') + 1);
}

//テクスチャのロード
bool Texture::LoadTexture(std::wstring fileName)
{
	auto extW = fileName.substr(fileName.find_last_of(L'.') + 1);
	std::string ext(extW.begin(), extW.end());
	auto result = loadLambdaTable[ext](fileName, &metadata, scratchImg);

	if (FAILED(result)) {
		printf("テクスチャがロードできません\n");
		return false;
	}

	return true;
}

//アップロードバッファの初期化
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
	if (FAILED(result))
	{
		std::cout << "アップロードバッファの初期化に失敗しました" << std::endl;
		return false;
	}
	return true;
}

//テクスチャバッファの初期化
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

	if (FAILED(result))
	{
		std::cout << "テクスチャバッファの初期化に失敗しました" << std::endl;
		return false;
	}

	return true;
}

//バッファのコピー
bool Texture::CopyBuffer()
{
	uint8_t* mapforImg = nullptr;
	auto result = uploadBuff->Map(0, nullptr, (void**)&mapforImg);
	if (FAILED(result))
	{
		std::cout << "アップロードバッファのマップに失敗しました" << std::endl;
		return false;
	}
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

//バリアの変更
void Texture::ChangeBarrier()
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_texBuff.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	_dx->GetCommandList()->ResourceBarrier(1, &barrier);
}

//デフォルトテクスチャの作成
bool Texture::CreateDefaultTexture(Microsoft::WRL::ComPtr<ID3D12Resource> &buffer, size_t width, size_t height)
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		width,
		height,
		1,
		1
	);

	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result))
	{
		std::cout << "デフォルトテクスチャの作成に失敗しました" << std::endl;
		return false;
	}

	return true;
}

//白テクスチャの初期化
bool Texture::WhileTextureInit()
{
	if (!CreateDefaultTexture(_texBuff, 4, 4)) return false;
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	auto result = _texBuff->WriteToSubresource(
		0, 
		nullptr, 
		data.data(), 
		4 * 4, 
		data.size()
	);
	if (FAILED(result))
	{
		std::cout << "白テクスチャの初期化に失敗しました" << std::endl;
		return false;
	}
	return true;
}

//黒テクスチャの初期化
bool Texture::BlackTextureInit()
{
	if (!CreateDefaultTexture(_texBuff, 4, 4)) return false;
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x0);

	auto result = _texBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size()
	);
	
	if (FAILED(result))
	{
		std::cout << "黒テクスチャの初期化に失敗しました" << std::endl;
		return false;
	}
	
	return true;
}

//グラデーションテクスチャの初期化
bool Texture::GradTextureInit()
{
	if (!CreateDefaultTexture(_texBuff, 4, 256)) return false;
	std::vector<unsigned char> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4)
	{
		auto col = (c << 0xff) | (c << 16) | (c << 8) | c;
		std::fill(it, it + 4, col);
		--c;
	}

	auto result = _texBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * sizeof(unsigned int),
		data.size() * sizeof(unsigned int)
	);

	if (FAILED(result)) {
		std::cout << "グラデーションテクスチャの初期化に失敗しました" << std::endl;
		return false;
	}
	return true;
}


