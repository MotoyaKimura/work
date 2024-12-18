#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <assimp/scene.h>

struct aabb
{
	float _xMin;
	float _xMax;
	float _yMin;
	float _yMax;
	float _zMin;
	float _zMax;
};

struct MeshVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 MorphPosition;
	DirectX::XMFLOAT4 MorphUV;
	DirectX::XMFLOAT3 SdefC;
	DirectX::XMFLOAT3 SdefR0;
	DirectX::XMFLOAT3 SdefR1;
	DirectX::XMFLOAT4 Q0;
	DirectX::XMFLOAT4 Q1;
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
		DirectX::XMFLOAT3 const& sdefC,
		DirectX::XMFLOAT3 const& sdefR0,
		DirectX::XMFLOAT3 const& sdefR1,
		DirectX::XMFLOAT4 const& q0,
		DirectX::XMFLOAT4 const& q1,
		int boneNo0, int boneNo1, int boneNo2, int boneNo3,
		float boneWeight0, float boneWeight1, float boneWeight2, float boneWeight3,
		unsigned char weightType
	) : Position(position),
		Normal(normal),
		TexCoord(texcoord),
		MorphPosition(morphPosition),
		MorphUV(morphUV),
		SdefC(sdefC),
		SdefR0(sdefR0),
		SdefR1(sdefR1),
		Q0(q0),
		Q1(q1),
		boneNo{ boneNo0, boneNo1, boneNo2, boneNo3 },
		boneWeight{ boneWeight0, boneWeight1, boneWeight2, boneWeight3 },
		weightType(weightType)
	{}
};


struct Mesh
{
	std::vector<MeshVertex> Vertices;
	std::vector<uint32_t> Indices;
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
	std::vector <DirectX::XMMATRIX> invBoneMatrices;

	DirectX::XMFLOAT3 _pos;
	DirectX::XMFLOAT3 _rotater;
	int boneMatricesNum = 0;
	char* materialMap = nullptr;

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
	

	bool VertexInit();
	bool IndexInit();
	bool TransBuffInit();
	bool MaterialBuffInit();
	virtual bool ModelHeapInit() = 0;
	bool CreateBuffer (Microsoft::WRL::ComPtr<ID3D12Resource>& buffer,  int width);

	
	aabb _aabb;
public:
	bool Init();
	
	virtual bool Load() = 0;
	virtual void Update(bool isStart) = 0;
	virtual void Draw() = 0;
	void SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer);
	void SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format);
	bool SetViews();
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
	DirectX::XMFLOAT3 GetAABBCenter() { return DirectX::XMFLOAT3(
		(_aabb._xMax + _aabb._xMin) / 2, (_aabb._yMax + _aabb._yMin) / 2, (_aabb._zMax + _aabb._zMin) / 2); }
	void SetAABB(float xMax, float xMin, float yMax, float yMin, float zMax, float zMin)
	{
		_aabb._xMax = xMax;
		_aabb._xMin = xMin;
		_aabb._yMax = yMax;
		_aabb._yMin = yMin;
		_aabb._zMax = zMax;
		_aabb._zMin = zMin;
	}
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const { return _modelHeap; }
	Model(std::shared_ptr<Wrapper> dx, std::shared_ptr<Camera> camera, std::string filePath);
	virtual ~Model();
};
