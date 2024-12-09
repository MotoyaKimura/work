#include "PmxModel.h"
#include <fstream>
#include <iostream>
#include "Texture.h"
#include "Wrapper.h"
#include "BoneNode.h"
#include "MorphManager.h"
#include "NodeManager.h"
#include "Application.h"

#pragma comment(lib, "winmm.lib");

using namespace DirectX;

std::wstring PmxModel::GetTexturePathFromModelAndTexPath(const std::string& modelPath, const std::wstring& texPathW)
{
	
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto modelDir = modelPath.substr(0, pathIndex + 1);
	auto modelDirW = _dx->GetWideStringFromString(modelDir);
	modelDirW.pop_back();
	return modelDirW + texPathW;

}

bool PmxModel::Load(std::string filePath)
{
	if (filePath.empty()) return false;
	std::ifstream pmxFile{ filePath, (std::ios::binary | std::ios::in) };
	if (pmxFile.fail()) return false;
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

	_nodeManager.reset(new NodeManager());
	_nodeManager->Init(pmxData.bones);


	std::shared_ptr<VMD> vmd;
	vmd.reset(new VMD());
	auto result = vmd->LoadVMD(L"vmdData\\1.ぼんやり待ち_(490f_移動なし).vmd");
	if (!result) return false;
	InitAnimation(vmd->vmdData);

	_morphManager.reset(new MorphManager());
	_morphManager->Init(pmxData.morphs, 
		vmd->vmdData.morphs, 
		pmxData.vertices.size(), 
		pmxData.materials.size(), 
		pmxData.bones.size()
	);

	return true;
}

bool PmxModel::ReadHeader(PMXFileData& data, std::ifstream& file)
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

	file.read(reinterpret_cast<char*>(&data.header.vertexIndexSize), sizeof(data.header.vertexIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.textureIndexSize), sizeof(data.header.textureIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.materialIndexSize), sizeof(data.header.materialIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.boneIndexSize), sizeof(data.header.boneIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.morphIndexSize), sizeof(data.header.morphIndexSize));
	file.read(reinterpret_cast<char*>(&data.header.rigidBodyIndexSize), sizeof(data.header.rigidBodyIndexSize));
	return true;
}

bool PmxModel::GetPMXStringUTF16(std::ifstream& _file, std::wstring& output)
{
	std::array<wchar_t, 1024> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);
	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::wstring(&wBuffer[0], &wBuffer[0] + textSize / 2);

	return true;
}

bool PmxModel::GetPMXStringUTF8(std::ifstream& _file, std::string& output)
{
	std::array<char, 1024> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);
	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::string(&wBuffer[0], &wBuffer[0] + textSize);

	return true;
}


bool PmxModel::ReadModelInfo(PMXFileData& data, std::ifstream& file)
{
	GetPMXStringUTF16(file, data.modelInfo.modelName);
	GetPMXStringUTF8(file, data.modelInfo.englishModelName);
	GetPMXStringUTF16(file, data.modelInfo.comment);
	GetPMXStringUTF8(file, data.modelInfo.englishComment);
	return true;
}

bool PmxModel::ReadVertex(PMXFileData& data, std::ifstream& file)
{
	unsigned int vertexCount;
	file.read(reinterpret_cast<char*>(&vertexCount), 4);
	vertexNum = vertexCount;
	data.vertices.resize(vertexCount);
	mesh.Vertices.resize(vertexCount);
	int i = 0;
	float xMin = (std::numeric_limits<float>::max)();
	float xMax = -(std::numeric_limits<float>::max)();
	float yMin = (std::numeric_limits<float>::max)();
	float yMax = -(std::numeric_limits<float>::max)();
	float zMin = (std::numeric_limits<float>::max)();
	float zMax = -(std::numeric_limits<float>::max)();
	for (auto& vertex : data.vertices)
	{
		file.read(reinterpret_cast<char*>(&vertex.position), 12);
		xMin = std::min(xMin, vertex.position.x * 0.2f);
		yMin = std::min(yMin, vertex.position.y * 0.2f);
		zMin = std::min(zMin, vertex.position.z * 0.2f);
		xMax = (std::max)(xMax, vertex.position.x * 0.2f);
		yMax = (std::max)(yMax, vertex.position.y * 0.2f);
		zMax = (std::max)(zMax, vertex.position.z * 0.2f);

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
		mesh.Vertices[i] = MeshVertex(
			XMFLOAT3(vertex.position.x * 0.2, vertex.position.y * 0.2, vertex.position.z * 0.2),
			XMFLOAT3(vertex.normal.x, vertex.normal.y, vertex.normal.z),
			XMFLOAT2(vertex.uv.x, vertex.uv.y),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
			vertex.boneIndices[0], vertex.boneIndices[1], vertex.boneIndices[2], vertex.boneIndices[3],
			vertex.boneWeights[0], vertex.boneWeights[1], vertex.boneWeights[2], vertex.boneWeights[3],
			(unsigned int)vertex.weightType
		);
		i++;
	}
	_aabb._xMax = xMax;
	_aabb._yMax = yMax;
	_aabb._zMax = zMax;
	_aabb._xMin = xMin;
	_aabb._yMin = yMin;
	_aabb._zMin = zMin;

	return true;
}

