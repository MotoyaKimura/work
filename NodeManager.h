#pragma once
#include <d3dx12.h>
#include "PmxModel.h"
#include <unordered_map>
#include "IKSolver.h"

class BoneNode;
class NodeManager
{

public:
	NodeManager();
	~NodeManager();

	
	void Init(const std::vector<PMXBone>& bones);
	void SortKey();

	BoneNode* GetBoneNodeByIndex(int index) const { return _boneNodeByIdx[index]; }
	BoneNode* GetBoneNodeByName(std::wstring& name) const;
	

	const std::vector<BoneNode*>& GetAllNodes() const { return _boneNodeByIdx; }

	void BeforeUpdateAnimation();
	void UpdateAnimation(unsigned int frameNo);

	void Dispose();

	void SetDuration(unsigned int duration) { _duration = duration; }
	unsigned int _duration = 0;
private:
	std::unordered_map<std::wstring, BoneNode*> _boneNodeByName;
	std::vector<BoneNode*> _boneNodeByIdx;
	std::vector<BoneNode*> _sortedNodes;

	std::vector<IKSolver*> _ikSolvers;
};