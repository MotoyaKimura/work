#include "Model.h"
#include <iostream>
#include <fstream>
#include "Camera.h"
#include "Wrapper.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma  comment(lib, "DirectXTex.lib")
#pragma  comment(lib, "assimp-vc143-mtd.lib")

using namespace DirectX;

std::wstring GetWideStringFromString(const std::string& str)
{
	auto num1 = MultiByteToWideChar(
		CP_ACP, 
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, 
		str.c_str(), 
		-1, 
		nullptr, 
		0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0],
		num1);

	assert(num1 == num2);
	return wstr;
}

std::string GetStringFromWideString(const std::wstring& wstr)
{
	auto num1 = WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr.c_str(),
		-1,
		nullptr,
		0,
		nullptr,
		nullptr);

	std::string str;
	str.resize(num1);

	auto num2 = WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr.c_str(),
		-1,
		&str[0],
		num1,
		nullptr,
		nullptr);

	assert(num1 == num2);
	return str;
}

bool Model::Load(std::string filePath)
{
	if (filePath == "") return false;

	_ext = filePath.substr(filePath.find_last_of('.') + 1);
	if(_ext == "pmx")
	{
		LoadPMX(filePath);
	}
	else
	{
		LoadByAssimp(filePath);
	}

	return true;
}

bool Model::LoadPMX(std::string filePath)
{

	if (filePath.empty()) return false;
	std::ifstream pmxFile{ filePath, (std::ios::binary | std::ios::in) };
	if(pmxFile.fail()) return false;
	if (!ReadHeader(pmxData, pmxFile)) return false;
	if (!ReadModelInfo(pmxData, pmxFile)) return false;
	if (!ReadVertex(pmxData, pmxFile)) return false;
	if (!ReadFace(pmxData, pmxFile)) return false;
	if (!ReadTexture(pmxData, pmxFile)) return false;
	if (!ReadMaterial(pmxData, pmxFile)) return false;
	if (!ReadBone(pmxData, pmxFile)) return false;
	if (!ReadMorph(pmxData, pmxFile)) return false;
	if (!ReadDisplayFrame(pmxData, pmxFile)) return false;
	if (!ReadRigidBody(pmxData, pmxFile)) return false;
	if (!ReadJoint(pmxData, pmxFile)) return false;
	if (!ReadSoftBody(pmxData, pmxFile)) return false;
	pmxFile.close();
	return true;
}

bool Model::ReadHeader(PMXFileData& data, std::ifstream& file)
{
	constexpr std::array<unsigned char, 4> PMX_MAGIC_NUMBER = { 0x50, 0x4d, 0x58, 0x20 };
	file.read(reinterpret_cast<char*>(data.header.magic.data()), data.header.magic.size());
	if (data.header.magic != PMX_MAGIC_NUMBER)
	{
		file.close();
		return false;
	}
	file.read(reinterpret_cast<char*>(&data.header.version), sizeof(data.header.version));
	file.read(reinterpret_cast<char*>(&data.header.dataLength), sizeof(data.header.dataLength));
	file.read(reinterpret_cast<char*>(&data.header.textEncoding), sizeof(data.header.textEncoding));
	file.read(reinterpret_cast<char*>(&data.header.addUVNum), sizeof(data.header.addUVNum));

	file.read(reinterpret_cast<char*>(&data.header.vertexIndexSize), sizeof(	data.header.vertexIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.textureIndexSize), sizeof(data.header.textureIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.materialIndexSize), sizeof(data.header.materialIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.boneIndexSize), sizeof(data.header.boneIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.morphIndexSize), sizeof(	data.header.morphIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.rigidBodyIndexSize), sizeof(data.header.rigidBodyIndexSize));
	return true;
}

bool Model::GetPMXStringUTF16(std::ifstream& _file, std::wstring& output)
{
	std::array<wchar_t, 1024> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);
	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::wstring(&wBuffer[0], &wBuffer[0] + textSize / 2);

	return true;
}

bool Model::GetPMXStringUTF8(std::ifstream& _file, std::string& output)
{
	std::array<char, 1024> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);
	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::string(&wBuffer[0], &wBuffer[0] + textSize);

	return true;
}


bool Model::ReadModelInfo(PMXFileData& data, std::ifstream& file)
{
	GetPMXStringUTF16(file, data.modelInfo.modelName);
	GetPMXStringUTF8(file, data.modelInfo.englishModelName);
	GetPMXStringUTF16(file, data.modelInfo.comment);
	GetPMXStringUTF8(file, data.modelInfo.englishComment);
	return true;
}

