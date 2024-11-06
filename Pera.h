#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>


class Wrapper;
class TitleScene;
class Pera
{
private:
	TitleScene& _title;
	struct PeraVertex {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	PeraVertex pv[4] =
	{
	{{-1, -1, 0.1}, {0, 1}},
	{{-1, 1, 0.1}, {0, 0}},
	{{1, -1, 0.1}, {1, 1}},
	{{1, 1, 0.1}, {1, 0}}
	};

	std::shared_ptr<Wrapper> _dx;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	bool VertexInit();

public:
	Pera(std::shared_ptr<Wrapper> dx, TitleScene& title);
	bool Init();
	void Draw();
	~Pera();
};