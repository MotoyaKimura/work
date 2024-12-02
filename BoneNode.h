#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include "PmxModel.h"


class BoneNode
{
public:
	BoneNode(unsigned int index, const PMXBone& pmxBone);

	unsigned int GetBoneIndex() const { return _boneIndex; }
	const std::wstring& GetName() const { return _name; }
	unsigned int GetParentBoneIndex() const { return _parentBoneIndex; }
	unsigned int GetDeformDepth() const { return _deformDepth; }
	unsigned int GetAppendBoneIndex() const { return _appendBoneIndex; }
	unsigned int GetIKTargetBoneIndex() const { return _ikTargetBoneIndex; }

	void SetParentBoneNode(BoneNode* parentNode)
	{
		_parentBoneNode = parentNode;
		_parentBoneNode->AddChildBoneNode(this);
	}
	const BoneNode* GetParentBoneNode() const { return _parentBoneNode; }

	void AddChildBoneNode(BoneNode* childNode) { _childrenNodes.push_back(childNode); }
	const std::vector<BoneNode*>& GetChildBoneNodes() const { return _childrenNodes; }

	const DirectX::XMMATRIX& GetInitInverseTransform() const { return _inverseInitTransform; }
	const DirectX::XMMATRIX& GetLocalTransform() const { return _localTransform; }
	const DirectX::XMMATRIX& GetGlobalTransform() const { return _globalTransform; }

	void SetAnimateRotation(const DirectX::XMMATRIX& rotation) { _animateRotation = rotation; }
	const DirectX::XMMATRIX& GetAnimateRotation() const { return _animateRotation; }
	const DirectX::XMFLOAT3& GetAnimatePosition() const { return _animatePosition; }

	void SetPosition(const DirectX::XMFLOAT3& position) { _position = position; }
	const DirectX::XMFLOAT3& GetPosition() const { return _position; }

	void SetIKRotation(const DirectX::XMMATRIX& rotation) { _ikRotation = rotation; }
	const DirectX::XMMATRIX& GetIKRotation() const { return _ikRotation; }

	void SetMorphPosition(const DirectX::XMFLOAT3& position) { _morphPosition = position; }
	void SetMorphRotation(const DirectX::XMMATRIX& rotation) { _morphRotation = rotation; }

	void AddMotionKey(unsigned int& frameNo, DirectX::XMFLOAT4& quaternion, DirectX::XMFLOAT3& offset, DirectX::XMFLOAT2& p1, DirectX::XMFLOAT2& p2);
	void SortAllKeys();

	void SetEnableAppendRotate(bool enable) { _isAppendRotate = enable; }
	void SetEnableAppendTranslate(bool enable) { _isAppendTranslate = enable; }
	void SetEnableAppendLocal(bool enable) { _isAppendLocal = enable; }
	void SetAppendWeight(float weight) { _appendWeight = weight; }
	float GetAppendWeight() const { return _appendWeight; }
	void SetAppendBoneNode(BoneNode* node) { _appendBoneNode = node; }
	BoneNode* GetAppendBoneNode() const { return _appendBoneNode; }
	const DirectX::XMMATRIX& GetAppendRotation() const { return _appendRotation; }
	const DirectX::XMFLOAT3& GetAppendTranslate() const { return _appendTranslate; }

	unsigned int GetMaxFrameNo() const;

	void UpdateLocalTransform();
	void UpdateGlobalTransform();

	void AnimateMotion(unsigned int frameNo);

private:
	float GetYFromXOnBezier(float x, DirectX::XMFLOAT2& a, DirectX::XMFLOAT2& b, uint8_t n);

	unsigned int _boneIndex;
	std::wstring _name;
	DirectX::XMFLOAT3 _position;
	unsigned int _parentBoneIndex = -1;
	unsigned int _deformDepth;
	PMXBoneFlags _boneFlag;
	unsigned int _appendBoneIndex;
	unsigned int _ikTargetBoneIndex;
	unsigned int _ikIterationCount;
	float _ikLimit;
	bool _enableIK = false;

	DirectX::XMFLOAT3 _animatePosition;
	DirectX::XMMATRIX _animateRotation;

	DirectX::XMFLOAT3 _morphPosition;
	DirectX::XMMATRIX _morphRotation;

	DirectX::XMMATRIX _ikRotation;

	DirectX::XMFLOAT3 _appendTranslate;
	DirectX::XMMATRIX _appendRotation;

	DirectX::XMMATRIX _inverseInitTransform;
	DirectX::XMMATRIX _localTransform;
	DirectX::XMMATRIX _globalTransform;

	BoneNode* _parentBoneNode = nullptr;
	std::vector <BoneNode*> _childrenNodes;

	bool _isAppendRotate = false;
	bool _isAppendTranslate = false;
	bool _isAppendLocal = false;
	float _appendWeight = 0.0f;
	BoneNode* _appendBoneNode = nullptr;

	struct VMDKey
	{
		unsigned int frameNo;
		DirectX::XMVECTOR quaternion;
		DirectX::XMFLOAT3 offset;
		DirectX::XMFLOAT2 p1;
		DirectX::XMFLOAT2 p2;

		VMDKey(unsigned int frameNo, const DirectX::XMVECTOR& quaternion, DirectX::XMFLOAT3& offset, DirectX::XMFLOAT2& p1, DirectX::XMFLOAT2& p2) :
			frameNo(frameNo),
			quaternion(quaternion),
			offset(offset),
			p1(p1),
			p2(p2)
		{}
	};

	std::vector<VMDKey> _motionKeys;

};