bool Model::ReadVertex(PMXFileData& data, std::ifstream& file)
{
	unsigned int vertexCount;
	file.read(reinterpret_cast<char*>(&vertexCount), 4);
	vertexNum = vertexCount;
	data.vertices.resize(vertexCount);
	mesh.Vertices.resize(vertexCount);
	int i = 0;
	for(auto& vertex : data.vertices)
	{
		file.read(reinterpret_cast<char*>(&vertex.position), 12);
		file.read(reinterpret_cast<char*>(&vertex.normal), 12);
		file.read(reinterpret_cast<char*>(&vertex.uv), 8);

		for (int i = 0; i < data.header.addUVNum; i++)
		{
			file.read(reinterpret_cast<char*>(&vertex.additionalUV[i]), 16);
		}

		file.read(reinterpret_cast<char*>(&vertex.weightType), 1);

		const unsigned char boneIndexSize = data.header.boneIndexSize;

		switch (vertex.weightType)
		{
		case PMXVertexWeight::BDEF1:
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[0]), boneIndexSize);
			break;
		case PMXVertexWeight::BDEF2:
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[0]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[1]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneWeights[0]), 4);
			break;
		case PMXVertexWeight::BDEF4:
		case PMXVertexWeight::QDEF:
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[0]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[1]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[2]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[3]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneWeights[0]), 4);
			file.read(reinterpret_cast<char*>(&vertex.boneWeights[1]), 4);
			file.read(reinterpret_cast<char*>(&vertex.boneWeights[2]), 4);
			file.read(reinterpret_cast<char*>(&vertex.boneWeights[3]), 4);
			break;
		case PMXVertexWeight::SDEF:
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[0]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneIndices[1]), boneIndexSize);
			file.read(reinterpret_cast<char*>(&vertex.boneWeights[0]), 4);
			file.read(reinterpret_cast<char*>(&vertex.sdefC), 12);
			file.read(reinterpret_cast<char*>(&vertex.sdefR0), 12);
			file.read(reinterpret_cast<char*>(&vertex.sdefR1), 12);
			break;
		default:
			return false;
		}
		file.read(reinterpret_cast<char*>(&vertex.edgeMag), 4);
		mesh.Vertices[i].Position = DirectX::XMFLOAT3(vertex.position.x * 0.1, vertex.position.y * 0.1, vertex.position.z * 0.1);
		mesh.Vertices[i].Normal = vertex.normal;
		mesh.Vertices[i].TexCoord = vertex.uv;
		i++;
	}

	return true;
}

bool Model::ReadFace(PMXFileData& data, std::ifstream& file)
{
	int faceCount;
	file.read(reinterpret_cast<char*>(&faceCount), 4);

	faceCount /= 3;
	data.faces.resize(faceCount);
	mesh.Indices.resize(faceCount * 3);
	numIndex = faceCount * 3;
	switch (data.header.vertexIndexSize)
	{
		case 1:
		{
			std::vector<uint8_t> vertices(faceCount * 3);
			file.read(reinterpret_cast<char*>(vertices.data()), sizeof(uint8_t) * vertices.size());
			for (int32_t faceIdx = 0; faceIdx < faceCount; faceIdx++)
			{
				data.faces[faceIdx].vertices[0] = vertices[faceIdx * 3 + 0];
				data.faces[faceIdx].vertices[1] = vertices[faceIdx * 3 + 1];
				data.faces[faceIdx].vertices[2] = vertices[faceIdx * 3 + 2];
				mesh.Indices[faceIdx * 3 + 0] = vertices[faceIdx * 3 + 0];
				mesh.Indices[faceIdx * 3 + 1] = vertices[faceIdx * 3 + 1];
				mesh.Indices[faceIdx * 3 + 2] = vertices[faceIdx * 3 + 2];
			}
		}
		break;
		case 2:
		{
			std::vector<uint16_t> vertices(faceCount * 3);
			file.read(reinterpret_cast<char*>(vertices.data()), sizeof(uint16_t) * vertices.size());
			for (int32_t faceIdx = 0; faceIdx < faceCount; faceIdx++)
			{
				data.faces[faceIdx].vertices[0] = vertices[faceIdx * 3 + 0];
				data.faces[faceIdx].vertices[1] = vertices[faceIdx * 3 + 1];
				data.faces[faceIdx].vertices[2] = vertices[faceIdx * 3 + 2];
				mesh.Indices[faceIdx * 3 + 0] = vertices[faceIdx * 3 + 0];
				mesh.Indices[faceIdx * 3 + 1] = vertices[faceIdx * 3 + 1];
				mesh.Indices[faceIdx * 3 + 2] = vertices[faceIdx * 3 + 2];
			}
		}
		break;
		case 4:
		{
			std::vector<uint32_t> vertices(faceCount * 3);
			file.read(reinterpret_cast<char*>(vertices.data()), sizeof(uint32_t) * vertices.size());
			for (int32_t faceIdx = 0; faceIdx < faceCount; faceIdx++)
			{
				data.faces[faceIdx].vertices[0] = vertices[faceIdx * 3 + 0];
				data.faces[faceIdx].vertices[1] = vertices[faceIdx * 3 + 1];
				data.faces[faceIdx].vertices[2] = vertices[faceIdx * 3 + 2];
				mesh.Indices[faceIdx * 3 + 0] = vertices[faceIdx * 3 + 0];
				mesh.Indices[faceIdx * 3 + 1] = vertices[faceIdx * 3 + 1];
				mesh.Indices[faceIdx * 3 + 2] = vertices[faceIdx * 3 + 2];
			}
		}
		break;
		default:
			return false;
	}
	return true;
}

