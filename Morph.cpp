#include "Morph.h"

//モーフクラス
Morph::Morph()
{
}

Morph::~Morph()
{
}

//モーフの位置の初期化
void Morph::SetPositionMorph(std::vector<PMXMorph::PositionMorph> pmxPositionMorphs)
{
	_positionMorphData.resize(pmxPositionMorphs.size());
	for (int i = 0; i < pmxPositionMorphs.size(); i++)
	{
		_positionMorphData[i].position = pmxPositionMorphs[i].position;
		_positionMorphData[i].vertexIndex = pmxPositionMorphs[i].vertexIndex;
	}
}

//モーフのUVの初期化
void Morph::SetUVMorph(std::vector<PMXMorph::UVMorph> pmxUVMorphs)
{
	_uvMorphData.resize(pmxUVMorphs.size());
	for (int i = 0; i < pmxUVMorphs.size(); i++)
	{
		_uvMorphData[i].uv = pmxUVMorphs[i].uv;
		_uvMorphData[i].vertexIndex = pmxUVMorphs[i].vertexIndex;
	}
}

//モーフのマテリアルの初期化
void Morph::SetMaterialMorph(std::vector<PMXMorph::MaterialMorph> pmxMaterialMorphs)
{
	_materialMorphData.resize(pmxMaterialMorphs.size());

	for (int i = 0; i < pmxMaterialMorphs.size(); i++)
	{
		_materialMorphData[i] = pmxMaterialMorphs[i];
	}
}

//モーフのボーンの初期化
void Morph::SetBoneMorph(std::vector<PMXMorph::BoneMorph> pmxBoneMorphs)
{
	_boneMorphData.resize(pmxBoneMorphs.size());

	for (int i = 0; i < pmxBoneMorphs.size(); i++)
	{
		_boneMorphData[i] = pmxBoneMorphs[i];
	}
}

//モーフのグループの初期化,モーフタイプが複数のとき
void Morph::SetGroupMorph(std::vector<PMXMorph::GroupMorph> pmxGroupMorphs)
{
	_groupMorphData.resize(pmxGroupMorphs.size());

	for (int i = 0; i < pmxGroupMorphs.size(); i++)
	{
		_groupMorphData[i] = pmxGroupMorphs[i];
	}
}



