#pragma once
#include "Renderer.h"
#include <d3dx12.h>
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
		float _endWipeCenter;
		float _fade;
		bool _isPause;
	};
	wipeBuffData* _wipeBuffData = {};
	int cnt = 0;
	bool isWipe = false;
	bool wipeBuffInit();
	
public:
	bool Init() override;
	bool RendererInit(
		std::wstring VShlslFile, 
		std::string VSEntryPoint, 
		std::wstring PShlslFile,
		std::string PSEntryPoint
	) override;
	void DataReset();
	void Draw() override;
	bool Update();
	bool IsPause();
	bool IsStart();
	bool FadeOut();
	bool WipeEnd();
	PeraRenderer(
		std::shared_ptr<Wrapper> dx,
		std::shared_ptr<Pera> pera, 
		std::shared_ptr<Keyboard> _keyboard, 
		std::vector<std::shared_ptr<Model>> models, 
		std::shared_ptr<Camera> camera
	);
	~PeraRenderer() override;
};