bool Model::ReadTexture(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfTexture = 0;
	file.read(reinterpret_cast<char*>(&numOfTexture), 4);
	data.textures.resize(numOfTexture);
	for (auto& texture : data.textures)
	{
		GetPMXStringUTF16(file, texture.textureName);
	}
	return true;
}

bool Model::ReadMaterial(PMXFileData& data, std::ifstream& file)
{
	int numOfMaterial = 0;
	file.read(reinterpret_cast<char*>(&numOfMaterial), 4);

	Materials.resize(numOfMaterial);
	mLoadedMaterial.resize(numOfMaterial);
	int materialIndex = 0;

	data.materials.resize(numOfMaterial);
	for(auto& mat : data.materials)
	{
		GetPMXStringUTF16(file, mat.name);
		GetPMXStringUTF8(file, mat.englishName);
		file.read(reinterpret_cast<char*>(&mat.diffuse), 16);
		file.read(reinterpret_cast<char*>(&mat.specular), 12);
		file.read(reinterpret_cast<char*>(&mat.specularPower), 4);
		file.read(reinterpret_cast<char*>(&mat.ambient), 12);

		file.read(reinterpret_cast<char*>(&mat.drawMode), 1);

		file.read(reinterpret_cast<char*>(&mat.edgeColor), 16);
		file.read(reinterpret_cast<char*>(&mat.edgeSize), 4);

		file.read(reinterpret_cast<char*>(&mat.textureIndex), data.header.textureIndexSize);
		file.read(reinterpret_cast<char*>(&mat.sphereTextureIndex), data.header.textureIndexSize);
		file.read(reinterpret_cast<char*>(&mat.sphereMode), 1);

		file.read(reinterpret_cast<char*>(&mat.toonMode), 1);
		if(mat.toonMode == PMXToonMode::Separate)
		{
			file.read(reinterpret_cast<char*>(&mat.toonTextureIndex), data.header.textureIndexSize);
		}
		else if (mat.toonMode == PMXToonMode::Common)
		{
			file.read(reinterpret_cast<char*>(&mat.toonTextureIndex), 1);
		}
		else
		{
			return false;
		}
		GetPMXStringUTF16(file, mat.memo);
		file.read(reinterpret_cast<char*>(&mat.numFaceVertices), 4);

		mLoadedMaterial[materialIndex].visible = true;
		mLoadedMaterial[materialIndex].name = GetStringFromWideString(mat.name);
		mLoadedMaterial[materialIndex].diffuse = mat.diffuse;
		mLoadedMaterial[materialIndex].specular = mat.specular;
		mLoadedMaterial[materialIndex].specularPower = mat.specularPower;
		mLoadedMaterial[materialIndex].ambient = mat.ambient;
		mLoadedMaterial[materialIndex].isTransparent = false;

		Materials[materialIndex].diffuse = mat.diffuse;
		Materials[materialIndex].specular = mat.specular;
		Materials[materialIndex].specularPower = mat.specularPower;
		Materials[materialIndex].ambient = mat.ambient;
		materialIndex++;
	}
	return true;
}

