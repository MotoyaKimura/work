#include "Model.h"
#include <iostream>

#include "Camera.h"
#include "Wrapper.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Texture.h"
#pragma  comment(lib, "DirectXTex.lib")
#pragma  comment(lib, "assimp-vc143-mtd.lib")

using namespace DirectX;

bool Model::Load(std::string filePath)
{
	
	return true;
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

Microsoft::WRL::ComPtr<ID3D12Resource> Model::CreateBuffer(int width, size_t num)
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(((width * num + 0xff) & ~0xff))  ;
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
	_worldBuff = CreateBuffer(sizeof(XMMATRIX), 1 + boneMatricesNum);
	auto result = _worldBuff->Map(
		0, 
		nullptr, 
		(void**)&worldMatrix
	);
	if (FAILED(result)) return false;
	world = XMMatrixIdentity();
	worldMatrix[0] = world;
	if (boneMatricesNum > 0)
	{
		copy(
			std::begin(boneMatrices),
			std::end(boneMatrices),
			worldMatrix + 1
		);
	}

	if (boneMatricesNum > 0)
	{
		_invTransBuff = CreateBuffer(sizeof(XMMATRIX), boneMatricesNum);
		result = _invTransBuff->Map(
			0,
			nullptr,
			(void**)&invTransMatrix
		);
		if (FAILED(result)) return false;

		copy(
			std::begin(boneMatrices),
			std::end(boneMatrices),
			invTransMatrix
		);
	}
	else
	{
		_invTransBuff = CreateBuffer(sizeof(XMMATRIX), 1);
		result = _invTransBuff->Map(
			0,
			nullptr,
			(void**)&invTransMatrix
		);
		if (FAILED(result)) return false;
	}
	


	SetCBV(_camera->GetSceneTransBuff());
	SetCBV(_worldBuff);
	SetCBV(_invTransBuff);
	return true;
}



bool Model::MaterialBuffInit()
{
	int materialBuffSize = sizeof(Material);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	_materialBuff = CreateBuffer(materialBuffSize, Materials.size());
	//Material* materialMap = nullptr;
	
	auto result = _materialBuff->Map(
		0, 
		nullptr, 
		(void**)&materialMap
	);
	if (FAILED(result)) return false;

	for (auto& material : Materials)
	{
		Material* uploadMat = reinterpret_cast<Material*>(materialMap);
		uploadMat->diffuse = material.diffuse;
		uploadMat->specular = material.specular;
		uploadMat->specularPower = material.specularPower;
		uploadMat->ambient = material.ambient;
		materialMap += materialBuffSize;
	}
	//_materialBuff->Unmap(0, nullptr);
	/*std::copy(
		std::begin(Materials), 
		std::end(Materials),
		materialMap
	);*/
	SetCBV(_materialBuff);

	for(int i = 0; i < Materials.size(); ++i)
	{
		if(mTextureResources[i] == nullptr)
		{
			std::shared_ptr<Texture> whiteTex;
			whiteTex.reset(new Texture(_dx, L""));
			whiteTex->WhileTextureInit();
			SetSRV(whiteTex->GetTexBuff(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			SetSRV(mTextureResources[i].Get(), mTextureResources[i]->GetDesc().Format);
		}

		if (mToonResources[i] == nullptr)
		{
			std::shared_ptr<Texture> gradTex;
			gradTex.reset(new Texture(_dx, L""));
			gradTex->WhileTextureInit();
			SetSRV(gradTex->GetTexBuff(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			SetSRV(mToonResources[i].Get(), mToonResources[i]->GetDesc().Format);
		}

		if (mSphereTextureResources[i] == nullptr)
		{
			std::shared_ptr<Texture> sphereTex;
			sphereTex.reset(new Texture(_dx, L""));
			sphereTex->BlackTextureInit();
			SetSRV(sphereTex->GetTexBuff(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			SetSRV(mSphereTextureResources[i].Get(), mSphereTextureResources[i]->GetDesc().Format);
		}
	}

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

	//最初の３つのディスクリプタはカメラ、ワールド、ライト深度
	for (int i = 0; i < 3; ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = cbvBuffs[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(cbvBuffs[i]->GetDesc().Width);
		auto handle = _modelHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * i;
		_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
	for (int i = 0; i < 1; ++i)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = srvBuffs[i].second;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		auto handle = _modelHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (i + 3);
		_dx->GetDevice()->CreateShaderResourceView(srvBuffs[i].first.Get(), &srvDesc, handle);
	}

	//ここからはマテリアル情報
	auto materialBuffSize = sizeof(Material);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = cbvBuffs[3]->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = materialBuffSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto matDescHeapH = _modelHeap->GetCPUDescriptorHandleForHeapStart();
	matDescHeapH.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
	auto incSize = _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (int i = 0; i < Materials.size(); ++i)
		{
			_dx->GetDevice()->CreateConstantBufferView(&matCBVDesc, matDescHeapH);

			matDescHeapH.ptr += incSize;
			matCBVDesc.BufferLocation += materialBuffSize;

			srvDesc.Format = srvBuffs[1 + 3 * i + 0].second;
			_dx->GetDevice()->CreateShaderResourceView(srvBuffs[1 + 3 * i + 0].first.Get(), &srvDesc, matDescHeapH);
			matDescHeapH.ptr += incSize;

			srvDesc.Format = srvBuffs[1 + 3 * i + 1].second;
			_dx->GetDevice()->CreateShaderResourceView(srvBuffs[1 + 3 * i + 1].first.Get(), &srvDesc, matDescHeapH);
			matDescHeapH.ptr += incSize;

			srvDesc.Format = srvBuffs[1 + 3 * i + 2].second;
			_dx->GetDevice()->CreateShaderResourceView(srvBuffs[1 + 3 * i + 2].first.Get(), &srvDesc, matDescHeapH);
			matDescHeapH.ptr += incSize;
		}
}


Model::Model(
	std::shared_ptr<Wrapper> dx, 
	std::shared_ptr<Camera> camera,
	std::string filePath
) : _dx(dx), _camera(camera), _pos(0, 0, 0), _rotater(0, 0, 0), _filePath(filePath)
{
}

bool Model::Init()
{
	//Load(_filePath);
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
	
	SetViews();
	return true;
}





void Model::Move(float x, float y, float z)
{
	_pos.x += x;
	_pos.y += y;
	_pos.z += z;
	_aabb._xMax += x;
	_aabb._xMin += x;
	_aabb._yMax += y;
	_aabb._yMin += y;
	_aabb._zMax += z;
	_aabb._zMin += z;
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