bool PmxModel::ReadFace(PMXFileData& data, std::ifstream& file)
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

bool PmxModel::ReadTexture(PMXFileData& data, std::ifstream& file)
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

bool PmxModel::ReadMaterial(PMXFileData& data, std::ifstream& file)
{
	int numOfMaterial = 0;
	file.read(reinterpret_cast<char*>(&numOfMaterial), 4);

	Materials.resize(numOfMaterial);
	mTextureResources.resize(numOfMaterial);
	mToonResources.resize(numOfMaterial);
	mSphereTextureResources.resize(numOfMaterial);

	mLoadedMaterial.resize(numOfMaterial);
	int materialIndex = 0;

	data.materials.resize(numOfMaterial);
	for (auto& mat : data.materials)
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
		if (mat.toonMode == PMXToonMode::Separate)
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
		std::string name(mat.name.begin(), mat.name.end());
		mLoadedMaterial[materialIndex].name = name;
		mLoadedMaterial[materialIndex].diffuse = mat.diffuse;
		mLoadedMaterial[materialIndex].specular = mat.specular;
		mLoadedMaterial[materialIndex].specularPower = mat.specularPower;
		mLoadedMaterial[materialIndex].ambient = mat.ambient;
		mLoadedMaterial[materialIndex].isTransparent = false;

		Materials[materialIndex].diffuse = mat.diffuse;
		Materials[materialIndex].specular = mat.specular;
		Materials[materialIndex].specularPower = mat.specularPower;
		Materials[materialIndex].ambient = mat.ambient;

		if (mat.textureIndex == 0xff)
		{
			mTextureResources[materialIndex] = nullptr;
		}
		else
		{
			std::shared_ptr<Texture> texture;
			texture.reset(new Texture(_dx));
			std::wstring texPath =
				GetTexturePathFromModelAndTexPath(
					_filePath,
					data.textures[mat.textureIndex].textureName
				);
			texture->Init(texPath);

			mTextureResources[materialIndex] = texture->GetTexBuff();
		}

		if (mat.toonTextureIndex == 0xff)
		{
			mToonResources[materialIndex] = nullptr;
		}
		else
		{
			std::shared_ptr<Texture> toonTexture;
			toonTexture.reset(new Texture(_dx));
			std::wstring toonPath =
				GetTexturePathFromModelAndTexPath(
					_filePath,
					data.textures[mat.toonTextureIndex].textureName
				);
			toonTexture->Init(toonPath);
			mToonResources[materialIndex] = toonTexture->GetTexBuff();
		}

		if (mat.sphereTextureIndex == 0xff)
		{
			mSphereTextureResources[materialIndex] = nullptr;
		}
		else
		{
			std::shared_ptr<Texture> sphereTexture;
			sphereTexture.reset(new Texture(_dx));
			std::wstring spherePath =
				GetTexturePathFromModelAndTexPath(
					_filePath,
					data.textures[mat.sphereTextureIndex].textureName
				);
			sphereTexture->Init(spherePath);
			mSphereTextureResources[materialIndex] = sphereTexture->GetTexBuff();
		}
		materialIndex++;
	}
	return true;
}

bool PmxModel::ReadBone(PMXFileData& data, std::ifstream& file)
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

