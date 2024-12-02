#pragma once
#include <d3dx12.h>
#include "PmxModel.h"
#include "BoneNode.h"
#include <unordered_map>

class NodeManager
{
public:
	NodeManager();
	~NodeManager();

	void Init(const std::vector<PMXBone>& bones);
	void SortKey();

	BoneNode* GetBoneNodeByIndex(int index) const;
	BoneNode* GetBoneNodeByName(std::wstring& name) const;

	const std::vector<BoneNode*>& GetAllNodes() const { return _boneNodeByIdx; }

	void UpdateAnimation(unsigned int frameNo);

	void Dispose();

private:
	std::unordered_map<std::wstring, BoneNode*> _boneNodeByName;
	std::vector<BoneNode*> _boneNodeByIdx;
	std::vector<BoneNode*> _sortedNodes;

	unsigned int _duration = 0;
};