bool Model::ReadBone(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfBone;
	file.read(reinterpret_cast<char*>(&numOfBone), 4);

	data.bones.resize(numOfBone);

	for (auto& bone : data.bones)
	{
		GetPMXStringUTF16(file, bone.name);
		GetPMXStringUTF8(file, bone.englishName);

		file.read(reinterpret_cast<char*>(&bone.position), 12);
		file.read(reinterpret_cast<char*>(&bone.parentBoneIndex), data.header.boneIndexSize);
		file.read(reinterpret_cast<char*>(&bone.deformDepth), 4);

		file.read(reinterpret_cast<char*>(&bone.boneFlag), 2);

		if (((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::TargetShowMode) == 0)
		{
			file.read(reinterpret_cast<char*>(&bone.positionOffset), 12);
		}
		else
		{
			file.read(reinterpret_cast<char*>(&bone.linkBoneIndex), data.header.boneIndexSize);
		}
		
		if (((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::AppendRotate) ||
			((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::AppendTranslate))
		{
			file.read(reinterpret_cast<char*>(&bone.appendBoneIndex), data.header.boneIndexSize);
			file.read(reinterpret_cast<char*>(&bone.appendWeight), 4);
		}

		if ((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::FixedAxis)
		{
			file.read(reinterpret_cast<char*>(&bone.fixedAxis), 12);
		}

		if ((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::LocalAxis)
		{
			file.read(reinterpret_cast<char*>(&bone.localXAxis), 12);
			file.read(reinterpret_cast<char*>(&bone.localZAxis), 12);
		}

		if ((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::DeformOuterParent)
		{
			file.read(reinterpret_cast<char*>(&bone.keyValue), 4);
		}

		if ((uint16_t)bone.boneFlag & (uint16_t)PMXBoneFlags::IK)
		{
			file.read(reinterpret_cast<char*>(&bone.ikTargetBoneIndex), data.header.boneIndexSize);
			file.read(reinterpret_cast<char*>(&bone.ikIterationCount), 4);
			file.read(reinterpret_cast<char*>(&bone.ikLimit), 4);
			unsigned int linkCount = 0;
			file.read(reinterpret_cast<char*>(&linkCount), 4);

			bone.ikLinks.resize(linkCount);
			for (auto& ikLink : bone.ikLinks)
			{
				file.read(reinterpret_cast<char*>(&ikLink.ikBoneIndex), data.header.boneIndexSize);
				file.read(reinterpret_cast<char*>(&ikLink.enableLimit), 1);
				if (ikLink.enableLimit != 0)
				{
					file.read(reinterpret_cast<char*>(&ikLink.LimitMin), 12);
					file.read(reinterpret_cast<char*>(&ikLink.LimitMax), 12);
				}
			}
		}
	}
	return true;
}

bool Model::ReadMorph(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfMorph = 0;
	file.read(reinterpret_cast<char*>(&numOfMorph), 4);

	data.morphs.resize(numOfMorph);

	for(auto& morph : data.morphs)
	{
		GetPMXStringUTF16(file, morph.name);
		GetPMXStringUTF8(file, morph.englishName);

		file.read(reinterpret_cast<char*>(&morph.controlPanel), 1);
		file.read(reinterpret_cast<char*>(&morph.morphType), 1);

		unsigned int dataCount;
		file.read(reinterpret_cast<char*>(&dataCount), 4);

		if(morph.morphType == PMXMorphType::Position)
		{
			morph.positionMorph.resize(dataCount);
			for (auto& morphData : morph.positionMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.vertexIndex), data.header.vertexIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.position), 12);
			}
		}
		else if(morph.morphType == PMXMorphType::UV ||
			morph.morphType == PMXMorphType::AddUV1 || 
			morph.morphType == PMXMorphType::AddUV2 ||
			morph.morphType == PMXMorphType::AddUV3 ||
			morph.morphType == PMXMorphType::AddUV4)
		{
			morph.uvMorph.resize(dataCount);
			for (auto& morphData : morph.uvMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.vertexIndex), data.header.vertexIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.uv), 16);
			}
		}
		else if(morph.morphType == PMXMorphType::Bone)
		{
			morph.boneMorph.resize(dataCount);
			for (auto& morphData : morph.boneMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.boneIndex), data.header.boneIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.position), 12);
				file.read(reinterpret_cast<char*>(&morphData.quaternion), 16);
			}
		}
		else if(morph.morphType == PMXMorphType::Material)
		{
			morph.materialMorph.resize(dataCount);
			for(auto& morphData : morph.materialMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.materialIndex), data.header.materialIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.opType), 1);
				file.read(reinterpret_cast<char*>(&morphData.diffuse), 16);
				file.read(reinterpret_cast<char*>(&morphData.specular), 12);
				file.read(reinterpret_cast<char*>(&morphData.specularPower), 4);
				file.read(reinterpret_cast<char*>(&morphData.ambient), 12);
				file.read(reinterpret_cast<char*>(&morphData.edgeColor), 16);
				file.read(reinterpret_cast<char*>(&morphData.edgeSize), 4);
				file.read(reinterpret_cast<char*>(&morphData.textureFactor), 16);
				file.read(reinterpret_cast<char*>(&morphData.sphereTextureFactor), 16);
				file.read(reinterpret_cast<char*>(&morphData.toonTextureFactor), 16);
			}
		}
		else if(morph.morphType == PMXMorphType::Group)
		{
			morph.groupMorph.resize(dataCount);
			for (auto& morphData : morph.groupMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.morphIndex), data.header.morphIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.weight), 4);
			}
		}
		else if(morph.morphType == PMXMorphType::Flip)
		{
			morph.flipMorph.resize(dataCount);
			for (auto& morphData : morph.flipMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.morphIndex), data.header.morphIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.weight), 4);
			}
		}
		else if (morph.morphType == PMXMorphType::Impulse)
		{
			morph.impulseMorph.resize(dataCount);
			for (auto& morphData : morph.impulseMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.rigidBodyIndex), data.header.rigidBodyIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.localFlag), 1);
				file.read(reinterpret_cast<char*>(&morphData.translateVelocity), 12);
				file.read(reinterpret_cast<char*>(&morphData.rotateTorque), 12);
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool Model::ReadDisplayFrame(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfDisplayFrame = 0;
	file.read(reinterpret_cast<char*>(&numOfDisplayFrame), 4);

	data.displayFrames.resize(numOfDisplayFrame);

	for(auto& displayFlame : data.displayFrames)
	{
		GetPMXStringUTF16(file, displayFlame.name);
		GetPMXStringUTF8(file, displayFlame.englishName);

		file.read(reinterpret_cast<char*>(&displayFlame.flag), 1);

		unsigned int targetCount = 0;
		file.read(reinterpret_cast<char*>(&targetCount), 4);

		displayFlame.targets.resize(targetCount);
		for (auto& target : displayFlame.targets)
		{
			file.read(reinterpret_cast<char*>(&target.type), 1);
			if (target.type == PMXDisplayFrame::TargetType::BoneIndex)
			{
				file.read(reinterpret_cast<char*>(&target.index), data.header.boneIndexSize);
			}
			else if (target.type == PMXDisplayFrame::TargetType::MorphIndex)
			{
				file.read(reinterpret_cast<char*>(&target.index), data.header.morphIndexSize);
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

bool Model::ReadRigidBody(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfRigidBody = 0;
	file.read(reinterpret_cast<char*>(&numOfRigidBody), 4);

	data.rigidBodies.resize(numOfRigidBody);

	for (auto& rigidBody : data.rigidBodies)
	{
		GetPMXStringUTF16(file, rigidBody.name);
		GetPMXStringUTF8(file, rigidBody.englishName);

		file.read(reinterpret_cast<char*>(&rigidBody.boneIndex), data.header.boneIndexSize);
		file.read(reinterpret_cast<char*>(&rigidBody.group), 1);
		file.read(reinterpret_cast<char*>(&rigidBody.collisionGroup), 2);
		file.read(reinterpret_cast<char*>(&rigidBody.shape), 1);
		file.read(reinterpret_cast<char*>(&rigidBody.shapeSize), 12);
		file.read(reinterpret_cast<char*>(&rigidBody.translate), 12);
		file.read(reinterpret_cast<char*>(&rigidBody.rotate), 12);
		file.read(reinterpret_cast<char*>(&rigidBody.mass), 4);
		file.read(reinterpret_cast<char*>(&rigidBody.translateDimmer), 4);
		file.read(reinterpret_cast<char*>(&rigidBody.rotateDimmer), 4);
		file.read(reinterpret_cast<char*>(&rigidBody.repulsion), 4);
		file.read(reinterpret_cast<char*>(&rigidBody.friction), 4);
		file.read(reinterpret_cast<char*>(&rigidBody.op), 1);
	}
	return true;
}

bool Model::ReadJoint(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfJoint = 0;
	file.read(reinterpret_cast<char*>(&numOfJoint), 4);

	data.joints.resize(numOfJoint);

	for(auto& joint : data.joints)
	{
		GetPMXStringUTF16(file, joint.name);
		GetPMXStringUTF8(file, joint.englishName);

		file.read(reinterpret_cast<char*>(&joint.type), 1);
		file.read(reinterpret_cast<char*>(&joint.rigidBodyAIndex), data.header.rigidBodyIndexSize);
		file.read(reinterpret_cast<char*>(&joint.rigidBodyBIndex), data.header.rigidBodyIndexSize);

		file.read(reinterpret_cast<char*>(&joint.translate), 12);
		file.read(reinterpret_cast<char*>(&joint.rotate), 12);

		file.read(reinterpret_cast<char*>(&joint.translateLowerLimit), 12);
		file.read(reinterpret_cast<char*>(&joint.translateUpperLimit), 12);
		file.read(reinterpret_cast<char*>(&joint.rotateLowerLimit), 12);
		file.read(reinterpret_cast<char*>(&joint.rotateUpperLimit), 12);

		file.read(reinterpret_cast<char*>(&joint.springTranslateFactor), 12);
		file.read(reinterpret_cast<char*>(&joint.springRotateFactor), 12);
	}
	return true;
}

bool Model::ReadSoftBody(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfSoftBody = 0;
	file.read(reinterpret_cast<char*>(&numOfSoftBody), 4);

	data.softBodies.resize(numOfSoftBody);

	for(auto& softBody : data.softBodies)
	{
		GetPMXStringUTF16(file, softBody.name);
		GetPMXStringUTF8(file, softBody.englishName);

		file.read(reinterpret_cast<char*>(&softBody.type), 1);

		file.read(reinterpret_cast<char*>(&softBody.materialIndex), data.header.materialIndexSize);
		file.read(reinterpret_cast<char*>(&softBody.group), 1);
		file.read(reinterpret_cast<char*>(&softBody.collisionGroup), 2);

		file.read(reinterpret_cast<char*>( & softBody.flag), 1);

		file.read(reinterpret_cast<char*>(&softBody.bLinkLength), 4);
		file.read(reinterpret_cast<char*>(&softBody.numClusters), 4);

		file.read(reinterpret_cast<char*>(&softBody.totalMass), 4);
		file.read(reinterpret_cast<char*>(&softBody.collisionMargin), 4);

		file.read(reinterpret_cast<char*>(&softBody.aeroModel), 4);

		file.read(reinterpret_cast<char*>(&softBody.vcf), 4);
		file.read(reinterpret_cast<char*>(&softBody.dp), 4);
		file.read(reinterpret_cast<char*>(&softBody.dg), 4);
		file.read(reinterpret_cast<char*>(&softBody.lf), 4);
		file.read(reinterpret_cast<char*>(&softBody.pr), 4);
		file.read(reinterpret_cast<char*>(&softBody.vc), 4);
		file.read(reinterpret_cast<char*>(&softBody.df), 4);
		file.read(reinterpret_cast<char*>(&softBody.mt), 4);
		file.read(reinterpret_cast<char*>(&softBody.chr), 4);
		file.read(reinterpret_cast<char*>(&softBody.khr), 4);
		file.read(reinterpret_cast<char*>(&softBody.shr), 4);
		file.read(reinterpret_cast<char*>(&softBody.ahr), 4);

		file.read(reinterpret_cast<char*>(&softBody.srhr_cl), 4);
		file.read(reinterpret_cast<char*>(&softBody.skhr_cl), 4);
		file.read(reinterpret_cast<char*>(&softBody.sshr_cl), 4);
		file.read(reinterpret_cast<char*>(&softBody.sr_splt_cl), 4);
		file.read(reinterpret_cast<char*>(&softBody.sk_splt_cl), 4);
		file.read(reinterpret_cast<char*>(&softBody.ss_splt_cl), 4);

		file.read(reinterpret_cast<char*>(&softBody.v_it), 4);
		file.read(reinterpret_cast<char*>(&softBody.p_it), 4);
		file.read(reinterpret_cast<char*>(&softBody.d_it), 4);
		file.read(reinterpret_cast<char*>(&softBody.c_it), 4);

		file.read(reinterpret_cast<char*>(&softBody.lst), 4);
		file.read(reinterpret_cast<char*>(&softBody.ast), 4);
		file.read(reinterpret_cast<char*>(&softBody.vst), 4);

		unsigned int anchorCount = 0;
		file.read(reinterpret_cast<char*>(&anchorCount), 4);

		softBody.anchorRigidBodies.resize(anchorCount);
		for (auto& anchor : softBody.anchorRigidBodies)
		{
			file.read(reinterpret_cast<char*>(&anchor.rigidBodyIndex), data.header.rigidBodyIndexSize);
			file.read(reinterpret_cast<char*>(&anchor.vertexIndex), data.header.vertexIndexSize);
			file.read(reinterpret_cast<char*>(&anchor.nearMode), 1);
		}

		unsigned int pinVertexCount = 0;
		file.read(reinterpret_cast<char*>(&pinVertexCount), 4);

		softBody.pinVertexIndices.resize(pinVertexCount);
		for (auto& pinVertex : softBody.pinVertexIndices)
		{
			file.read(reinterpret_cast<char*>(&pinVertex), data.header.vertexIndexSize);
		}
	}
	return true;
}

bool Model::LoadByAssimp(std::string filePath)
{
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_ConvertToLeftHanded;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_ValidateDataStructure;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;
	flag |= aiProcess_GenBoundingBoxes;

	auto pScene = importer.ReadFile(filePath.c_str(), flag);
	if (pScene == nullptr) return false;

	auto node = pScene->mRootNode;
	auto numMeshes = node->mNumMeshes;

	
	const auto pMesh = pScene->mMeshes[0];
	ParseMesh(mesh, pMesh);
	


	Materials.clear();
	Materials.resize(pScene->mNumMaterials);

	for (size_t i = 0; i < Materials.size(); ++i)
	{
		const auto pMaterial = pScene->mMaterials[i];
		ParseMaterial(Materials[i], pMaterial);
	}

	pScene = nullptr;

	return true;
}


void Model::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh)
{
	/*aiVector3D aabbMax = pSrcMesh->mAABB.mMax;
	aiVector3D aabbMin = pSrcMesh->mAABB.mMin;
	std::cout << "aabbMax.x = " << aabbMax.x << ", aabbMax.y = " << aabbMax.y << ", aabbMax.z = " << aabbMax.z << "\n" << std::endl;*/
	aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	dstMesh.Vertices.resize(pSrcMesh->mNumVertices);
	vertexNum += pSrcMesh->mNumVertices;
	for(auto i = 0u; i < pSrcMesh->mNumVertices; ++i)
	{
		auto pPosition = &(pSrcMesh->mVertices[i]);
		auto pNormal = &(pSrcMesh->mNormals[i]);
		auto pTexCoord = pSrcMesh->HasTextureCoords(0) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;
		auto pTangent = pSrcMesh->HasTangentsAndBitangents() ? &(pSrcMesh->mTangents[i]) : &zero3D;

		dstMesh.Vertices[i] = MeshVertex(
			XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
			XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
			XMFLOAT2(pTexCoord->x, pTexCoord->y),
			XMFLOAT3(pTangent->x, pTangent->y, pTangent->z)
		);
	}

	dstMesh.Indices.resize(pSrcMesh->mNumFaces * 3);
	
	numIndex += pSrcMesh->mNumFaces * 3;
	for(auto i = 0u; i < pSrcMesh->mNumFaces; ++i)
	{
		const auto& face = pSrcMesh->mFaces[i];
		assert(face.mNumIndices == 3);

		dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
		dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
		dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
	}
}

void Model::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial)
{
	{
		aiColor4D color(0.0f, 0.0f, 0.0f, 0.0f);

		if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		{
			dstMaterial.diffuse = XMFLOAT4(color.r, color.g, color.b, color.a);
		}
		else
		{
			dstMaterial.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		}
		/*SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
		std::string name = pSrcMaterial->GetName().C_Str();
		std::cout << name << std::endl;*/

		aiString path;
		if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
		{
			std::string str = path.C_Str();
			auto num1 = MultiByteToWideChar(
				CP_UTF8,
				0U,
				str.data(),
				-1,
				nullptr,
				0U);
			std::wstring wstr(num1, L'\0');
			auto num2 = MultiByteToWideChar(
				CP_UTF8,
				0U,
				str.data(),
				-1,
				&wstr[0],
				num1);
			
		}
		else
		{
			std::cout << "no texture" << std::endl;
		}
	}

	{
		aiColor3D color(0.0f, 0.0f, 0.0f);
		if (pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
		{
			dstMaterial.specular = XMFLOAT3(color.r, color.g, color.b);
		}
		else
		{
			dstMaterial.specular = XMFLOAT3(0.0f, 0.0f, 0.0f);
		}
	}
	
	{
		float shininess;
		if (pSrcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
		{
			dstMaterial.specularPower = shininess;
		}
		else
		{
			dstMaterial.specularPower = 0.0f;
		}
	}

	{
		aiColor3D ambient(0.0f, 0.0f, 0.0f);
		if (pSrcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS)
		{
			dstMaterial.ambient = XMFLOAT3(ambient.r, ambient.g, ambient.b);
		}
		else
		{
			dstMaterial.ambient = XMFLOAT3(0.0f, 0.0f, 0.0f);
		}
	}
}


bool Model::VertexInit()
{
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = vertexNum * sizeof(MeshVertex);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(vertexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	MeshVertex* vertMap = nullptr;
	result = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(result)) return false;
	
	
	std::copy(std::begin(mesh.Vertices), std::end(mesh.Vertices), vertMap);
	
	
	//std::copy(std::begin(Meshes[0].Vertices), std::end(Meshes[0].Vertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertexNum * sizeof(MeshVertex);
	vbView.StrideInBytes = sizeof(MeshVertex);

	return true;
}

bool Model::IndexInit()
{
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = numIndex * sizeof(uint32_t);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(indexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	uint32_t* indexMap = nullptr;
	result = indexBuffer->Map(0, nullptr, (void**)&indexMap);
	if (FAILED(result)) return false;
	
	std::copy(std::begin(mesh.Indices), std::end(mesh.Indices), indexMap);

	indexBuffer->Unmap(0, nullptr);
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = numIndex * sizeof(uint32_t);

	return true;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Model::CreateBuffer(UINT64 width)
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((width + 0xff) & ~0xff);
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer = nullptr;
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
	);
	return buffer.Get();
}

bool Model::WorldBuffInit()
{
	_worldBuff = CreateBuffer(sizeof(world));
	auto result = _worldBuff->Map(
		0, 
		nullptr, 
		(void**)&worldMatrix
	);
	if (FAILED(result)) return false;
	world = XMMatrixIdentity();
	*worldMatrix = world;
	SetCBV(_camera->GetSceneTransBuff());
	SetCBV(_worldBuff);
	return true;
}

bool Model::ModelHeapInit()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = GetCbvDescs() + GetSrvDescs();
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&descHeapDesc,
		IID_PPV_ARGS(_modelHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}


bool Model::MaterialBuffInit()
{
	_materialBuff = CreateBuffer(sizeof(Materials[0]));
	Material* materialMap = nullptr;
	auto result = _materialBuff->Map(
		0, 
		nullptr, 
		(void**)&materialMap
	);
	if (FAILED(result)) return false;
	std::copy(
		std::begin(Materials), 
		std::end(Materials),
		materialMap
	);
	SetCBV(_materialBuff);
	return true;
}



void Model::SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format)
{
	std::pair< Microsoft::WRL::ComPtr<ID3D12Resource>, DXGI_FORMAT> srvBuff = { buffer, format };
	srvBuffs.emplace_back(srvBuff);
}

void Model::SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer)
{
	cbvBuffs.emplace_back(buffer);
}

void Model::SetViews()
{
	if (!ModelHeapInit()) return;
	for (int i = 0; i < cbvBuffs.size(); ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = cbvBuffs[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(cbvBuffs[i]->GetDesc().Width);
		auto handle = _modelHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * i;
		_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
	for (int i = 0; i < srvBuffs.size(); ++i)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = srvBuffs[i].second;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		auto handle = _modelHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (i + cbvBuffs.size());
		_dx->GetDevice()->CreateShaderResourceView(srvBuffs[i].first.Get(), &srvDesc, handle);
	}
}


Model::Model(
	std::shared_ptr<Wrapper> dx, 
	std::shared_ptr<Camera> camera,
	std::string filePath
) : _dx(dx), _camera(camera), _pos(0, 0, 0), _rotater(0, 0, 0)
{
	Load(filePath);
}

bool Model::Init()
{
	if (!VertexInit()) return false;
	if (!IndexInit()) return false;
	if (!WorldBuffInit()) return false;
	if (!MaterialBuffInit()) return false;
	world =
		XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	*worldMatrix = world;
	return true;
}

bool Model::RendererInit()
{
	ModelHeapInit();
	SetViews();
	return true;
}

void Model::Update()
{
	world =
		 XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	
	*worldMatrix = world;
	
}

void Model::Draw()
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_dx->GetCommandList() ->IASetVertexBuffers(0, 1, &vbView);
	_dx->GetCommandList()->IASetIndexBuffer(&ibView);

	ID3D12DescriptorHeap* heaps[] = { _modelHeap.Get() };

	_dx->GetCommandList()->SetDescriptorHeaps(1, heaps);
	_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
		0,
		_modelHeap->GetGPUDescriptorHandleForHeapStart());
	
	_dx->GetCommandList()->DrawIndexedInstanced(
		numIndex,
		1, 
		0, 
		0, 
		0
	);
}

void Model::Move(float x, float y, float z)
{
	_pos.x += x;
	_pos.y += y;
	_pos.z += z;
}

void Model::Rotate(float x, float y, float z)
{
	_rotater.x += x;
	_rotater.y += y;
	_rotater.z += z;
}

Model::~Model()
{
}