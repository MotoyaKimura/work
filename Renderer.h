#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

class Wrapper;
class Model;
class Renderer
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::vector<std::shared_ptr<Model>> _models;
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	bool CheckResult(HRESULT result);
	bool CompileShaderFile(std::wstring hlslFile, std::string EntryPoint, std::string model, Microsoft::WRL::ComPtr<ID3DBlob>& _xsBlob);
	bool RootSignatureInit();
	bool PipelineStateInit();
	
public:
	Renderer(std::shared_ptr<Wrapper> dx);
	bool Init();
	void AddModel(std::shared_ptr<Model> model);
	void Update();
	void BeforeDraw();
	void Draw();
	~Renderer();
};
