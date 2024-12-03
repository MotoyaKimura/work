#include "NodeManager.h"
#include "BoneNode.h"
#include <iostream>

using namespace DirectX;

void NodeManager::Init(const std::vector<PMXBone>& bones)
{
	_boneNodeByIdx.resize(bones.size());
	_sortedNodes.resize(bones.size());

	for (int index = 0; index < bones.size(); index++)
	{
		const PMXBone& currentBoneData = bones[index];
		_boneNodeByIdx[index] = new BoneNode(index, currentBoneData);
		_boneNodeByName[_boneNodeByIdx[index]->GetName() + L'\0'] = _boneNodeByIdx[index];
		_sortedNodes[index] = _boneNodeByIdx[index];
	}

	for (int index = 0; index < _boneNodeByIdx.size(); index++)
	{
		BoneNode* currentBoneNode = _boneNodeByIdx[index];

		unsigned int parentBoneIndex = currentBoneNode->GetParentBoneIndex();
		if (parentBoneIndex != 65535 && _boneNodeByIdx.size() > parentBoneIndex)
		{
			currentBoneNode->SetParentBoneNode(_boneNodeByIdx[currentBoneNode->GetParentBoneIndex()]);
		}
	}

	for (int index = 0; index < _boneNodeByIdx.size(); index++)
	{
		BoneNode* currentBoneNode = _boneNodeByIdx[index];

		if(currentBoneNode->GetParentBoneNode() == nullptr)
		{
			continue;
		}

		XMVECTOR pos = XMLoadFloat3(&bones[currentBoneNode->GetBoneIndex()].position);
		XMVECTOR parentPos = XMLoadFloat3(&bones[currentBoneNode->GetParentBoneIndex()].position);

		XMFLOAT3 resultPos;
		XMStoreFloat3(&resultPos, pos - parentPos);

		currentBoneNode->SetPosition(resultPos);
	}

	std::stable_sort(_sortedNodes.begin(), _sortedNodes.end(),
		[](const BoneNode* left, const BoneNode* right)
		{
			return left->GetDeformDepth() < right->GetDeformDepth();
		});
}

void NodeManager::SortKey()
{
	for (int index = 0; index < _boneNodeByIdx.size(); index++)
	{
		BoneNode* currentBoneNode = _boneNodeByIdx[index];
		currentBoneNode->SortAllKeys();
		_duration = std::max<unsigned int>(_duration, currentBoneNode->GetMaxFrameNo());
	}
}

void NodeManager::UpdateAnimation(unsigned int frameNo)
{
	
	for(BoneNode* curNode : _boneNodeByIdx)
	{
		curNode->AnimateMotion(frameNo);
		curNode->UpdateLocalTransform();
	}

	if(_boneNodeByIdx.size() > 0)
	{
		_boneNodeByIdx[0]->UpdateGlobalTransform();
	}
}

BoneNode* NodeManager::GetBoneNodeByName(std::wstring& name) const
{
	auto index = _boneNodeByName.find(name);
	if (index != _boneNodeByName.end())
	{
		return index->second;
	}
	return nullptr;
}

void NodeManager::Dispose()
{
	for (BoneNode* curNode : _boneNodeByIdx)
	{
		delete curNode;
	}
}

NodeManager::NodeManager()
{
}

NodeManager::~NodeManager()
{
}