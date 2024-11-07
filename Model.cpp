#include "Model.h"

#include <iostream>

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

template<class T>
void Model::LoadIndexBuffer(std::vector<T>& indices, int numIndex, FILE* fp)
{
	indices.resize(numIndex);
	for (int indexNo = 0; indexNo < numIndex; indexNo++) {
		T index;
		fread(&index, sizeof(index), 1, fp);
		indices[indexNo] = index - 1;	//todo maxのインデックスは1から開始しているので、-1する。
		//todo エクスポーターで減らすようにしましょう。
	}
}

std::string Model::LoadTextureFileName(FILE* fp)
{
	std::string fileName;
	std::uint32_t fileNameLen;
	fread(&fileNameLen, sizeof(fileNameLen), 1, fp);

	if (fileNameLen > 0) {
		char* localFileName = reinterpret_cast<char*>(malloc(fileNameLen + 1));
		//ヌル文字分も読み込むので＋１
		fread(localFileName, fileNameLen + 1, 1, fp);
		fileName = localFileName;
		free(localFileName);
	}

	return fileName;
}

void Model::BuildMaterial(SMaterial& tkmMat, FILE* fp, std::string filePath)
{
	//アルベドのファイル名をロード。
	tkmMat.albedoMapFileName = LoadTextureFileName(fp);
	//法線マップのファイル名をロード。
	tkmMat.normalMapFileName = LoadTextureFileName(fp);
	//スペキュラマップのファイル名をロード。
	tkmMat.specularMapFileName = LoadTextureFileName(fp);
	//リフレクションマップのファイル名をロード。
	tkmMat.reflectionMapFileName = LoadTextureFileName(fp);
	//屈折マップのファイル名をロード。
	tkmMat.refractionMapFileName = LoadTextureFileName(fp);

	std::string texFilePath = filePath;
	auto loadTexture = [&](
		std::string& texFileName,
		std::unique_ptr<char[]>& ddsFileMemory,
		unsigned int& fileSize,
		std::string& texFilePathDst
		) {
			int filePathLength = static_cast<int>(texFilePath.length());
			if (texFileName.length() > 0) {
				//モデルのファイルパスからラストのフォルダ区切りを探す。
				auto replaseStartPos = texFilePath.find_last_of('/');
				if (replaseStartPos == std::string::npos) {
					replaseStartPos = texFilePath.find_last_of('\\');
				}
				replaseStartPos += 1;
				auto replaceLen = filePathLength - replaseStartPos;
				texFilePath.replace(replaseStartPos, replaceLen, texFileName);
				//拡張子をddsに変更する。
				replaseStartPos = texFilePath.find_last_of('.') + 1;
				replaceLen = texFilePath.length() - replaseStartPos;
				texFilePath.replace(replaseStartPos, replaceLen, "dds");
				//テクスチャファイルパスを記憶しておく。
				texFilePathDst = texFilePath;
				std::wstring wstr = GetWideStringFromString(texFilePath);
				LoadFromDDSFile(wstr.c_str(), DDS_FLAGS_NONE, &metadata, scratchImage);

				//テクスチャをロード。
				FILE* texFileFp;
				fopen_s(&texFileFp, texFilePath.c_str(), "rb");
				if (texFileFp != nullptr) {
					//ファイルサイズを取得。
					fseek(texFileFp, 0L, SEEK_END);
					fileSize = ftell(texFileFp);
					fseek(texFileFp, 0L, SEEK_SET);

					ddsFileMemory = std::make_unique<char[]>(fileSize);
					fread(ddsFileMemory.get(), fileSize, 1, texFileFp);
					fclose(texFileFp);
				}
				else {

					MessageBoxA(nullptr, "テクスチャのロードに失敗しました。", "エラー", MB_OK);
					std::abort();
				}
			}
		};
	//テクスチャをロード。
	loadTexture(
		tkmMat.albedoMapFileName,
		tkmMat.albedoMap,
		tkmMat.albedoMapSize,
		tkmMat.albedoMapFilePath
	);
	loadTexture(
		tkmMat.normalMapFileName,
		tkmMat.normalMap,
		tkmMat.normalMapSize,
		tkmMat.normalMapFilePath
	);
	loadTexture(
		tkmMat.specularMapFileName,
		tkmMat.specularMap,
		tkmMat.specularMapSize,
		tkmMat.specularMapFilePath
	);
	loadTexture(
		tkmMat.reflectionMapFileName,
		tkmMat.reflectionMap,
		tkmMat.reflectionMapSize,
		tkmMat.reflectionMapFilePath
	);
	loadTexture(
		tkmMat.refractionMapFileName,
		tkmMat.refractionMap,
		tkmMat.refractionMapSize,
		tkmMat.refractionMapFilePath
	);
}


