#include "Model.h"
#include <iostream>
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

bool Model::Load(std::string filePath)
{
	if (filePath == "") return false;

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
	Meshes.clear();
	Meshes.resize(pScene->mNumMeshes);
	
	for(size_t i = 0; i < Meshes.size(); ++i)
	{
		const auto pMesh = pScene->mMeshes[i];
		ParseMesh(Meshes[i], pMesh);
	}


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
	dstMesh.MaterialId = pSrcMesh->mMaterialIndex;
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
		aiColor3D color(0.0f, 0.0f, 0.0f);

		if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		{
			dstMaterial.Diffuse = XMFLOAT3(color.r, color.g, color.b);
		}
		else
		{
			dstMaterial.Diffuse = XMFLOAT3(0.5f, 0.5f, 0.5f);
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
			dstMaterial.Specular = XMFLOAT3(color.r, color.g, color.b);
		}
		else
		{
			dstMaterial.Specular = XMFLOAT3(0.0f, 0.0f, 0.0f);
		}
	}

	{
		float shininess;
		if (pSrcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
		{
			dstMaterial.Shininess = shininess;
		}
		else
		{
			dstMaterial.Shininess = 0.0f;
		}
	}

	{
		/*aiString path;
		if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
		{
			dstMaterial.DiffuseMap = path.C_Str();
		}
		else
		{
			dstMaterial.DiffuseMap.clear();
		}*/
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

	size_t idx = 0;
	for(size_t i = 0; i < Meshes.size(); ++i)
	{
		for (size_t j = 0; j < Meshes[i].Vertices.size(); ++j)
		{
			vertMap[idx + j] = Meshes[i].Vertices[j];
		}
		idx += Meshes[i].Vertices.size();
	}
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

	size_t idx = 0;
	for (size_t i = 0; i < Meshes.size(); ++i)
	{
		for (size_t j = 0; j < Meshes[i].Indices.size(); ++j)
		{
			indexMap[idx + j] = Meshes[i].Indices[j];
		}
		idx += Meshes[i].Indices.size();
	}
	//std::copy(std::begin(Meshes[0].Indices), std::end(Meshes[0].Indices), indexMap);
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