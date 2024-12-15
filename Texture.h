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
	std::wstring _fileName;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _texBuff = nullptr;
	size_t AlignmentedSize(size_t size, size_t alignment) { return size + alignment - size % alignment; }
	bool LoadTexture(std::wstring fileName);
	bool UploadBufferInit();
	bool TexBufferInit();
	bool CopyBuffer();
	void ChangeBarrier();

	std::string GetStringFromWideString(const std::wstring& wstr);
	std::string GetExtension(const std::wstring& path);

	using LoadLambda_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLambda_t> loadLambdaTable;
	void DefineLambda();
public:
	bool Init();
	bool WhileTextureInit();
	bool BlackTextureInit();
	bool GradTextureInit();
	bool CreateDefaultTexture(Microsoft::WRL::ComPtr<ID3D12Resource> &buffer, size_t width, size_t height);

	Microsoft::WRL::ComPtr<ID3D12Resource> GetTexBuff() { return _texBuff.Get(); }
	DirectX::TexMetadata GetMetadata() { return metadata; }
	Texture(std::shared_ptr<Wrapper> dx, std::wstring fileName);
	~Texture();
};