bool Model::LoadModel(std::string filePath)
{
	FILE* fp;

	fopen_s(&fp, filePath.c_str(), "rb");
	if (fp == nullptr) {
		MessageBoxA(nullptr, "tkmファイルが開けません。", "エラー", MB_OK);
		return false;
	}

	SHeader header;
	fread(&header, sizeof(header), 1, fp);
	if (header.version != VERSION) {
		//tkmファイルのバージョンが違う。
		MessageBoxA(nullptr, "tkmファイルのバージョンが異なっています。", "エラー", MB_OK);
	}

	m_meshParts.resize(header.numMeshParts);

	for (int meshPartsNo = 0; meshPartsNo < header.numMeshParts; meshPartsNo++) {

		SMesh& meshPart = m_meshParts[meshPartsNo];
		meshPart.isFlatShading = header.isFlatShading;

		SMeshePartsHeader meshPartsHeader;
		fread(&meshPartsHeader, sizeof(meshPartsHeader), 1, fp);

		meshPart.materials.resize(meshPartsHeader.numMaterial);
		vertexNum = meshPartsHeader.numVertex;

		for (unsigned int materialNo = 0; materialNo < meshPartsHeader.numMaterial; materialNo++) {
			auto& material = meshPart.materials[materialNo];
			BuildMaterial(material, fp, filePath);
		}

		meshPart.vertexBuffer.resize(meshPartsHeader.numVertex);
		//fread(meshPart.vertexBuffer.data(), meshPart.vertexBuffer.size(), 1, fp);
		
		for (unsigned int vertNo = 0; vertNo < meshPartsHeader.numVertex; vertNo++) {
			auto& vertex = meshPart.vertexBuffer[vertNo];
			fread(&vertex, sizeof(vertex), 1, fp);
		}

		if (meshPartsHeader.indexSize == 2) {
			//16bitのインデックスバッファ。
			meshPart.indexBuffer16Array.resize(meshPartsHeader.numMaterial);
		}
		else {
			//32bitのインデックスバッファ。
			meshPart.indexBuffer32Array.resize(meshPartsHeader.numMaterial);
		}

		for (unsigned int materialNo = 0; materialNo < meshPartsHeader.numMaterial; materialNo++) {
			//ポリゴン数をロード。
			int numPolygon;
			fread(&numPolygon, sizeof(numPolygon), 1, fp);
			//トポロジーはトライアングルリストオンリーなので、3を乗算するとインデックスの数になる。
			numIndex = numPolygon * 3;
			if (meshPartsHeader.indexSize == 2) {
				LoadIndexBuffer(
					meshPart.indexBuffer16Array[materialNo].indices,
					numIndex,
					fp
				);
			}
			else {
				LoadIndexBuffer(
					meshPart.indexBuffer32Array[materialNo].indices,
					numIndex,
					fp
				);
			}
		}
	}
	fclose(fp);
}

