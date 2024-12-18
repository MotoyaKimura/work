#include "VMD.h"
#include <fstream>
#include <iostream>
#include "Wrapper.h"
#include "Application.h"

//モーションクラス
VMD::VMD()
{
}

VMD::~VMD()
{
}

//JIS文字列をwstringに変換
void VMD::ReadJISToWString(std::ifstream& _file, std::wstring& output, size_t length)
{
	char* charStr = new char[length];
	_file.read(reinterpret_cast<char*>(charStr), length);
	std::string jisString = charStr;
	output = Application::_dx->GetWideStringFromString(jisString);
	delete[] charStr;
}

//VMDファイルの読み込み
bool VMD::LoadVMD(std::wstring filePath)
{
	if (filePath.empty()) return false;
	std::ifstream vmdFile{ filePath, (std::ios::binary | std::ios::in) };
	if (vmdFile.fail()) return false;
	if (!ReadHeader(vmdData, vmdFile)) return false;
	if (!ReadMotion(vmdData, vmdFile)) return false;
	if (!ReadMorph(vmdData, vmdFile)) return false;
	if (!ReadCamera(vmdData, vmdFile)) return false;
	if (!ReadLight(vmdData, vmdFile)) return false;
	if (!ReadShadow(vmdData, vmdFile)) return false;
	if (!ReadIK(vmdData, vmdFile)) return false;
	vmdFile.close();
	return true;
}

//VMDファイルのヘッダー読み込み
bool VMD::ReadHeader(VMDFileData& data, std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&data.header.header), 30);
	std::string headerStr = data.header.header;

	if (headerStr != "Vocaloid Motion Data 0002" && 
		headerStr != "Vocaloid Motion Data")
	{
		Application::DebugOutputFormatString("VMDファイルのヘッダーが不正です\n");
		return false;
	}
	file.read(reinterpret_cast<char*>(&data.header.modelName), 20);
	return true;
}

//モーションデータの読み込み
bool VMD::ReadMotion(VMDFileData& data, std::ifstream& file)
{
	unsigned int count = 0;
	file.read(reinterpret_cast<char*>(&count), sizeof(unsigned int));

	data.motions.resize(count);

	for (auto& motion : data.motions)
	{
		ReadJISToWString(file, motion.boneName, 15);
		file.read(reinterpret_cast<char*>(&motion.frame), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&motion.translate), sizeof(DirectX::XMFLOAT3));
		file.read(reinterpret_cast<char*>(&motion.quaternion), sizeof(DirectX::XMFLOAT4));
		file.read(reinterpret_cast<char*>(&motion.interpolation), 64);
	}
	return true;
}

//モーフデータの読み込み
bool VMD::ReadMorph(VMDFileData& data, std::ifstream& file)
{
	unsigned int count = 0;
	file.read(reinterpret_cast<char*>(&count), sizeof(unsigned int));
	data.morphs.resize(count);

	for (auto& morph : data.morphs)
	{
		ReadJISToWString(file, morph.blendShapeName, 15);
		file.read(reinterpret_cast<char*>(&morph.frame), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&morph.weight), sizeof(float));
	}
	return true;
}

//カメラデータの読み込み
bool VMD::ReadCamera(VMDFileData& data, std::ifstream& file)
{
	unsigned int count = 0;
	file.read(reinterpret_cast<char*>(&count), sizeof(unsigned int));
	data.cameras.resize(count);

	for (auto& camera : data.cameras)
	{
		file.read(reinterpret_cast<char*>(&camera.frame), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&camera.distance), sizeof(float));
		file.read(reinterpret_cast<char*>(&camera.interest), sizeof(DirectX::XMFLOAT3));
		file.read(reinterpret_cast<char*>(&camera.rotate), sizeof(DirectX::XMFLOAT3));
		file.read(reinterpret_cast<char*>(&camera.interpolation), 24);
		file.read(reinterpret_cast<char*>(&camera.fov), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&camera.isPerspective), sizeof(unsigned char));
	}
	return true;
}

//ライトデータの読み込み
bool VMD::ReadLight(VMDFileData& data, std::ifstream& file)
{
	unsigned int count = 0;
	file.read(reinterpret_cast<char*>(&count), sizeof(unsigned int));
	data.lights.resize(count);

	for (auto& light : data.lights)
	{
		file.read(reinterpret_cast<char*>(&light.frame), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&light.color), sizeof(DirectX::XMFLOAT3));
		file.read(reinterpret_cast<char*>(&light.position), sizeof(DirectX::XMFLOAT3));
	}
	return true;
}

//シャドウデータの読み込み
bool VMD::ReadShadow(VMDFileData& data, std::ifstream& file)
{
	unsigned int count = 0;
	file.read(reinterpret_cast<char*>(&count), sizeof(unsigned int));
	data.shadows.resize(count);

	for (auto& shadow : data.shadows)
	{
		file.read(reinterpret_cast<char*>(&shadow.frame), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&shadow.shadowType), sizeof(unsigned char));
		file.read(reinterpret_cast<char*>(&shadow.distance), sizeof(float));
	}
	return true;
}

//IKデータの読み込み
bool VMD::ReadIK(VMDFileData& data, std::ifstream& file)
{
	unsigned int count = 0;
	file.read(reinterpret_cast<char*>(&count), sizeof(unsigned int));
	data.iks.resize(count);

	for (auto& ik : data.iks)
	{
		file.read(reinterpret_cast<char*>(&ik.frame), sizeof(unsigned int));
		file.read(reinterpret_cast<char*>(&ik.show), sizeof(unsigned char));

		unsigned int ikInfoCount = 0;
		file.read(reinterpret_cast<char*>(&ikInfoCount), sizeof(unsigned int));
		ik.ikInfos.resize(ikInfoCount);

		for (auto& ikInfo : ik.ikInfos)
		{
			ReadJISToWString(file, ikInfo.name, 20);
			file.read(reinterpret_cast<char*>(&ikInfo.enable), sizeof(unsigned char));
		}
	}
	return true;
}
