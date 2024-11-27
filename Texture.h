#pragma once
#include <d3dx12.h>
#include <DirectXTex.h>
#include <map>

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
	bool LoadTexture(std::string fileName);
	bool UploadBufferInit();
	bool TexBufferInit();
	bool CopyBuffer();
	void ChangeBarrier();
	std::wstring GetWideStringFromString(const std::string& str);
	std::string GetExtension(const std::string& path);

	using LoadLambda_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLambda_t> loadLambdaTable;
	void DefineLambda();
public:
	bool Init(std::string fileName);
	void SetSRV();
	Texture(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera);
	~Texture();
};
