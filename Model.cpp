#include "Model.h"
#include "Texture.h"
#include <iostream>
#include "Camera.h"
#include "Wrapper.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma  comment(lib, "DirectXTex.lib")
#pragma  comment(lib, "assimp-vc143-mtd.lib")

using namespace DirectX;

//���f���N���X
Model::Model(
	std::shared_ptr<Wrapper> dx,
	std::shared_ptr<Camera> camera,
	std::string filePath
) : _dx(dx),
_camera(camera),
_pos(0, 0, 0),
_rotater(0, 0, 0),
_filePath(filePath)
{
}

Model::~Model()
{
}

//���f���̏�����
bool Model::Init()
{
	if (!VertexInit()) return false;
	if (!IndexInit()) return false;
	if (!TransBuffInit()) return false;
	if (!MaterialBuffInit()) return false;
	world =
		XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	*worldMatrix = world;
	if (!SetViews()) return false;
	return true;
}

//���_�o�b�t�@�[�A���_�r���[�̏�����
bool Model::VertexInit()
{
	CreateBuffer(vertexBuffer, vertexNum * sizeof(MeshVertex));
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(result)) return false;
	std::copy(std::begin(mesh.Vertices), std::end(mesh.Vertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertexNum * sizeof(MeshVertex);
	vbView.StrideInBytes = sizeof(MeshVertex);
	return true;
}

//�C���f�b�N�X�o�b�t�@�[�A�C���f�b�N�X�r���[�̏�����
bool Model::IndexInit()
{
	if (!CreateBuffer(indexBuffer, numIndex * sizeof(uint32_t))) return false;
	uint32_t* indexMap = nullptr;
	auto result = indexBuffer->Map(0, nullptr, (void**)&indexMap);
	if (FAILED(result)) return false;
	
	std::copy(std::begin(mesh.Indices), std::end(mesh.Indices), indexMap);

	indexBuffer->Unmap(0, nullptr);
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = numIndex * sizeof(uint32_t);

	return true;
}

//���f���ϊ��o�b�t�@�[�̏�����
bool Model::TransBuffInit()
{
	//�i���[���h�E�{�[���j�o�b�t�@�[
	if (!CreateBuffer(_worldBuff, 
		(sizeof(XMMATRIX) * (1 + boneMatricesNum) + 0xff) & ~0xff))
		return false;
	auto result = _worldBuff->Map(
		0, 
		nullptr, 
		(void**)&worldMatrix
	);
	if (FAILED(result)) return false;
	world = XMMatrixIdentity();
	worldMatrix[0] = world;

	//�{�[���t�s��o�b�t�@�[
	if (!CreateBuffer(_invTransBuff, 
		(sizeof(XMMATRIX) * (1 +boneMatricesNum) + 0xff) & ~0xff))
		return false;
	result = _invTransBuff->Map(
		0,
		nullptr,
		(void**)&invTransMatrix
	);
	if (FAILED(result)) return false;

	//�{�[���s��̏�����
	if (boneMatricesNum > 0)
	{
		copy(
			std::begin(boneMatrices),
			std::end(boneMatrices),
			worldMatrix + 1
		);
		copy(
			std::begin(boneMatrices),
			std::end(boneMatrices),
			invTransMatrix
		);
	}

	SetCBV(_camera->GetSceneTransBuff());
	SetCBV(_worldBuff);
	SetCBV(_invTransBuff);
	return true;
}

//�}�e���A���o�b�t�@�[�̏�����
bool Model::MaterialBuffInit()
{
	int materialBuffSize = sizeof(Material);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	
	if (!CreateBuffer(_materialBuff, materialBuffSize * Materials.size())) return false;
	
	auto result = _materialBuff->Map(
		0, 
		nullptr, 
		(void**)&materialMap
	);
	if (FAILED(result)) return false;

	//�}�e���A�����̃R�s�[
	char* mappedMaterialPtr = materialMap;
	for (auto& material : Materials)
	{
		Material* uploadMat = reinterpret_cast<Material*>(mappedMaterialPtr);
		uploadMat->diffuse = material.diffuse;
		uploadMat->specular = material.specular;
		uploadMat->specularPower = material.specularPower;
		uploadMat->ambient = material.ambient;
		mappedMaterialPtr += materialBuffSize;
	}
	_materialBuff->Unmap(0, nullptr);
	SetCBV(_materialBuff);

	//�e�N�X�`���̏�����
	for(int i = 0; i < Materials.size(); ++i)
	{
		//�f�B�t���[�Y�e�N�X�`�����Ȃ��ꍇ�͔��e�N�X�`�����g�p
		if(mTextureResources[i] == nullptr)
		{
			std::shared_ptr<Texture> whiteTex(new Texture(_dx, L""));
			if (!whiteTex->WhileTextureInit()) return false;
			SetSRV(whiteTex->GetTexBuff(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			SetSRV(mTextureResources[i].Get(), mTextureResources[i]->GetDesc().Format);
		}

		//�g�D�[���e�N�X�`�����Ȃ��ꍇ�̓O���f�[�V�����e�N�X�`�����g�p
		if (mToonResources[i] == nullptr)
		{
			std::shared_ptr<Texture> gradTex(new Texture(_dx, L""));
			if (!gradTex->WhileTextureInit()) return false;
			SetSRV(gradTex->GetTexBuff(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			SetSRV(mToonResources[i].Get(), mToonResources[i]->GetDesc().Format);
		}

		//�X�t�B�A�e�N�X�`�����Ȃ��ꍇ�͍��e�N�X�`�����g�p
		if (mSphereTextureResources[i] == nullptr)
		{
			std::shared_ptr<Texture> sphereTex(new Texture(_dx, L""));
			if (!sphereTex->BlackTextureInit()) return false;
			SetSRV(sphereTex->GetTexBuff(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			SetSRV(mSphereTextureResources[i].Get(), mSphereTextureResources[i]->GetDesc().Format);
		}
	}
	return true;
}

//SRV�o�b�t�@�[���ꎞ�I�ɂ��߂Ă����ꏊ
void Model::SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format)
{
	std::pair< Microsoft::WRL::ComPtr<ID3D12Resource>, DXGI_FORMAT> srvBuff = { buffer, format };
	srvBuffs.emplace_back(srvBuff);
}

//CBV�o�b�t�@�[���ꎞ�I�ɂ��߂Ă����ꏊ
void Model::SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer)
{
	cbvBuffs.emplace_back(buffer);
}

//�q�[�v�̏������A�r���[�̐ݒ�
bool Model::SetViews()
{
	if (!ModelHeapInit()) return false;

	//�ŏ��̂R�̃f�B�X�N���v�^�̓J�����A�i���[���h�E�{�[���j�Ainv�{�[��
	for (int i = 0; i < 3; ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = cbvBuffs[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(cbvBuffs[i]->GetDesc().Width);
		auto handle = _modelHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * i;
		_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
	//����1�̓��C�g�[�x�e�N�X�`��
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

	//��������̓}�e���A�����
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
			//�}�e���A�����̐ݒ�
			_dx->GetDevice()->CreateConstantBufferView(&matCBVDesc, matDescHeapH);

			matDescHeapH.ptr += incSize;
			matCBVDesc.BufferLocation += materialBuffSize;

			//�}�e���A���e�N�X�`���̐ݒ�
			//�f�B�t���[�Y
			srvDesc.Format = srvBuffs[1 + 3 * i + 0].second;
			_dx->GetDevice()->CreateShaderResourceView(srvBuffs[1 + 3 * i + 0].first.Get(), &srvDesc, matDescHeapH);
			matDescHeapH.ptr += incSize;

			//�g�D�[��
			srvDesc.Format = srvBuffs[1 + 3 * i + 1].second;
			_dx->GetDevice()->CreateShaderResourceView(srvBuffs[1 + 3 * i + 1].first.Get(), &srvDesc, matDescHeapH);
			matDescHeapH.ptr += incSize;

			//�X�t�B�A
			srvDesc.Format = srvBuffs[1 + 3 * i + 2].second;
			_dx->GetDevice()->CreateShaderResourceView(srvBuffs[1 + 3 * i + 2].first.Get(), &srvDesc, matDescHeapH);
			matDescHeapH.ptr += incSize;
		}
		return true;
}

//�o�b�t�@�[�̏�����
bool Model::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, int width)
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(width);
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		std::cout << "CreateBuffer Failed, width : " << width << std::endl;
		return false;
	}
	return true;
}

//���f���̈ړ�
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

//���f���̉�]
void Model::Rotate(float x, float y, float z)
{
	_rotater.x += x;
	_rotater.y += y;
	_rotater.z += z;
}

