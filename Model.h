#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <assimp/scene.h>




class Wrapper;
class Model
{
private:
	std::shared_ptr<Wrapper> _dx;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	unsigned int vertexNum = 0;
	unsigned int numIndex = 0;

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImage = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> texBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _texHeap = nullptr;
	D3D12_INDEX_BUFFER_VIEW texView = {};

	std::uint16_t VERSION = 100;

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
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT4 weights;
		unsigned short indices[4];
	};

	struct SMaterial {
		std::string albedoMapFileName;			//アルベドマップのファイル名。
		std::string normalMapFileName;			//法線マップのファイル名。
		std::string specularMapFileName;		//スペキュラマップのファイル名。
		std::string reflectionMapFileName;		//リフレクションマップのファイル名。
		std::string refractionMapFileName;		//屈折マップのファイル名。
		std::unique_ptr<char[]>	albedoMap;		//ロードされたアルベドマップ。(ddsファイル)
		unsigned int albedoMapSize;				//アルベドマップのサイズ。(ddsファイル)
		std::unique_ptr<char[]>	normalMap;		//ロードされた法線マップ。(ddsファイル)
		unsigned int normalMapSize;				//法線マップのサイズ。
		std::unique_ptr<char[]>	specularMap;	//ロードされたスペキュラマップ。(ddsファイル)
		unsigned int specularMapSize;			//スペキュラマップのサイズ。(ddsファイル)
		std::unique_ptr<char[]>	reflectionMap;	//ロードされたリフレクションマップ。(ddsファイル)
		unsigned int reflectionMapSize;			//リフレクションマップのサイズ。(ddsファイル)
		std::unique_ptr<char[]>	refractionMap;	//ロードされた屈折マップ。(ddsファイル)
		unsigned int refractionMapSize;			//屈折マップのサイズ。(ddsファイル)
		std::string albedoMapFilePath;			//アルベドマップのファイルパス。
		std::string normalMapFilePath;			//法線マップのファイルパス。
		std::string specularMapFilePath;		//スペキュラマップのファイルパス。
		std::string reflectionMapFilePath;		//リフレクションマップのファイルパス。
		std::string refractionMapFilePath;		//屈折マップのファイルパス。
	};

	struct SIndexBuffer32 {
		std::vector< uint32_t > indices;	//インデックス。
	};

	struct SIndexbuffer16 {
		std::vector< uint16_t > indices;	//インデックス。
	};

	struct SMesh
	{
		bool isFlatShading;
		std::vector<SMaterial> materials;
		std::vector<SVertex> vertexBuffer;
		std::vector<SIndexBuffer32> indexBuffer32Array;		
		std::vector<SIndexbuffer16> indexBuffer16Array;
	};

	std::vector< SMesh>	m_meshParts;

	Microsoft::WRL::ComPtr<ID3D12Resource> _mTransBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _mTransHeap = nullptr;
	unsigned int mTransHeapNum = 0;

	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX* mTransMatrix;
	float angle = 0.0f;

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

	struct Material
	{
		DirectX::XMFLOAT3 Diffuse;
		DirectX::XMFLOAT3 Specular;
		float Alpha;
		float Shininess;
		//std::string DiffuseMap;
	};
	std::vector<Material> Materials;

	struct Mesh
	{
		std::vector<MeshVertex> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t MaterialId;
	};
	std::vector<Mesh> Meshes;

	Microsoft::WRL::ComPtr<ID3D12Resource> _materialBuff = nullptr;

	template<class T>
	void LoadIndexBuffer(std::vector<T>& indices, int numIndex, FILE* fp);
	std::string LoadTextureFileName(FILE* fp);
	void BuildMaterial(SMaterial& tkmMat, FILE* fp, std::string filePath);
	bool VertexInit();
	bool IndexInit();
	bool TextureInit();
	bool MTransBuffInit();
	bool MaterialBuffInit();
	void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);
	void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);
	
public:
	bool Load(std::string filePath);
	bool LoadModel(std::string filePath);
	Model(std::shared_ptr<Wrapper> dx);
	bool Init();
	void Update();
	void Draw(bool isShadow);
	void Move(float x, float y, float z);
	void Rotate(float x, float y, float z);
	DirectX::XMFLOAT3* GetPos();
	~Model();
};