bool PmxModel::ReadMorph(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfMorph = 0;
	file.read(reinterpret_cast<char*>(&numOfMorph), 4);

	data.morphs.resize(numOfMorph);

	for (auto& morph : data.morphs)
	{
		GetPMXStringUTF16(file, morph.name);
		GetPMXStringUTF8(file, morph.englishName);

		file.read(reinterpret_cast<char*>(&morph.controlPanel), 1);
		file.read(reinterpret_cast<char*>(&morph.morphType), 1);

		unsigned int dataCount;
		file.read(reinterpret_cast<char*>(&dataCount), 4);

		if (morph.morphType == PMXMorphType::Position)
		{
			morph.positionMorph.resize(dataCount);
			for (auto& morphData : morph.positionMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.vertexIndex), data.header.vertexIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.position), 12);
				mesh.Vertices[morphData.vertexIndex].MorphPosition = morphData.position;
			}
		}
		else if (morph.morphType == PMXMorphType::UV ||
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
				mesh.Vertices[morphData.vertexIndex].MorphUV = morphData.uv;
			}
		}
		else if (morph.morphType == PMXMorphType::Bone)
		{
			morph.boneMorph.resize(dataCount);
			for (auto& morphData : morph.boneMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.boneIndex), data.header.boneIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.position), 12);
				file.read(reinterpret_cast<char*>(&morphData.quaternion), 16);
			}
		}
		else if (morph.morphType == PMXMorphType::Material)
		{
			morph.materialMorph.resize(dataCount);
			for (auto& morphData : morph.materialMorph)
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
		else if (morph.morphType == PMXMorphType::Group)
		{
			morph.groupMorph.resize(dataCount);
			for (auto& morphData : morph.groupMorph)
			{
				file.read(reinterpret_cast<char*>(&morphData.morphIndex), data.header.morphIndexSize);
				file.read(reinterpret_cast<char*>(&morphData.weight), 4);
			}
		}
		else if (morph.morphType == PMXMorphType::Flip)
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

bool PmxModel::ReadDisplayFrame(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfDisplayFrame = 0;
	file.read(reinterpret_cast<char*>(&numOfDisplayFrame), 4);

	data.displayFrames.resize(numOfDisplayFrame);

	for (auto& displayFlame : data.displayFrames)
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

bool PmxModel::ReadRigidBody(PMXFileData& data, std::ifstream& file)
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

bool PmxModel::ReadJoint(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfJoint = 0;
	file.read(reinterpret_cast<char*>(&numOfJoint), 4);

	data.joints.resize(numOfJoint);

	for (auto& joint : data.joints)
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

bool PmxModel::ReadSoftBody(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfSoftBody = 0;
	file.read(reinterpret_cast<char*>(&numOfSoftBody), 4);

	data.softBodies.resize(numOfSoftBody);

	for (auto& softBody : data.softBodies)
	{
		GetPMXStringUTF16(file, softBody.name);
		GetPMXStringUTF8(file, softBody.englishName);

		file.read(reinterpret_cast<char*>(&softBody.type), 1);

		file.read(reinterpret_cast<char*>(&softBody.materialIndex), data.header.materialIndexSize);
		file.read(reinterpret_cast<char*>(&softBody.group), 1);
		file.read(reinterpret_cast<char*>(&softBody.collisionGroup), 2);

		file.read(reinterpret_cast<char*>(&softBody.flag), 1);

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

void PmxModel::InitAnimation(VMDFileData& vmdData)
{
	const std::vector<BoneNode*>& allNodes = _nodeManager->GetAllNodes();
	for (auto& node : allNodes)
	{
		node->ClearKey();
	}

	for (auto& motion : vmdData.motions)
	{
		auto boneNode = _nodeManager->GetBoneNodeByName(motion.boneName);
		if(boneNode == nullptr)
		{
			continue;
		}
		boneNode->AddMotionKey(motion.frame,
			motion.quaternion,
			motion.translate,
			XMFLOAT2(static_cast<float>(motion.interpolation[3]) / 127.0f, static_cast<float>(motion.interpolation[7]) / 127.0f),
			XMFLOAT2(static_cast<float>(motion.interpolation[11]) / 127.0f, static_cast<float>(motion.interpolation[15]) / 127.0f));
	}

	for(VMDIK& ik : vmdData.iks)
	{
		for (VMDIKInfo& ikInfo : ik.ikInfos)
		{
			auto boneNode = _nodeManager->GetBoneNodeByName(ikInfo.name);
			if (boneNode == nullptr)
			{
				continue;
			}
			bool enable = ikInfo.enable;
			boneNode->AddIKKey(ik.frame, enable);
		}
	}

	_nodeManager->SortKey();
	//InitParallelVertexSkinningSetting();

	//PlayAnimation();
}

void PmxModel::UpdateAnimation()
{
	if(_startTime <= 0)
	{
		_startTime = timeGetTime();
	}

	DWORD elapsedTime = timeGetTime() - _startTime;
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	if(Application::GetIsMoveKeyUp())
	{
		motionCountDown = 0;
		Application::SetIsMoveKeyDown(false);
		if (frameNo > _nodeManager->_duration && motionCountUp == 0)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\4.止る_滑り_(25f_前移動30).vmd");
			motionCountUp++;
		}
		if (frameNo > _nodeManager->_duration && motionCountUp == 1)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\1.ぼんやり待ち_(490f_移動なし).vmd");
			motionCountUp = 0;
			Application::SetIsMoveKeyUp(false);
		}
	}

	if (Application::GetIsMoveKeyDown())
	{
		motionCountUp = 0;
		Application::SetIsMoveKeyUp(false);
		if (motionCountDown == 0)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\1.走り出し_(15f_前移動20).vmd");
			motionCountDown++;
		}
		if (frameNo > _nodeManager->_duration && motionCountDown == 1)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\2.走り75L_ダッシュ_(16f_前移動60).vmd");
			motionCountDown++;

		}
	}

	BYTE key[256];
	GetKeyboardState(key);
	if (Application::GetIsKeyJump())
	{
		if(motionCountJump == 0)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\1.予備動作_(7f_移動なし).vmd");
			motionCountJump++;
		}
		if (frameNo > _nodeManager->_duration && motionCountJump == 1)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\2.ジャンプ_(11f_上移動3~10の間_前移動0~10の間).vmd");
			motionCountJump++;
		}
		if (frameNo > _nodeManager->_duration && motionCountJump == 2)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\3.着地_(8f_移動なし).vmd");
			motionCountJump++;
		}
		if (frameNo > _nodeManager->_duration && motionCountJump == 3)
		{
			_startTime = timeGetTime();
			frameNo = 0;
			ChangeVMD(L"vmdData\\1.ぼんやり待ち_(490f_移動なし).vmd");
			motionCountJump = 0;
			Application::SetIsKeyJump(false);
		}
	}

	if(frameNo > _nodeManager->_duration)
	{
		_startTime = timeGetTime();
		frameNo = 0;
	}

	_nodeManager->BeforeUpdateAnimation();

	_morphManager->Animate(frameNo);
	_nodeManager->UpdateAnimation(frameNo);

	MorphMaterial();
	MorphBone();

    VertexSkinning();

	std::copy(mesh.Vertices.begin(), mesh.Vertices.end(), vertMap);
}

