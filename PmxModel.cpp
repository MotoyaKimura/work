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

//PMXモデルクラス
PmxModel::PmxModel(std::shared_ptr<Wrapper> dx
	, std::shared_ptr<Camera> camera,
	std::string filePath,
	std::wstring firstVmdPath,
	bool isRepeat
) : Model(dx, camera, filePath), _dx(dx), _camera(camera), _filePath(filePath), _firstVmdPath(firstVmdPath), _isRepeat(isRepeat)
{
}

PmxModel::~PmxModel()
{
}

//テクスチャパスをモデルパスを使ってアプリケーションからみたパスに変換
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

//PMXモデルの読み込み
bool PmxModel::Load()
{
	if (_filePath.empty()) return false;
	std::ifstream pmxFile{ _filePath, (std::ios::binary | std::ios::in) };
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

	//ノードマネージャの初期化
	_nodeManager.reset(new NodeManager());
	_nodeManager->Init(pmxData.bones);

	//VMDモーションの読み込み
	_firstVMD.reset(new VMD());
	_firstVMD->LoadVMD(_firstVmdPath);
	/*_wait.reset(new VMD());
	if (!_wait->LoadVMD(L"vmdData\\1.ぼんやり待ち_(490f_移動なし).vmd")) return false;
	_preRun.reset(new VMD);
	if (!_preRun->LoadVMD(L"vmdData\\1.走り出し_(15f_前移動20).vmd")) return false;
	_run.reset(new VMD);
	if (!_run->LoadVMD(L"vmdData\\2.走り75L_ダッシュ_(16f_前移動60).vmd")) return false;
	_endRun.reset(new VMD);
	if (!_endRun->LoadVMD(L"vmdData\\4.止る_滑り_(25f_前移動30).vmd")) return false;
	_preJump.reset(new VMD);
	if (!_preJump->LoadVMD(L"vmdData\\1.予備動作_(7f_移動なし).vmd")) return false;
	_jump.reset(new VMD);
	if (!_jump->LoadVMD(L"vmdData\\2.ジャンプ_(11f_上移動3~10の間_前移動0~10の間).vmd")) return false;
	_endJump.reset(new VMD);
	if (!_endJump->LoadVMD(L"vmdData\\3.着地_(8f_移動なし).vmd")) return false;
	_jumpFromRun.reset(new VMD);
	if (!_jumpFromRun->LoadVMD(L"vmdData\\1.走りLから大ジャンプ_60L推奨_(25f_前移動90).vmd")) return false;
	_endJumpToRun.reset(new VMD);
	if (!_endJumpToRun->LoadVMD(L"vmdData\\2.着地して走りへ_クッション2_(33f_前移動35).vmd")) return false;
	_endJump2.reset(new VMD);
	if (!_endJump2->LoadVMD(L"vmdData\\3.着地して棒立ち_(25f_前移動5).vmd")) return false;*/


	//最初のモーションをセット
	InitAnimation(_firstVMD->vmdData);
	if(!_isRepeat)
	{
		_nodeManager->SetDuration((std::numeric_limits<unsigned int>::max)());
	}
	_morphManager.reset(new MorphManager(&mesh));
	_morphManager->Init(pmxData.morphs, 
		_firstVMD->vmdData.morphs,
		pmxData.vertices.size(), 
		pmxData.materials.size(), 
		pmxData.bones.size()
	);

	return true;
}

//ヘッダーの読み込み
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

//UTF16文字列の読み込み
bool PmxModel::GetPMXStringUTF16(std::ifstream& _file, std::wstring& output)
{
	std::array<wchar_t, 1024> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);
	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::wstring(&wBuffer[0], &wBuffer[0] + textSize / 2);

	return true;
}

//UTF8文字列の読み込み
bool PmxModel::GetPMXStringUTF8(std::ifstream& _file, std::string& output)
{
	std::array<char, 1024> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);
	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::string(&wBuffer[0], &wBuffer[0] + textSize);

	return true;
}

//モデル情報の読み込み
bool PmxModel::ReadModelInfo(PMXFileData& data, std::ifstream& file)
{
	GetPMXStringUTF16(file, data.modelInfo.modelName);
	GetPMXStringUTF8(file, data.modelInfo.englishModelName);
	GetPMXStringUTF16(file, data.modelInfo.comment);
	GetPMXStringUTF8(file, data.modelInfo.englishComment);
	return true;
}

