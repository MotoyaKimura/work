#pragma once
#include "Renderer.h"
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#pragma comment(lib, "DirectXTex.lib")

class PeraRenderer : public Renderer
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Keyboard> _keyboard;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Camera> _camera;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> _wipeBuff = nullptr;
	struct wipeBuffData {
		float _startWipeRight;
		float _endWipeRight;
		float _endWipeDown;
		bool _isPause;
	};
	wipeBuffData* _wipeBuffData = {};
	bool isWipe = false;
	bool wipeBuffInit();
	void SetCBVToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs) const;
	bool TextureInit();
	Microsoft::WRL::ComPtr<ID3D12Resource> _texBuff = nullptr;
	size_t AlignmentedSize(size_t size, size_t alignment) { return size + alignment - size % alignment; }
public:
	bool Init() override;
	void Draw() override;
	bool Update();
	void SetRootSigParam() override;
	bool LinearWipe();
	PeraRenderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard, std::vector<std::shared_ptr<Model>> models, std::shared_ptr<Camera> camera);
	~PeraRenderer() override;
};