void PmxModel::ChangeVMD(std::wstring vmdFile)
{
	std::shared_ptr<VMD> vmd;
	vmd.reset(new VMD());
	auto result = vmd->LoadVMD(vmdFile);
	if (!result) return;
	_nodeManager->_duration = 0;
	InitAnimation(vmd->vmdData);
	_morphManager->Init(pmxData.morphs,
		vmd->vmdData.morphs,
		pmxData.vertices.size(),
		pmxData.materials.size(),
		pmxData.bones.size());
}


void PmxModel::PlayAnimation()
{
	_startTime = timeGetTime();
}


void PmxModel::VertexSkinning()
{
	for(unsigned int i = 0; i < pmxData.vertices.size(); ++i)
	{
		const PMXVertex& currentVertexData = pmxData.vertices[i];
		XMVECTOR position = XMLoadFloat3(&currentVertexData.position);
		XMVECTOR morphPosition = XMLoadFloat3(&_morphManager->GetMorphVertexPosition(i));

		switch (currentVertexData.weightType)
		{
		case PMXVertexWeight::BDEF1:
		{
			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			position += morphPosition;
			position = XMVector3Transform(position, m0);

			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);
			XMMATRIX rotation = XMMatrixSet(
				m0.r[0].m128_f32[0], m0.r[0].m128_f32[1], m0.r[0].m128_f32[2], 0.0f,
				m0.r[1].m128_f32[0], m0.r[1].m128_f32[1], m0.r[1].m128_f32[2], 0.0f,
				m0.r[2].m128_f32[0], m0.r[2].m128_f32[1], m0.r[2].m128_f32[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::BDEF2:
		{
			float weight0 = currentVertexData.boneWeights[0];
			float weight1 = 1.0f - weight0;

			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			BoneNode* bone1 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[1]);

			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			XMMATRIX m1 = XMMatrixMultiply(bone1->GetInitInverseTransform(), bone1->GetGlobalTransform());

			XMMATRIX mat = m0 * weight0 + m1 * weight1;
			position += morphPosition;
			position = XMVector3Transform(position, mat);

			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);
			XMMATRIX rotation = XMMatrixSet(
				mat.r[0].m128_f32[0], mat.r[0].m128_f32[1], mat.r[0].m128_f32[2], 0.0f,
				mat.r[1].m128_f32[0], mat.r[1].m128_f32[1], mat.r[1].m128_f32[2], 0.0f,
				mat.r[2].m128_f32[0], mat.r[2].m128_f32[1], mat.r[2].m128_f32[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::BDEF4:
			{
			float weight0 = currentVertexData.boneWeights[0];
			float weight1 = currentVertexData.boneWeights[1];
			float weight2 = currentVertexData.boneWeights[2];
			float weight3 = currentVertexData.boneWeights[3];

			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			BoneNode* bone1 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[1]);
			BoneNode* bone2 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[2]);
			BoneNode* bone3 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[3]);

			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			XMMATRIX m1 = XMMatrixMultiply(bone1->GetInitInverseTransform(), bone1->GetGlobalTransform());
			XMMATRIX m2 = XMMatrixMultiply(bone2->GetInitInverseTransform(), bone2->GetGlobalTransform());
			XMMATRIX m3 = XMMatrixMultiply(bone3->GetInitInverseTransform(), bone3->GetGlobalTransform());

			XMMATRIX mat = m0 * weight0 + m1 * weight1 + m2 * weight2 + m3 * weight3;
			position += morphPosition;
			position = XMVector3Transform(position, mat);

			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);
			XMMATRIX rotation = XMMatrixSet(
				mat.r[0].m128_f32[0], mat.r[0].m128_f32[1], mat.r[0].m128_f32[2], 0.0f,
				mat.r[1].m128_f32[0], mat.r[1].m128_f32[1], mat.r[1].m128_f32[2], 0.0f,
				mat.r[2].m128_f32[0], mat.r[2].m128_f32[1], mat.r[2].m128_f32[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::SDEF:
		{
			float w0 = currentVertexData.boneWeights[0];
			float w1 = 1.0f - w0;

			XMVECTOR sdefc = XMLoadFloat3(&currentVertexData.sdefC);
			XMVECTOR sdefr0 = XMLoadFloat3(&currentVertexData.sdefR0);
			XMVECTOR sdefr1 = XMLoadFloat3(&currentVertexData.sdefR1);

			XMVECTOR rw = sdefr0 * w0 + sdefr1 * w1;
			XMVECTOR r0 = sdefc + sdefr0 - rw;
			XMVECTOR r1 = sdefc + sdefr1 - rw;

			XMVECTOR cr0 = (sdefc + r0) * 0.5f;
			XMVECTOR cr1 = (sdefc + r1) * 0.5f;

			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			BoneNode* bone1 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[1]);

			XMVECTOR q0 = XMQuaternionRotationMatrix(bone0->GetGlobalTransform());
			XMVECTOR q1 = XMQuaternionRotationMatrix(bone1->GetGlobalTransform());

			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			XMMATRIX m1 = XMMatrixMultiply(bone1->GetInitInverseTransform(), bone1->GetGlobalTransform());

			XMMATRIX rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(q0, q1, w1));

			position += morphPosition;

			position = XMVector3Transform(position - sdefc, rotation) + XMVector3Transform(cr0, m0) * w0 + XMVector3Transform(cr1, m1) * w1;
			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);

			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::QDEF:
		{
			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());

			position += morphPosition;
			position = XMVector3Transform(position, m0);

			break;
		}
		default:
			break;
		}
		XMStoreFloat3(&mesh.Vertices[i].Position, XMVectorScale(position, 0.2));

		const XMFLOAT4& morphUV = _morphManager->GetMorphUV(i);
		const XMFLOAT2& originalUV = mesh.Vertices[i].TexCoord;
		mesh.Vertices[i].TexCoord = XMFLOAT2(originalUV.x + morphUV.x, originalUV.y + morphUV.y);
	}
}