//頂点情報の読み込み
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
			XMFLOAT3(vertex.position),
			XMFLOAT3(vertex.normal.x, vertex.normal.y, vertex.normal.z),
			XMFLOAT2(vertex.uv.x, vertex.uv.y),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
			XMFLOAT3(vertex.sdefC.x, vertex.sdefC.y, vertex.sdefC.z),
			XMFLOAT3(vertex.sdefR0.x, vertex.sdefR0.y, vertex.sdefR0.z),
			XMFLOAT3(vertex.sdefR1.x, vertex.sdefR1.y, vertex.sdefR1.z),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
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

//インデックス情報の読み込み
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

//テクスチャ情報の読み込み
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

//マテリアル情報の読み込み
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
		mat.name = mat.name + L'\0';
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
			std::wstring texPath =
				GetTexturePathFromModelAndTexPath(
					_filePath,
					data.textures[mat.textureIndex].textureName
				);
			texture.reset(new Texture(_dx, texPath));
			if (!texture->Init()) return false;

			mTextureResources[materialIndex] = texture->GetTexBuff();
		}

		if (mat.toonTextureIndex == 0xff)
		{
			mToonResources[materialIndex] = nullptr;
		}
		else
		{
			std::shared_ptr<Texture> toonTexture;
			
			std::wstring toonPath =
				GetTexturePathFromModelAndTexPath(
					_filePath,
					data.textures[mat.toonTextureIndex].textureName
				);
			toonTexture.reset(new Texture(_dx, toonPath));
			if (!toonTexture->Init()) return false;
			mToonResources[materialIndex] = toonTexture->GetTexBuff();
		}

		if (mat.sphereTextureIndex == 0xff)
		{
			mSphereTextureResources[materialIndex] = nullptr;
		}
		else
		{
			std::shared_ptr<Texture> sphereTexture;
			
			std::wstring spherePath =
				GetTexturePathFromModelAndTexPath(
					_filePath,
					data.textures[mat.sphereTextureIndex].textureName
				);
			sphereTexture.reset(new Texture(_dx, spherePath));
			if (!sphereTexture->Init()) return false;
			mSphereTextureResources[materialIndex] = sphereTexture->GetTexBuff();
		}
		materialIndex++;
	}
	return true;
}

//ボーン情報の読み込み
bool PmxModel::ReadBone(PMXFileData& data, std::ifstream& file)
{
	unsigned int numOfBone;
	file.read(reinterpret_cast<char*>(&numOfBone), 4);

	data.bones.resize(numOfBone);
	boneMatricesNum = numOfBone;
	boneMatrices.resize(numOfBone);
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());
	invBoneMatrices.resize(numOfBone);
	std::fill(invBoneMatrices.begin(), invBoneMatrices.end(), XMMatrixIdentity());
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

//モーフ情報の読み込み
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

//ディスプレイフレーム情報の読み込み
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

//剛体情報の読み込み
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

//ジョイント情報の読み込み
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

//ソフトボディ情報の読み込み
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

//モーションキー情報の読み込み
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
}

