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
		float pos[3];
		float normal[3];
		float uv[2];
		float weights[4];
		std::int16_t indices[4];
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

	template<class T>
	void LoadIndexBuffer(std::vector<T>& indices, int numIndex, FILE* fp);
	std::string LoadTextureFileName(FILE* fp);
	void BuildMaterial(SMaterial& tkmMat, FILE* fp, std::string filePath);
	bool LoadModel(std::string filePath);

	
public:
	Model(std::shared_ptr<Wrapper> dx);
	bool Init(std::string filePath);
	void Update();
	void Draw();
	~Model();
};