//void PmxModel::VertexSkinning()
//{
//	const int futureCount = _parallelUpdateFutures.size();
//
//	for (int i = 0; i < futureCount; i++)
//	{
//		const SkinningRange& currentRange = _skinningRanges[i];
//		_parallelUpdateFutures[i] = std::async(std::launch::async, [this, currentRange]()
//			{
//				this->VertexSkinningByRange(currentRange);
//			});
//	}
//
//	for (const std::future<void>& future : _parallelUpdateFutures)
//	{
//		future.wait();
//	}
//}

void PmxModel::InitParallelVertexSkinningSetting()
{
	unsigned int threadCount = std::thread::hardware_concurrency();
	unsigned int divNum = threadCount - 1;

	_skinningRanges.resize(threadCount);
	_parallelUpdateFutures.resize(threadCount);

	unsigned int divVertexCount = pmxData.vertices.size() / divNum;
	unsigned int remainder = pmxData.vertices.size() % divNum;

	int startIndex = 0;
	for(int i = 0; i < _skinningRanges.size() - 1; i++)
	{
		_skinningRanges[i].startIndex = startIndex;
		_skinningRanges[i].vertexCount = divVertexCount;

		startIndex += _skinningRanges[i].vertexCount;
	}

	_skinningRanges[_skinningRanges.size() - 1].startIndex = startIndex;
	_skinningRanges[_skinningRanges.size() - 1].vertexCount = remainder;
}

