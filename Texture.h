#pragma once
#include <d3dx12.h>
#include <DirectXTex.h>

class Pera;
class Wrapper;
class Texture
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};
	const DirectX::Image* image = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _texBuff = nullptr;
	size_t AlignmentedSize(size_t size, size_t alignment) { return size + alignment - size % alignment; }
	bool LoadTexture(std::wstring fileName);
	bool UploadBufferInit();
	bool TexBufferInit();
	bool CopyBuffer();
	void ChangeBarrier();
public:
	bool Init(std::wstring fileName);
	Texture(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera);
	~Texture();
};