//モーションの更新
void PmxModel::UpdateAnimation(bool isStart)
{
	DWORD elapsedTime = timeGetTime() - _startTime;
	unsigned int frameNo = 30 * (elapsedTime / 1000.0f);

	if(_startTime <= 0)
	{
		_startTime = timeGetTime();
		frameNo = 0;
	}

	//幕が上がるまで待機
	//if(isStart)
	//{
	//	//ジャンプしたら
	//	if (GetAsyncKeyState(VK_SPACE) & 0x8000 || isJumping)
	//	{
	//		isJumping = true;
	//		//直前で走っていたとき
	//		if (isRunning)
	//		{
	//			if (motionCountJump == 0)
	//			{
	//				//走りジャンプのモーションに変更
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_jumpFromRun);
	//				motionCountJump++;
	//			}
	//			
	//			else if (motionCountJump == 2)
	//			{
	//				//着地してすぐジャンプキーが押されたらすぐさまジャンプに戻る
	//				if (GetAsyncKeyState(VK_SPACE) & 0x8000) motionCountJump = 0;
	//			}
	//			//着地時に移動キーが押されていたら走りのつなぎモーションに変更
	//			if (frameNo > _nodeManager->_duration && motionCountJump == 1)
	//			{
	//				if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState('A') & 0x8000 ||
	//					GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState('D') & 0x8000)
	//				{
	//					_startTime = timeGetTime();
	//					frameNo = 0;
	//					ChangeVMD(_endJumpToRun);
	//					motionCountJump++;
	//				}
	//				//移動キーが押されていなかったら待ちモーションへのつなぎモーションに変更
	//				else
	//				{
	//					_startTime = timeGetTime();
	//					frameNo = 0;
	//					ChangeVMD(_endJump2);
	//					motionCountJump++;
	//				}
	//			}

	//			//つなぎモーションが終わったら
	//			if (frameNo > _nodeManager->_duration && motionCountJump == 2)
	//			{
	//				//移動している場合、走りモーションに変更
	//				if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState('A') & 0x8000 ||
	//					GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState('D') & 0x8000)
	//				{
	//					_startTime = timeGetTime();
	//					frameNo = 0;
	//					ChangeVMD(_run);
	//					motionCountJump = 0;
	//					Application::SetIsKeyJump(false);
	//					isJumping = false;
	//					motionCountDown = 2;
	//				}
	//				//移動していない場合、待ちモーションに変更
	//				else
	//				{
	//					_startTime = timeGetTime();
	//					frameNo = 0;
	//					ChangeVMD(_wait);
	//					motionCountJump = 0;
	//					Application::SetIsKeyJump(false);
	//					isJumping = false;
	//					isRunning = false;
	//					motionCountDown = 0;
	//				}
	//			}
	//		}
	//		//直前で走っていなかったとき、普通のジャンプモーションに変更
	//		else
	//		{
	//			//まずは助走
	//			if (motionCountJump == 0)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_preJump);
	//				motionCountJump++;
	//			}
	//			//次にジャンプ
	//			if (frameNo > _nodeManager->_duration && motionCountJump == 1)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_jump);
	//				motionCountJump++;
	//			}
	//			//次に着地
	//			if (frameNo > _nodeManager->_duration && motionCountJump == 2)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_endJump);
	//				motionCountJump++;
	//			}
	//			//最後に待ち
	//			if (frameNo > _nodeManager->_duration && motionCountJump == 3)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_wait);
	//				motionCountJump = 0;
	//				Application::SetIsKeyJump(false);
	//				isJumping = false;
	//				motionCountDown = 0;
	//			}
	//		}
	//	}
	//	//ジャンプしていない
	//	else
	//	{
	//		//移動キーが押されていたら走りモーションに変更
	//		if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState('A') & 0x8000 ||
	//			GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState('D') & 0x8000)
	//		{
	//			isRunning = true;
	//			motionCountUp = 0;
	//			Application::SetIsMoveKeyUp(false);
	//			//助走
	//			if (motionCountDown == 0)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_preRun);
	//				motionCountDown++;
	//			}
	//			//走り
	//			if (frameNo > _nodeManager->_duration && motionCountDown == 1)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_run);
	//				motionCountDown++;

	//			}
	//		}
	//		//走りが終わったら
	//		else if (isRunning)
	//		{
	//			//止まるモーションに変更
	//			motionCountDown = 0;
	//			Application::SetIsMoveKeyDown(false);
	//			if (frameNo > _nodeManager->_duration && motionCountUp == 0)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_endRun);
	//				motionCountUp++;
	//			}
	//			//その後、待ちモーションに変更
	//			if (frameNo > _nodeManager->_duration && motionCountUp == 1)
	//			{
	//				_startTime = timeGetTime();
	//				frameNo = 0;
	//				ChangeVMD(_wait);
	//				motionCountUp = 0;
	//				isRunning = false;
	//				Application::SetIsMoveKeyUp(false);
	//			}
	//		}
	//	}
	//}
	//
	//モーションキーが最大値を超えたらリセット
	if(frameNo > _nodeManager->_duration)
	{
		_startTime = timeGetTime();
		frameNo = 0;
	}

	_nodeManager->BeforeUpdateAnimation();

	//ボーンのアニメーション
	_morphManager->Animate(frameNo);
	_nodeManager->UpdateAnimation(frameNo);

	//モーフのアニメーション
	MorphMaterial();
	MorphBone();

	//頂点スキニング
    VertexSkinning();

	//シェーダーに送る
	std::copy(mesh.Vertices.begin(), mesh.Vertices.end(), vertMap);
}

//モーションの変更
void PmxModel::ChangeVMD(std::shared_ptr<VMD> vmd)
{
	
	_nodeManager->_duration = 0;
	InitAnimation(vmd->vmdData);
	_morphManager->Init(pmxData.morphs,
		vmd->vmdData.morphs,
		pmxData.vertices.size(),
		pmxData.materials.size(),
		pmxData.bones.size());
}