void PmxModel::VertexSkinningByRange(const SkinningRange& range)
{
	for (unsigned int i = 0; i < pmxData.vertices.size(); ++i)
	{
		const PMXVertex& currentVertexData = pmxData.vertices[i];
		XMVECTOR position = XMLoadFloat3(&currentVertexData.position);
		XMVECTOR morphPosition = XMLoadFloat3(&_morphManager->GetMorphVertexPosition(i));

		switch (currentVertexData.weightType)
		{
		case PMXVertexWeight::BDEF1:
		{
			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			position += morphPosition;
			position = XMVector3Transform(position, m0);

			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);
			XMMATRIX rotation = XMMatrixSet(
				m0.r[0].m128_f32[0], m0.r[0].m128_f32[1], m0.r[0].m128_f32[2], 0.0f,
				m0.r[1].m128_f32[0], m0.r[1].m128_f32[1], m0.r[1].m128_f32[2], 0.0f,
				m0.r[2].m128_f32[0], m0.r[2].m128_f32[1], m0.r[2].m128_f32[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::BDEF2:
		{
			float weight0 = currentVertexData.boneWeights[0];
			float weight1 = 1.0f - weight0;

			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			BoneNode* bone1 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[1]);

			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			XMMATRIX m1 = XMMatrixMultiply(bone1->GetInitInverseTransform(), bone1->GetGlobalTransform());

			XMMATRIX mat = m0 * weight0 + m1 * weight1;
			position += morphPosition;
			position = XMVector3Transform(position, mat);

			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);
			XMMATRIX rotation = XMMatrixSet(
				mat.r[0].m128_f32[0], mat.r[0].m128_f32[1], mat.r[0].m128_f32[2], 0.0f,
				mat.r[1].m128_f32[0], mat.r[1].m128_f32[1], mat.r[1].m128_f32[2], 0.0f,
				mat.r[2].m128_f32[0], mat.r[2].m128_f32[1], mat.r[2].m128_f32[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::BDEF4:
		{
			float weight0 = currentVertexData.boneWeights[0];
			float weight1 = currentVertexData.boneWeights[1];
			float weight2 = currentVertexData.boneWeights[2];
			float weight3 = currentVertexData.boneWeights[3];

			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			BoneNode* bone1 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[1]);
			BoneNode* bone2 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[2]);
			BoneNode* bone3 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[3]);

			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			XMMATRIX m1 = XMMatrixMultiply(bone1->GetInitInverseTransform(), bone1->GetGlobalTransform());
			XMMATRIX m2 = XMMatrixMultiply(bone2->GetInitInverseTransform(), bone2->GetGlobalTransform());
			XMMATRIX m3 = XMMatrixMultiply(bone3->GetInitInverseTransform(), bone3->GetGlobalTransform());

			XMMATRIX mat = m0 * weight0 + m1 * weight1 + m2 * weight2 + m3 * weight3;
			position += morphPosition;
			position = XMVector3Transform(position, mat);

			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);
			XMMATRIX rotation = XMMatrixSet(
				mat.r[0].m128_f32[0], mat.r[0].m128_f32[1], mat.r[0].m128_f32[2], 0.0f,
				mat.r[1].m128_f32[0], mat.r[1].m128_f32[1], mat.r[1].m128_f32[2], 0.0f,
				mat.r[2].m128_f32[0], mat.r[2].m128_f32[1], mat.r[2].m128_f32[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::SDEF:
		{
			float w0 = currentVertexData.boneWeights[0];
			float w1 = 1.0f - w0;

			XMVECTOR sdefc = XMLoadFloat3(&currentVertexData.sdefC);
			XMVECTOR sdefr0 = XMLoadFloat3(&currentVertexData.sdefR0);
			XMVECTOR sdefr1 = XMLoadFloat3(&currentVertexData.sdefR1);

			XMVECTOR rw = sdefr0 * w0 + sdefr1 * w1;
			XMVECTOR r0 = sdefc + sdefr0 - rw;
			XMVECTOR r1 = sdefc + sdefr1 - rw;

			XMVECTOR cr0 = (sdefc + r0) * 0.5f;
			XMVECTOR cr1 = (sdefc + r1) * 0.5f;

			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			BoneNode* bone1 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[1]);

			XMVECTOR q0 = XMQuaternionRotationMatrix(bone0->GetGlobalTransform());
			XMVECTOR q1 = XMQuaternionRotationMatrix(bone1->GetGlobalTransform());

			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());
			XMMATRIX m1 = XMMatrixMultiply(bone1->GetInitInverseTransform(), bone1->GetGlobalTransform());

			XMMATRIX rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(q0, q1, w1));

			position += morphPosition;

			position = XMVector3Transform(position - sdefc, rotation) + XMVector3Transform(cr0, m0) * w0 + XMVector3Transform(cr1, m1) * w1;
			XMVECTOR normal = XMLoadFloat3(&currentVertexData.normal);

			normal = XMVector3Transform(normal, rotation);
			XMStoreFloat3(&mesh.Vertices[i].Normal, normal);
			break;
		}
		case PMXVertexWeight::QDEF:
		{
			BoneNode* bone0 = _nodeManager->GetBoneNodeByIndex(currentVertexData.boneIndices[0]);
			XMMATRIX m0 = XMMatrixMultiply(bone0->GetInitInverseTransform(), bone0->GetGlobalTransform());

			position += morphPosition;
			position = XMVector3Transform(position, m0);

			break;
		}
		default:
			break;
		}
		XMStoreFloat3(&mesh.Vertices[i].Position, XMVectorScale(position, 0.2));

		const XMFLOAT4& morphUV = _morphManager->GetMorphUV(i);
		const XMFLOAT2& originalUV = mesh.Vertices[i].TexCoord;
		mesh.Vertices[i].TexCoord = XMFLOAT2(originalUV.x + morphUV.x, originalUV.y + morphUV.y);
	}
}

