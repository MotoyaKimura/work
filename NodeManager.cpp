#include "NodeManager.h"
#include "BoneNode.h"
#include <iostream>

using namespace DirectX;

//ボーンノード管理クラス
NodeManager::NodeManager()
{
}

NodeManager::~NodeManager()
{
}

//ボーンノードの初期化
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

		const PMXBone& currentPmxBone = bones[index];

		bool appendRotate = ((uint16_t)currentPmxBone.boneFlag & (uint16_t)PMXBoneFlags::AppendRotate);
		bool appendTranslate = ((uint16_t)currentPmxBone.boneFlag & (uint16_t)PMXBoneFlags::AppendTranslate);
		currentBoneNode->SetEnableAppendRotate(appendRotate);
		currentBoneNode->SetEnableAppendTranslate(appendTranslate);
		if ((appendRotate || appendTranslate) && currentPmxBone.appendBoneIndex < _boneNodeByIdx.size())
		{
			if(index > currentPmxBone.appendBoneIndex)
			{
				bool appendLocal = (uint16_t)currentPmxBone.boneFlag & (uint16_t)PMXBoneFlags::AppendLocal;
				BoneNode* appendBoneNode = _boneNodeByIdx[currentPmxBone.appendBoneIndex];
				currentBoneNode->SetEnableAppendLocal(appendLocal);
				currentBoneNode->SetAppendBoneNode(appendBoneNode);
				currentBoneNode->SetAppendWeight(currentPmxBone.appendWeight);
			}
		}

		if(((uint16_t)currentPmxBone.boneFlag & (uint16_t)PMXBoneFlags::IK) && currentPmxBone.ikTargetBoneIndex < _boneNodeByIdx.size())
		{
			BoneNode* targetNode = _boneNodeByIdx[currentPmxBone.ikTargetBoneIndex];
			unsigned int iterationCount = currentPmxBone.ikIterationCount;
			float limitAngle = currentPmxBone.ikLimit;

			_ikSolvers.push_back(new IKSolver(currentBoneNode, targetNode, iterationCount, limitAngle));

			IKSolver* solver = _ikSolvers[_ikSolvers.size() - 1];

			for(const PMXIKLink& ikLink : currentPmxBone.ikLinks)
			{
				if (ikLink.ikBoneIndex < 0 || ikLink.ikBoneIndex >= _boneNodeByIdx.size())
				{
					continue;
				}

				BoneNode* linkNode = _boneNodeByIdx[ikLink.ikBoneIndex];
				if(ikLink.enableLimit == true)
				{
					solver->AddIKChain(linkNode, ikLink.enableLimit, ikLink.LimitMin, ikLink.LimitMax);
				}
				else
				{
					solver->AddIKChain(
						linkNode, 
						ikLink.enableLimit, 
						XMFLOAT3(0.5f, 0.0f, 0.0f), 
						XMFLOAT3(180.0f, 0.0f, 0.0f));
				}
				linkNode->SetIKEnable(true);
			}
			currentBoneNode->SetIKSolver(solver);
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

//キーフレームをソート
void NodeManager::SortKey()
{
	for (int index = 0; index < _boneNodeByIdx.size(); index++)
	{
		BoneNode* currentBoneNode = _boneNodeByIdx[index];
		currentBoneNode->SortAllKeys();
		_duration = std::max<unsigned int>(_duration, currentBoneNode->GetMaxFrameNo());
	}
}

//アニメーション更新前処理
void NodeManager::BeforeUpdateAnimation()
{
	for (BoneNode* curNode : _boneNodeByIdx)
	{
		curNode->SetMorphPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		curNode->SetMorphRotation(XMMatrixIdentity());
	}
}

//アニメーション更新
void NodeManager::UpdateAnimation(unsigned int frameNo)
{
	
	for(BoneNode* curNode : _boneNodeByIdx)
	{
		curNode->AnimateMotion(frameNo);
		curNode->AnimateIK(frameNo);
		curNode->UpdateLocalTransform();
	}

	if(_boneNodeByIdx.size() > 0)
	{
		_boneNodeByIdx[0]->UpdateGlobalTransform();
	}
	int i = 0;
	for(BoneNode* curNode : _sortedNodes)
	{
	
		if (curNode->GetAppendBoneNode() != nullptr)
		{
			curNode->UpdateAppendTransform();
			curNode->UpdateGlobalTransform();
		}
		IKSolver* curSolver = curNode->GetIKSolver();
		if(curSolver != nullptr)
		{
			curSolver->Solve();
			curNode->UpdateGlobalTransform();
		}
	}
}

//名前からボーンノードを取得
BoneNode* NodeManager::GetBoneNodeByName(std::wstring& name) const
{
	auto index = _boneNodeByName.find(name);
	if (index != _boneNodeByName.end())
	{
		return index->second;
	}
	return nullptr;
}

//解放
void NodeManager::Dispose()
{
	for (BoneNode* curNode : _boneNodeByIdx)
	{
		delete curNode;
	}
}