//アニメーションの開始
void PmxModel::PlayAnimation()
{
	_startTime = timeGetTime();
}

//頂点スキニング
void PmxModel::VertexSkinning()
{
	const std::vector<BoneNode*>& allNodes = _nodeManager->GetAllNodes();
	for(int i = 0; i < allNodes.size(); i++)
	{
		boneMatrices[i] = allNodes[i]->GetGlobalTransform();
		invBoneMatrices[i] = allNodes[i]->GetInitInverseTransform();
	}
	std::copy(boneMatrices.begin(), boneMatrices.end(), worldMatrix + 1);
	std::copy(invBoneMatrices.begin(), invBoneMatrices.end(), invTransMatrix);

	for(unsigned int i = 0; i < pmxData.vertices.size(); ++i)
	{
		const PMXVertex& currentVertexData = pmxData.vertices[i];

		switch (currentVertexData.weightType)
		{
		case PMXVertexWeight::BDEF1:
		{
			mesh.Vertices[i].boneNo[0] = currentVertexData.boneIndices[0];
			break;
		}
		case PMXVertexWeight::BDEF2:
		{
			mesh.Vertices[i].boneWeight[0] = currentVertexData.boneWeights[0];
			mesh.Vertices[i].boneNo[0] = currentVertexData.boneIndices[0];
			mesh.Vertices[i].boneNo[1] = currentVertexData.boneIndices[1];
			break;
		}
		case PMXVertexWeight::BDEF4:
			{
			mesh.Vertices[i].boneWeight[0] = currentVertexData.boneWeights[0];
			mesh.Vertices[i].boneWeight[1] = currentVertexData.boneWeights[1];
			mesh.Vertices[i].boneWeight[2] = currentVertexData.boneWeights[2];
			mesh.Vertices[i].boneWeight[3] = currentVertexData.boneWeights[3];

			mesh.Vertices[i].boneNo[0] = currentVertexData.boneIndices[0];
			mesh.Vertices[i].boneNo[1] = currentVertexData.boneIndices[1];
			mesh.Vertices[i].boneNo[2] = currentVertexData.boneIndices[2];
			mesh.Vertices[i].boneNo[3] = currentVertexData.boneIndices[3];

			break;
		}
		case PMXVertexWeight::SDEF:
		{
			mesh.Vertices[i].boneWeight[0] = currentVertexData.boneWeights[0];

			mesh.Vertices[i].SdefC = currentVertexData.sdefC;
			mesh.Vertices[i].SdefR0 = currentVertexData.sdefR0;
			mesh.Vertices[i].SdefR1 = currentVertexData.sdefR1;
			mesh.Vertices[i].boneNo[0] = currentVertexData.boneIndices[0];
			mesh.Vertices[i].boneNo[1] = currentVertexData.boneIndices[1];
			XMStoreFloat4(&mesh.Vertices[i].Q0, XMQuaternionRotationMatrix(allNodes[currentVertexData.boneIndices[0]]->GetGlobalTransform()));
			XMStoreFloat4(&mesh.Vertices[i].Q1, XMQuaternionRotationMatrix(allNodes[currentVertexData.boneIndices[1]]->GetGlobalTransform()));
			break;
		}
		case PMXVertexWeight::QDEF:
		{
			mesh.Vertices[i].boneNo[0] = currentVertexData.boneIndices[0];
			break;
		}
		default:
			break;
		}
	}
}

//モーフのマテリアル更新
void PmxModel::MorphMaterial()
{
	size_t bufferSize = sizeof(Material);
	bufferSize = (bufferSize + 0xff) & ~0xff;


	char* mappedMaterialPtr = materialMap;
	for(int i = 0; i < pmxData.materials.size(); i++)
	{
		PMXMaterial& material = pmxData.materials[i];

		Material* uploadMat = nullptr;
		uploadMat = reinterpret_cast<Material*>(mappedMaterialPtr);
		if (uploadMat == nullptr) return;

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

//モーフのボーン更新
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

//ヒープの初期化
bool PmxModel::ModelHeapInit()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 4 + pmxData.materials.size() * 4;
	
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&descHeapDesc,
		IID_PPV_ARGS(_modelHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}

void PmxModel::Update(bool isStart)
{
	world =
		XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	*worldMatrix = world;
	UpdateAnimation(isStart);
}

//モデルの描画
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
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
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
