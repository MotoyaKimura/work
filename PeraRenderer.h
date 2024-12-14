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
		float _startWipeOpen;
		float _endWipeClose;
		float _fade;
		float _gameOverFade;
		float _clearFade;
		float _monochromeRate;
		float ScreenWidth;
		float ScreenHeight;
		float _startHoverCnt;
		float _restartHoverCnt;
		float _titleHoverCnt;
		float credit;
		int _milliSecond;
		bool _isPause;
	};
	wipeBuffData* _wipeBuffData = {};
	int wipeOpenCnt = 0;
	int wipeCloseCnt = 0;
	float hoverCnt = 0.0f;
	float monochromeCnt = 0.0f;
	bool isWipe = false;
	DWORD startTime = 0;
	
	bool wipeBuffInit();
public:
	void HoverButton(std::string buttonName);
	void HoverCntReset();
	bool Init() override;
	bool RendererInit(
		std::wstring VShlslFile, 
		std::string VSEntryPoint, 
		std::wstring PShlslFile,
		std::string PSEntryPoint
	) override;
	void DataReset();
	void Draw() override;
	
	bool IsPause();
	void FadeIn();
	bool FadeOut();
	bool GameOverFadeOut();
	bool ClearFadeOut();
	bool IsWipeOpen();
	bool IsWipeClose();
	bool MonochromeToColor();
	void TimeStart();
	void CalcTime();
	bool TimeLimit();
	void TimeStop();
	void creditRoll();
	PeraRenderer(
		std::shared_ptr<Wrapper> dx,
		std::shared_ptr<Pera> pera, 
		std::shared_ptr<Keyboard> _keyboard, 
		std::vector<std::shared_ptr<Model>> models, 
		std::shared_ptr<Camera> camera
	);
	~PeraRenderer() override;
};

