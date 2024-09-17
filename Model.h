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



	struct SHeader
	{
		std::uint8_t version;
		std::uint8_t isFlatShading;
		std::uint16_t numMeshParts;
	};

	struct SMeshePartsHeader
	{
		std::uint32_t numMaterial;
		std::uint32_t numVertex;
		std::uint8_t indexSize;
		std::uint8_t pad[3];
	};

	struct SVertex
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float weights[4];
		std::int16_t indices[4];
	};
	
public:
	Model(std::shared_ptr<Wrapper> dx);
	bool Init();
	void Load();
	void Update();
	void Draw();
	~Model();
};
