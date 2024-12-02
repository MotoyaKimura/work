#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <assimp/scene.h>
#include <array>

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
		) : Position(position),
			Normal(normal),
			TexCoord(texcoord),
			Tangent(tangent)
		{}
	};

	struct Mesh
	{
		std::vector<MeshVertex> Vertices;
		std::vector<uint32_t> Indices;
	};
	Mesh mesh;

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

	//ここからVMDファイルの読み込みに必要な構造体
	struct VMDHeader
	{
		char header[30];
		char modelName[20];
	};

	struct VMDMotion
	{
		std::wstring boneName;
		unsigned int frame;
		DirectX::XMFLOAT3 translate;
		DirectX::XMFLOAT4 quaternion;
		unsigned char interpolation[64];
	};

	struct VMDMorph
	{
		std::wstring blendShapeName;
		unsigned int frame;
		float weight;
	};

	struct VMDCamera
	{
		unsigned int frame;
		float distance;
		DirectX::XMFLOAT3 interest;
		DirectX::XMFLOAT3 rotate;
		unsigned char interpolation[24];
		unsigned int fov;
		unsigned char isPerspective;
	};

	struct VMDLight
	{
		unsigned int frame;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 position;
	};

	struct VMDShadow
	{
		unsigned int frame;
		unsigned char shadowType;
		float distance;
	};

	struct VMDIKInfo
	{
		std::wstring name;
		unsigned char enable;
	};

	struct VMDIK
	{
		unsigned int frame;
		unsigned char show;
		std::vector<VMDIKInfo> ikInfos;
	};

	struct VMDFileData
	{
		VMDHeader header;
		std::vector<VMDMotion> motions;
		std::vector<VMDMorph> morphs;
		std::vector<VMDCamera> cameras;
		std::vector< VMDLight> lights;
		std::vector<VMDShadow> shadows;
		std::vector<VMDIK> iks;
	};

	//ここまで
	
	virtual bool Load(std::string filePath) = 0;

	

	bool LoadVMD(std::string filePath);

	bool VertexInit();
	bool IndexInit();
	bool WorldBuffInit();
	bool MaterialBuffInit();
	virtual bool ModelHeapInit() = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer ( int width, size_t num);
public:
	bool Init();
	bool RendererInit();
	void Update();
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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const { return _modelHeap; }
	Model(std::shared_ptr<Wrapper> dx, std::shared_ptr<Camera> camera, std::string filePath);
	virtual ~Model();
};