void PmxModel::MorphMaterial()
{
	size_t bufferSize = sizeof(Material);
	bufferSize = (bufferSize + 0xff) & ~0xff;

	
	char* mappedMaterialPtr = materialMap;

	for(int i = 0; i < pmxData.materials.size(); i++)
	{
		PMXMaterial& material = pmxData.materials[i];

		Material* uploadMat = reinterpret_cast<Material*>(mappedMaterialPtr);

		XMVECTOR diffuse = XMLoadFloat4(&material.diffuse);
		XMVECTOR specular = XMLoadFloat3(&material.specular);
		XMVECTOR ambient = XMLoadFloat3(&material.ambient);

		const MaterialMorphData& morphMaterial = _morphManager->GetMorphMaterial(i);
		float weight = morphMaterial.weight;

		XMFLOAT4 resultDiffuse;
		XMFLOAT3 resultSpecular;
		XMFLOAT3 resultAmbient;

		if(morphMaterial.opType == PMXMorph::MaterialMorph::OpType::Add)
		{
			XMStoreFloat4(&resultDiffuse, diffuse + XMLoadFloat4(&morphMaterial.diffuse) * weight);
			XMStoreFloat3(&resultSpecular, specular + XMLoadFloat3(&morphMaterial.specular) * weight);
			XMStoreFloat3(&resultAmbient, ambient + XMLoadFloat3(&morphMaterial.ambient) * weight);
			uploadMat->specularPower += morphMaterial.specularPower * weight;
		}
		else
		{
			XMVECTOR morphDiffuse = XMLoadFloat4(&morphMaterial.diffuse);
			XMVECTOR morphSpecular = XMLoadFloat3(&morphMaterial.specular);
			XMVECTOR morphAmbient = XMLoadFloat3(&morphMaterial.ambient);

			XMStoreFloat4(&resultDiffuse, XMVectorLerp(diffuse, morphDiffuse, weight));
			XMStoreFloat3(&resultSpecular, XMVectorLerp(specular, morphSpecular, weight));
			XMStoreFloat3(&resultAmbient, XMVectorLerp(ambient, morphAmbient, weight));
			uploadMat->specularPower = std::lerp(material.specularPower, material.specularPower * morphMaterial.specularPower, weight);
		}

		uploadMat->diffuse = resultDiffuse;
		uploadMat->specular = resultSpecular;
		uploadMat->ambient = resultAmbient;

		mappedMaterialPtr += bufferSize;
	}
}

