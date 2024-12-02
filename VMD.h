#pragma once
#include <d3dx12.h>
#include <DirectXTex.h>

class VMD
{
private:
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

	VMDFileData vmdData;

	bool LoadVMD(std::string filePath);
	bool ReadHeader(VMDFileData& data, std::ifstream& file);
	bool ReadMotion(VMDFileData& data, std::ifstream& file);
	bool ReadMorph(VMDFileData& data, std::ifstream& file);
	bool ReadCamera(VMDFileData& data, std::ifstream& file);
	bool ReadLight(VMDFileData& data, std::ifstream& file);
	bool ReadShadow(VMDFileData& data, std::ifstream& file);
	bool ReadIK(VMDFileData& data, std::ifstream& file);

	void ReadJISToWString(std::ifstream& _file, std::wstring& output, size_t length);

public:
	VMD();
	~VMD();
};
