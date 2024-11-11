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
protected:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::vector<std::shared_ptr<Model>> _models;
	std::shared_ptr<Keyboard> _keyboard;

	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipelineDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
	std::vector< CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_ROOT_PARAMETER rootParam;
	std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers;
	int modelID  = 0;
	float clsClr[4];

	UINT _numBuffers;
	UINT resWidth;
	UINT resHeight;
	DXGI_FORMAT _format;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _buffers;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr;
	void SetNumBuffers(UINT num) { _numBuffers = num; }
	void SetResSize(UINT width, UINT height) { resWidth = width; resHeight = height; }
	void SetFormat(DXGI_FORMAT format) { _format = format; }
	void SetClearValue(float r, float g, float b, float a) { clsClr[0] = r; clsClr[1] = g; clsClr[2] = b; clsClr[3] = a; }
	void AddElement(const char* semantics, DXGI_FORMAT format);

	bool CheckResult(HRESULT result);
	bool CompileShaderFile(std::wstring hlslFile, std::string EntryPoint, std::string model, Microsoft::WRL::ComPtr<ID3DBlob>& _xsBlob);
	bool RootSignatureInit();
	bool PipelineStateInit();
	void SetBarrierState(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> const& buffers, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const;
	void SetBarrierState(Microsoft::WRL::ComPtr<ID3D12Resource> const& buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const;
	void SetRenderTargets(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap, bool isBackBuffer);
	void SetVPAndSR(UINT windowWidth, UINT windowHeight);
	void SetSRVDesc(DXGI_FORMAT format);
	
public:
	void SetRTsToHeapAsSRV(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs);
	bool CreateBuffers();
	bool CreateDepthBuffer();
	Renderer(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera, std::shared_ptr<Keyboard> _keyboard);
	virtual bool Init() = 0;
	virtual void Draw() = 0;
	virtual void SetRootSigParam() = 0;
	void AddModel(std::shared_ptr<Model> model);
	void Move();
	void Update();
	void BeforeDraw(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelinestate, Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature);
	void DrawModel();
	void DrawPera();
	void ResizeBuffers();
	virtual ~Renderer();
};