void PmxModel::MorphBone()
{
	const std::vector<BoneNode*>& allNodes = _nodeManager->GetAllNodes();

	for(BoneNode* boneNode : allNodes)
	{
		BoneMorphData morph = _morphManager->GetMorphBone(boneNode->GetBoneIndex());
		boneNode->SetMorphPosition(XMFLOAT3(
			std::lerp(0.0f, morph.position.x, morph.weight),
			std::lerp(0.0f, morph.position.y, morph.weight),
			std::lerp(0.0f, morph.position.z, morph.weight)
		));

		XMVECTOR animateRotation = XMQuaternionRotationMatrix(boneNode->GetAnimateRotation());
		XMVECTOR morphRotation = XMLoadFloat4(&morph.quaternion);

		animateRotation = XMQuaternionSlerp(animateRotation, morphRotation, morph.weight);
		boneNode->SetAnimateRotation(XMMatrixRotationQuaternion(animateRotation));
	}
}


bool PmxModel::ModelHeapInit()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 3 + pmxData.materials.size() * 4;
	
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&descHeapDesc,
		IID_PPV_ARGS(_modelHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}

void PmxModel::Update()
{
	world =
		XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);

	*worldMatrix = world;
	UpdateAnimation();
}


void PmxModel::Draw()
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_dx->GetCommandList()->IASetVertexBuffers(0, 1, &vbView);
	_dx->GetCommandList()->IASetIndexBuffer(&ibView);

	ID3D12DescriptorHeap* heaps[] = { _modelHeap.Get() };

	_dx->GetCommandList()->SetDescriptorHeaps(1, heaps);
	auto handle = _modelHeap->GetGPUDescriptorHandleForHeapStart();
	_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
		0,
		handle);

	auto incSize = _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 3;
	unsigned int idxOffset = 0;

	for (int i = 0; i < Materials.size(); i++)
	{
		unsigned int numVertex = 0;
		numVertex = pmxData.materials[i].numFaceVertices;
		_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
			1,
			handle);

		_dx->GetCommandList()->DrawIndexedInstanced(
			numVertex,
			1,
			idxOffset,
			0,
			0
		);

		handle.ptr += incSize;
		idxOffset += numVertex;
	}
}


PmxModel::PmxModel(std::shared_ptr<Wrapper> dx
	, std::shared_ptr<Camera> camera,
	std::string filePath
) : Model(dx, camera, filePath), _dx(dx), _camera(camera), _filePath(filePath)
{
	Load(filePath);
}

PmxModel::~PmxModel()
{
}