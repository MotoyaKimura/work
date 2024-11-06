#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include <array>
#include <DirectXMath.h>
#include <map>

class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;
class TitleScene : public Scene
{
private:
	static std::shared_ptr<Pera> _pera;
	static std::shared_ptr<Renderer> _renderer;
	static std::shared_ptr<Model> _model;
	static std::shared_ptr<Model> _model2;
	static std::shared_ptr<Model> _model3;
	static std::shared_ptr<Model> _model4;

	static std::shared_ptr<Keyboard> _keyboard;
	
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> _peraBuff;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 3> _RSMBuff;
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _lightDepthBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _ssaoBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _sceneTransBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraSRVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _RSMRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _ssaoRTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _ssaoSRVHeap = nullptr;
	std::map<std::string, unsigned int> _peraViewMap;
	unsigned int peraSRVHeapNum = 0;
	unsigned int shadow_difinition = 1024;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	D3D12_RESOURCE_BARRIER RSMBuffBarrierDesc = {};
	D3D12_RESOURCE_BARRIER peraBuffBarrierDesc = {};
	D3D12_RESOURCE_BARRIER ssaoBuffBarrierDesc = {};
	D3D12_RESOURCE_BARRIER backBuffBarrierDesc = {};

	struct SceneTransMatrix {
		DirectX::XMMATRIX view;//ビュー
		DirectX::XMMATRIX projection;//プロジェクション
		DirectX::XMMATRIX invProjection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX shadowOffsetY;
		DirectX::XMMATRIX invShadowOffsetY;
		DirectX::XMMATRIX lightView;
		DirectX::XMMATRIX lightCamera;
		DirectX::XMFLOAT3 lightVec;
		DirectX::XMFLOAT3 eye;
	};
	SceneTransMatrix* _sceneTransMatrix;

	DirectX::XMFLOAT3 lightVec;
	DirectX::XMFLOAT3 eye;
	DirectX::XMFLOAT3 rotate;
	DirectX::XMFLOAT3 target;
	DirectX::XMFLOAT3 up;

	void FadeoutUpdate();
	bool CreatePeraRTVAndSRV();
	bool RSMBuffInit();
	bool DepthBuffInit();
	bool CreateDepthHeap();
	bool CreateDepthView();
	bool LightDepthBuffInit();
	bool CreateSSAOBuff();
	bool CreateSSAOHeap();
	bool CreateSSAORTVAndSRV();
	bool SceneTransBuffInit();
	void CalcSceneTrans();


	
	void BeginDrawShade();
	void EndDrawShade();
	void BeginDrawTeapot();
	void EndDrawTeapot();
	void BeginDrawSSAO();
	void EndDrawSSAO();
	void BeginDrawPera();
	void EndDrawPera();
	
	void Draw();
	void Update();

public:
	TitleScene(SceneManager& controller);
	~TitleScene();

	SceneManager& _controller;
	void SceneUpdate(void);
	bool SceneInit(void);
	void SceneFinal(void);
	void SceneRender(void);
	const char* GetSceneName(void);

	DirectX::XMFLOAT3* GetEyePos();
	DirectX::XMFLOAT3* GetTargetPos();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetSceneTransBuff();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetLightDepthBuff();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetPeraSRVHeap() const;

};


