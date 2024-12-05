#pragma once
#include "PmxModel.h"

class Morph;
struct MaterialMorphData
{
	float weight;
	PMXMorph::MaterialMorph::OpType opType;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT3 specular;
	float specularPower;
	DirectX::XMFLOAT3 ambient;
	DirectX::XMFLOAT4 edgeColor;
	float edgeSize;
	DirectX::XMFLOAT4 textureFactor;
	DirectX::XMFLOAT4 sphereTextureFactor;
	DirectX::XMFLOAT4 toonTextureFactor;
};

struct BoneMorphData
{
	float weight;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 quaternion;
};

class MorphManager
{
public:
	MorphManager();
	~MorphManager();

	void Init(const std::vector<PMXMorph>& pmxMorphs, 
		const std::vector<VMDMorph>& vmdMorphs,
		unsigned int vertexCount, 
		unsigned int materialCount,
		unsigned int boneCount
	);

	void Animate(unsigned int frame);

	const DirectX::XMFLOAT3& GetMorphVertexPosition(unsigned int index) const { return _morphVertexPosition[index]; }
	const DirectX::XMFLOAT4& GetMorphUV(unsigned int index) const { return _morphUV[index]; }
	const MaterialMorphData& GetMorphMaterial(unsigned int index) const { return _morphMaterial[index]; }
	const BoneMorphData& GetMorphBone(unsigned int index) const { return _morphBone[index]; }

private:
	void AnimateMorph(Morph& morph, float weight = 1.0f);
	void AnimatePositionMorph(Morph& morph, float weight);
	void AnimateUVMorph(Morph& morph, float weight);
	void AnimateMaterialMorph(Morph& morph, float weight);
	void AnimateBoneMorph(Morph& morph, float weight);
	void AnimateGroupMorph(Morph& morph, float weight);

	void ResetMorphData();

	std::vector<Morph> _morphs;
	std::unordered_map<std::wstring, Morph*> _morphByName;

	std::vector<VMDMorph> _morphKeys;
	std::unordered_map<std::wstring, std::vector<VMDMorph*>> _morphKeyByName;

	std::vector<DirectX::XMFLOAT3> _morphVertexPosition;
	std::vector<DirectX::XMFLOAT4> _morphUV;
	std::vector<MaterialMorphData> _morphMaterial;
	std::vector<BoneMorphData> _morphBone;
};