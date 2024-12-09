#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <assimp/scene.h>
#include <array>

struct aabb
{
	float _xMin;
	float _xMax;
	float _yMin;
	float _yMax;
	float _zMin;
	float _zMax;
};

class Camera;
class Wrapper;
class Model
{
protected:
	unsigned int vertexNum = 0;
	unsigned int numIndex = 0;
	std::string _filePath;
	std::string _ext;
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Camera> _camera;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> _worldBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _invTransBuff = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> _materialBuff = nullptr;
	std::vector<std::pair< Microsoft::WRL::ComPtr<ID3D12Resource>, DXGI_FORMAT>> srvBuffs;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> cbvBuffs;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _modelHeap = nullptr;
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX* worldMatrix = nullptr;
	DirectX::XMMATRIX* invTransMatrix = nullptr;
	std::vector <DirectX::XMMATRIX> boneMatrices;

	DirectX::XMFLOAT3 _pos;
	DirectX::XMFLOAT3 _rotater;
	int boneMatricesNum = 0;
	char* materialMap = nullptr;

	struct MeshVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT3 MorphPosition;
		DirectX::XMFLOAT4 MorphUV;
		int boneNo[4];
		float boneWeight[4];
		unsigned char weightType;

		MeshVertex() = default;
		MeshVertex(
			DirectX::XMFLOAT3 const& position,
			DirectX::XMFLOAT3 const& normal,
			DirectX::XMFLOAT2 const& texcoord,
			DirectX::XMFLOAT3 const& morphPosition,
			DirectX::XMFLOAT4 const& morphUV,
			int boneNo0, int boneNo1, int boneNo2, int boneNo3,
			float boneWeight0, float boneWeight1, float boneWeight2, float boneWeight3,
			unsigned char weightType
		) : Position(position),
			Normal(normal),
			TexCoord(texcoord),
			MorphPosition(morphPosition),
			MorphUV(morphUV),
			boneNo{boneNo0, boneNo1, boneNo2, boneNo3},
			boneWeight{ boneWeight0, boneWeight1, boneWeight2, boneWeight3 },
			weightType(weightType)
		{}
	};

	struct Mesh
	{
		std::vector<MeshVertex> Vertices;
		std::vector<uint32_t> Indices;
	};
	Mesh mesh;

	MeshVertex* vertMap = nullptr;

	struct Material
	{
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT3 specular;
		float specularPower;
		DirectX::XMFLOAT3 ambient;
	};
	std::vector<Material> Materials;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mTextureResources;
	std::vector <Microsoft::WRL::ComPtr<ID3D12Resource>> mToonResources;
	std::vector <Microsoft::WRL::ComPtr<ID3D12Resource>> mSphereTextureResources;

	DirectX::XMVECTOR _eye = DirectX::XMVectorSet(0.0f,0.0f,-1.0f,0.0f);
	
	virtual bool Load(std::string filePath) = 0;

	bool VertexInit();
	bool IndexInit();
	bool WorldBuffInit();
	bool MaterialBuffInit();
	virtual bool ModelHeapInit() = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer ( int width, size_t num);

	
	aabb _aabb;
public:
	bool Init();
	bool RendererInit();
	virtual void Update() = 0;
	virtual void Draw() = 0;
	void SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer);
	void SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format);
	void SetViews();
	void Move(float x, float y, float z);
	void Rotate(float x, float y, float z);
	DirectX::XMFLOAT3* GetPos() { return &_pos; }
	DirectX::XMFLOAT3* GetRotate() { return &_rotater; }
	size_t GetSrvDescs() const { return srvBuffs.size(); }
	size_t GetCbvDescs() const { return cbvBuffs.size(); }
	std::string GetExt() const { return _ext; }
	DirectX::XMVECTOR GetEye() const { return _eye; }
	void SetEye(DirectX::XMVECTOR eye) { _eye = eye; }
	aabb* GetAABB()  { return &_aabb; }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const { return _modelHeap; }
	Model(std::shared_ptr<Wrapper> dx, std::shared_ptr<Camera> camera, std::string filePath);
	virtual ~Model();
};
