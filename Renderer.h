#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

class Wrapper;
class Pera;
class Model;
class Keyboard;
class Renderer
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Keyboard> _keyboard;

	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _teapotPipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> teapotRootsignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _peraPipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> peraRootsignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _shadowPipelinestate = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC teapotGpipeline = {};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC peraGpipeline = {};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _ssaoPipelinestate = nullptr;

	int modelID  = 0;


	bool CheckResult(HRESULT result);
	bool CompileShaderFile(std::wstring hlslFile, std::string EntryPoint, std::string model, Microsoft::WRL::ComPtr<ID3DBlob>& _xsBlob);
	bool TeapotRootSignatureInit();
	bool TeapotPipelineStateInit();
	bool PeraRootSignatureInit();
	bool PeraPipelineStateInit();
	bool ShadowPipelineStateInit();
	bool SSAOPipelineStateInit();
	
public:
	Renderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard);
	bool Init();
	void AddModel(std::shared_ptr<Model> model);
	void Move();
	void Update();
	void BeforeDrawTeapot();
	void BeforeDrawShade();
	void DrawTeapot();
	void DrawShade();
	void BeforeDrawSSAO();
	void DrawSSAO();
	void BeforeDrawPera();
	void DrawPera();
	~Renderer();
};
