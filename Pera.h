#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>


class Wrapper;
class Pera
{
private:

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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _peraHeaps = nullptr;
	std::vector<std::pair< Microsoft::WRL::ComPtr<ID3D12Resource>, DXGI_FORMAT>> srvBuffs;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> cbvBuffs;

	bool VertexInit();
	bool HeapInit();
public:
	void SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format);
	void SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer);
	void SetViews();
	Pera(std::shared_ptr<Wrapper> dx);
	bool Init();
	void Draw() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const{ return _peraHeaps.Get(); }
	size_t GetSrvDescs() const { return srvBuffs.size(); }
	size_t GetCbvDescs() const { return cbvBuffs.size(); }
	~Pera();
};