bool Model::Load(std::string filePath)
{
	if (filePath == "") return false;

	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;
	flag |= aiProcess_GenBoundingBoxes;

	auto pScene = importer.ReadFile(filePath.c_str(), flag);
	if (pScene == nullptr) return false;

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
			XMFLOAT3(-pPosition->x, pPosition->y, pPosition->z),
			XMFLOAT3(-pNormal->x, pNormal->y, pNormal->z),
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

bool Model::TextureInit()
{
	LoadFromWICFile(L"modelData/teapot/default.png", WIC_FLAGS_NONE, &metadata, scratchImage);
	//LoadFromWICFile(L"modelData/erato/erato-101.jpg", WIC_FLAGS_NONE, &metadata, scratchImage);
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapProp.CreationNodeMask = 0;
	heapProp.VisibleNodeMask = 0;
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = metadata.width;
	resDesc.Height = metadata.height;
	resDesc.DepthOrArraySize = metadata.arraySize;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(texBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	auto img = scratchImage.GetImage(0, 0, 0);
	result = texBuffer->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		img->rowPitch,
		img->slicePitch
	);

	
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	_mTransMap["texBuffer"] = mTransHeapNum++;
	_mTransMap["_lightDepthBuff"] = mTransHeapNum++;

	auto handle = _mTransHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _mTransMap["texBuffer"];
	_dx->GetDevice()->CreateShaderResourceView(
		texBuffer.Get(),
		&srvDesc,
		handle);

	/*srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	handle = _mTransHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _mTransMap["_lightDepthBuff"];
	_dx->GetDevice()->CreateShaderResourceView(
		_dx->GetLightDepthBuff().Get(),
		&srvDesc,
		handle);*/

	

	return true;
}

bool Model::MTransBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(world) + 0xff) & ~0xff);

	_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_mTransBuff.ReleaseAndGetAddressOf())
	);
	
	auto result = _mTransBuff->Map(0, nullptr, (void**)&mTransMatrix);
	if (FAILED(result)) return false;
	world = XMMatrixIdentity();
	*mTransMatrix = world;

	return true;
}

bool Model::MTransHeapInit()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 5;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&descHeapDesc,
		IID_PPV_ARGS(_mTransHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	_mTransMap["_sceneTransBuff"] = mTransHeapNum++;
	_mTransMap["_mTransBuff"] = mTransHeapNum++;
	
	

	return true;
}

bool Model::CreateMTransView()
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _dx->GetSceneTransBuff()->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_dx->GetSceneTransBuff()->GetDesc().Width);
	auto handle = _mTransHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _mTransMap["_sceneTransBuff"];
	_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);

	cbvDesc.BufferLocation = _mTransBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_mTransBuff->GetDesc().Width);
	handle = _mTransHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _mTransMap["_mTransBuff"];
	_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	return true;
}


bool Model::MaterialBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(Materials[0]) + 0xff) & ~0xff);

	_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_materialBuff.ReleaseAndGetAddressOf())
	);

	_mTransMap["_materialBuff"] = mTransHeapNum++;

	Material* materialMap = nullptr;
	auto result = _materialBuff->Map(0, nullptr, (void**)&materialMap);
	if (FAILED(result)) return false;
	
	std::copy(std::begin(Materials), std::end(Materials), materialMap);


	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _materialBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_materialBuff->GetDesc().Width);
	auto handle = _mTransHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _mTransMap["_materialBuff"];
	_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);

	return true;
}



Model::Model(std::shared_ptr<Wrapper> dx, std::string filePath) : _dx(dx), _pos(0, 0, 0), _rotater(0, 0, 0)
{
	Load(filePath);
}

bool Model::Init()
{
	if (!VertexInit()) return false;
	if (!IndexInit()) return false;
	if (!MTransBuffInit()) return false;
	if (!MTransHeapInit()) return false;
	if (!CreateMTransView()) return false;
	if (!MaterialBuffInit()) return false;
	if (!TextureInit()) return false;
	return true;
}



void Model::Update()
{
	angle = 0.001f;
	world =
		 XMMatrixRotationRollPitchYaw(_rotater.x, _rotater.y, _rotater.z)
		
		* XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	
	*mTransMatrix = world;
	
}

void Model::Draw(bool isShadow)
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_dx->GetCommandList() ->IASetVertexBuffers(0, 1, &vbView);
	_dx->GetCommandList()->IASetIndexBuffer(&ibView);

	ID3D12DescriptorHeap* heaps[] = { _mTransHeap.Get() };

	_dx->GetCommandList()->SetDescriptorHeaps(1, heaps);
	_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
		0,
		_mTransHeap->GetGPUDescriptorHandleForHeapStart());
	if (isShadow) {
		_dx->GetCommandList()->DrawIndexedInstanced(numIndex, 1, 0, 0, 0);
	}
	else
	{
		_dx->GetCommandList()->DrawIndexedInstanced(numIndex, 1, 0, 0, 0);
	}
	
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

DirectX::XMFLOAT3* Model::GetPos()
{
	return &_pos;
}


DirectX::XMFLOAT3* Model::GetRotate()
{
	return &_rotater;
}


Model::~Model()
{
}