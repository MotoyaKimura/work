#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>



class Wrapper;
class Model
{
private:
	std::shared_ptr<Wrapper> _dx;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	
public:
	Model(std::shared_ptr<Wrapper> dx);
	bool Init();
	void Load();
	void Update();
	void Draw();
	~Model();
};
