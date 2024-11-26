#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <assimp/scene.h>

class Camera;
class Wrapper;
class Model
{
private:
	unsigned int vertexNum = 0;
	unsigned int numIndex = 0;
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Camera> _camera;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> _worldBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _materialBuff = nullptr;
	std::vector<std::pair< Microsoft::WRL::ComPtr<ID3D12Resource>, DXGI_FORMAT>> srvBuffs;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> cbvBuffs;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _modelHeap = nullptr;
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX* worldMatrix;
	DirectX::XMFLOAT3 _pos;
	DirectX::XMFLOAT3 _rotater;

	struct MeshVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT3 Tangent;

		MeshVertex() = default;
		MeshVertex(
			DirectX::XMFLOAT3 const& position,
			DirectX::XMFLOAT3 const& normal,
			DirectX::XMFLOAT2 const& texcoord,
			DirectX::XMFLOAT3 const& tangent
		): Position(position),
		Normal(normal),
		TexCoord(texcoord),
		Tangent(tangent)
		{}
	};
	struct Mesh
	{
		std::vector<MeshVertex> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t MaterialId;
	};
	std::vector<Mesh> Meshes;

	struct Material
	{
		DirectX::XMFLOAT3 Diffuse;
		DirectX::XMFLOAT3 Specular;
		float Alpha;
		float Shininess;
	};
	std::vector<Material> Materials;
	
	bool Load(std::string filePath);
	void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);
	void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);
	bool VertexInit();
	bool IndexInit();
	bool WorldBuffInit();
	bool MaterialBuffInit();
	bool ModelHeapInit();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer ( UINT64 width);
public:
	bool Init();
	bool RendererInit();
	void Update();
	void Draw();
	void SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer);
	void SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format);
	void SetViews();
	void Move(float x, float y, float z);
	void Rotate(float x, float y, float z);
	DirectX::XMFLOAT3* GetPos() { return &_pos; }
	DirectX::XMFLOAT3* GetRotate() { return &_rotater; }
	size_t GetSrvDescs() const { return srvBuffs.size(); }
	size_t GetCbvDescs() const { return cbvBuffs.size(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const { return _modelHeap; }
	Model(std::shared_ptr<Wrapper> dx, std::shared_ptr<Camera> camera, std::string filePath);
	~Model();
};
