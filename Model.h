#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>



class Wrapper;
class Model
{
private:
	std::shared_ptr<Wrapper> _dx;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	unsigned int vertexNum;
	unsigned int numIndex;

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
		std::string albedoMapFileName;			//�A���x�h�}�b�v�̃t�@�C�����B
		std::string normalMapFileName;			//�@���}�b�v�̃t�@�C�����B
		std::string specularMapFileName;		//�X�y�L�����}�b�v�̃t�@�C�����B
		std::string reflectionMapFileName;		//���t���N�V�����}�b�v�̃t�@�C�����B
		std::string refractionMapFileName;		//���܃}�b�v�̃t�@�C�����B
		std::unique_ptr<char[]>	albedoMap;		//���[�h���ꂽ�A���x�h�}�b�v�B(dds�t�@�C��)
		unsigned int albedoMapSize;				//�A���x�h�}�b�v�̃T�C�Y�B(dds�t�@�C��)
		std::unique_ptr<char[]>	normalMap;		//���[�h���ꂽ�@���}�b�v�B(dds�t�@�C��)
		unsigned int normalMapSize;				//�@���}�b�v�̃T�C�Y�B
		std::unique_ptr<char[]>	specularMap;	//���[�h���ꂽ�X�y�L�����}�b�v�B(dds�t�@�C��)
		unsigned int specularMapSize;			//�X�y�L�����}�b�v�̃T�C�Y�B(dds�t�@�C��)
		std::unique_ptr<char[]>	reflectionMap;	//���[�h���ꂽ���t���N�V�����}�b�v�B(dds�t�@�C��)
		unsigned int reflectionMapSize;			//���t���N�V�����}�b�v�̃T�C�Y�B(dds�t�@�C��)
		std::unique_ptr<char[]>	refractionMap;	//���[�h���ꂽ���܃}�b�v�B(dds�t�@�C��)
		unsigned int refractionMapSize;			//���܃}�b�v�̃T�C�Y�B(dds�t�@�C��)
		std::string albedoMapFilePath;			//�A���x�h�}�b�v�̃t�@�C���p�X�B
		std::string normalMapFilePath;			//�@���}�b�v�̃t�@�C���p�X�B
		std::string specularMapFilePath;		//�X�y�L�����}�b�v�̃t�@�C���p�X�B
		std::string reflectionMapFilePath;		//���t���N�V�����}�b�v�̃t�@�C���p�X�B
		std::string refractionMapFilePath;		//���܃}�b�v�̃t�@�C���p�X�B
	};

	struct SIndexBuffer32 {
		std::vector< uint32_t > indices;	//�C���f�b�N�X�B
	};

	struct SIndexbuffer16 {
		std::vector< uint16_t > indices;	//�C���f�b�N�X�B
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

	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX* mTransMatrix;
	float angle = 0.0f;

	template<class T>
	void LoadIndexBuffer(std::vector<T>& indices, int numIndex, FILE* fp);
	std::string LoadTextureFileName(FILE* fp);
	void BuildMaterial(SMaterial& tkmMat, FILE* fp, std::string filePath);
	bool LoadModel(std::string filePath);
	bool VertexInit();
	bool IndexInit();
	bool TextureInit();
	bool MTransBuffInit();
	
public:
	Model(std::shared_ptr<Wrapper> dx);
	bool Init(std::string filePath);
	void Update();
	void Draw();
	~Model();
};
