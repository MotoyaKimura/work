#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

class Wrapper;
class Renderer
{
private:
	Wrapper& _dx;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errBlob = nullptr;

	ID3D12PipelineState* _pipelinestate = nullptr;
	ID3D12RootSignature* rootsignature = nullptr;
	
public:
	Renderer(Wrapper& dx);
	void Init();
	void Update();
	void Draw();
	~Renderer();
};
