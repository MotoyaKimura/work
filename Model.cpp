#include "Model.h"
#include "Wrapper.h"

using namespace DirectX;

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
			int numIndex = numPolygon * 3;
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


Model::Model(std::shared_ptr<Wrapper> dx) : _dx(dx)
{
}

bool Model::Init(std::string filePath)
{
	DirectX::XMFLOAT3 vertices[] =
	{
		{-1.0f, -1.0f, 0.0f},
		{-1.0f, 1.0f, 0.0f},
		{1.0f, -1.0f, 0.0f}
	};

	if(!LoadModel(filePath)) return false;

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = m_meshParts[0].vertexBuffer.size() * sizeof(SVertex);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	auto result =_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(vertexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	SVertex* vertMap = nullptr;
	result = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(result)) return false;

	std::copy(std::begin(m_meshParts[0].vertexBuffer), std::end(m_meshParts[0].vertexBuffer), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = m_meshParts[0].vertexBuffer.size() * sizeof(SVertex);
	vbView.StrideInBytes = sizeof(SVertex);


	return true;
}



void Model::Update()
{
}

void Model::Draw()
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	_dx->GetCommandList() ->IASetVertexBuffers(0, 1, &vbView);
	

	_dx->GetCommandList()->DrawInstanced(vertexNum, 1, 0, 0);
}

Model::~Model()
{
}