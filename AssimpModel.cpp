#include "AssimpModel.h"
#include "Wrapper.h"
#include "Texture.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma  comment(lib, "assimp-vc143-mtd.lib")

using namespace DirectX;

bool AssimpModel::Load(std::string filePath)
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
	const auto pMesh = pScene->mMeshes[0];
	
	ParseMesh(mesh, pMesh);

	Materials.clear();
	Materials.resize(pScene->mNumMaterials);
	mTextureResources.resize(pScene->mNumMaterials);
	mToonResources.resize(pScene->mNumMaterials);
	mSphereTextureResources.resize(pScene->mNumMaterials);

	for (size_t i = 0; i < Materials.size(); ++i)
	{
		const auto pMaterial = pScene->mMaterials[i];
		ParseMaterial(Materials[i], pMaterial);
	}

	pScene = nullptr;

	return true;
}

//���b�V�����̎擾
void AssimpModel::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh)
{
	aiVector3D aabbMax = pSrcMesh->mAABB.mMax;
	aiVector3D aabbMin = pSrcMesh->mAABB.mMin;
	_aabb._xMax = aabbMax.x;
	_aabb._yMax = aabbMax.y;
	_aabb._zMax = aabbMax.z;
	_aabb._xMin = aabbMin.x;
	_aabb._yMin = aabbMin.y;
	_aabb._zMin = aabbMin.z;

	aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	dstMesh.Vertices.resize(pSrcMesh->mNumVertices);
	vertexNum += pSrcMesh->mNumVertices;
	//�e���_�����擾�i�{�[���ƃ��[�t�͖��Ή��j
	for (auto i = 0u; i < pSrcMesh->mNumVertices; ++i)
	{
		auto pPosition = &(pSrcMesh->mVertices[i]);
		auto pNormal = &(pSrcMesh->mNormals[i]);
		auto pTexCoord = pSrcMesh->HasTextureCoords(0) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;

		dstMesh.Vertices[i] = MeshVertex(
			XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
			XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
			XMFLOAT2(pTexCoord->x, pTexCoord->y),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
			0, 0, 0, 0,
			0.0f, 0.0f, 0.0f, 0.0f, 
			100
		);
		boneMatricesNum = 0;
	}

	//�C���f�b�N�X�����擾
	dstMesh.Indices.resize(pSrcMesh->mNumFaces * 3);
	numIndex += pSrcMesh->mNumFaces * 3;
	for (auto i = 0u; i < pSrcMesh->mNumFaces; ++i)
	{
		const auto& face = pSrcMesh->mFaces[i];
		assert(face.mNumIndices == 3);

		dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
		dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
		dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
	}
}

//�}�e���A�����̎擾
void AssimpModel::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial)
{
	{
		aiColor4D color(0.0f, 0.0f, 0.0f, 0.0f);

		//�g�U���ːF�̎擾
		if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		{
			dstMaterial.diffuse = XMFLOAT4(color.r, color.g, color.b, color.a);
		}
		else
		{
			dstMaterial.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		}

		//�g�U���˃e�N�X�`���̎擾
		aiString path;
		if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
		{
			std::string str = path.C_Str();
			std::wstring strW = _dx->GetWideStringFromString(str);
			strW = L"texture/" + strW;
			strW.pop_back();
			std::shared_ptr<Texture> texture;
			texture.reset(new Texture(_dx));
			texture->Init(strW);
			mTextureResources[Materials.size() - 1] = texture->GetTexBuff();
		}
	}

	//�X�y�L�������ːF�̎擾
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
	//�X�y�L�������ˌW���̎擾
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
	//�A���r�G���g���ːF�̎擾
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

//�q�[�v�̏�����
bool AssimpModel::ModelHeapInit()
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

void AssimpModel::Update(bool isStart)
{
	world = XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	*worldMatrix = world;
}

void AssimpModel::Draw()
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_dx->GetCommandList()->IASetVertexBuffers(0, 1, &vbView);
	_dx->GetCommandList()->IASetIndexBuffer(&ibView);

	ID3D12DescriptorHeap* heaps[] = { _modelHeap.Get() };

	//���߂�4�̓��f���̃J�����A�i���[���h�E�{�[���j�Ainv�{�[���A���C�g�[�x�e�N�X�`��
	_dx->GetCommandList()->SetDescriptorHeaps(1, heaps);
	auto handle = _modelHeap->GetGPUDescriptorHandleForHeapStart();
	_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
		0,
		handle);

	//��������̓}�e���A�����Ƃ̏����i�}�e���A���A�}�e���A���e�N�X�`���j
	auto incSize = _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
	unsigned int idxOffset = 0;
	for (int i = 0; i < Materials.size(); i++)
	{
		unsigned int numVertex = 0;
		numVertex = numIndex;

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

AssimpModel::AssimpModel(std::shared_ptr<Wrapper> dx,
	std::shared_ptr<Camera> camera,
	std::string filePath
) : Model(dx, camera, filePath), _dx(dx), _camera(camera)
{
	Load(filePath);
}

AssimpModel::~AssimpModel()
{
}