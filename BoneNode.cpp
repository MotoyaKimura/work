#include "BoneNode.h"

using namespace DirectX;


BoneNode::BoneNode(unsigned int index, const PMXBone& pmxBone) :
_boneIndex(index),
_name(pmxBone.name),
_position(pmxBone.position),
_parentBoneIndex(pmxBone.parentBoneIndex),
_deformDepth(pmxBone.deformDepth),
_boneFlag(pmxBone.boneFlag),
_appendBoneIndex(pmxBone.appendBoneIndex),
_ikTargetBoneIndex(pmxBone.ikTargetBoneIndex),
_ikIterationCount(pmxBone.ikIterationCount),
_ikLimit(pmxBone.ikLimit),
_animatePosition(XMFLOAT3(0.0f, 0.0f, 0.0f)),
_animateRotation(XMMatrixIdentity()),
_inverseInitTransform(XMMatrixTranslation(-pmxBone.position.x, -pmxBone.position.y, -pmxBone.position.z)),
_localTransform(XMMatrixIdentity()),
_globalTransform(XMMatrixIdentity()),
_appendTranslate(XMFLOAT3(0.0f, 0.0f, 0.0f)),
_appendRotation(XMMatrixIdentity())
{
}

void BoneNode::AddMotionKey(unsigned int& frameNo, XMFLOAT4& quaternion, XMFLOAT3& offset, const XMFLOAT2& p1, const XMFLOAT2& p2)
{
	_motionKeys.emplace_back(frameNo, XMLoadFloat4(&quaternion), offset, p1, p2);
}

void BoneNode::AddIKKey(unsigned int& frameNo, bool& enable)
{
	_ikKeys.emplace_back(frameNo, enable);
}


void BoneNode::SortAllKeys()
{
	std::sort(_motionKeys.begin(), _motionKeys.end(), 
		[](const VMDKey& left, const VMDKey& right)
		{
			return left.frameNo < right.frameNo;
		});
}

void BoneNode::AnimateMotion(unsigned int frameNo)
{
	_animateRotation = XMMatrixIdentity();
	_animatePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	if(_motionKeys.size() <= 0)
	{
		return;
	}

	auto rit = std::find_if(_motionKeys.rbegin(), _motionKeys.rend(),
		[frameNo](const VMDKey& key)
		{
			return key.frameNo <= frameNo;
		});

	if (rit == _motionKeys.rend())
	{
		return;
	}
	XMVECTOR animatePosition = XMLoadFloat3(&rit->offset);

	auto iterator = rit.base();

	if(iterator != _motionKeys.end())
	{
		float t = static_cast<float>(frameNo - rit->frameNo) / static_cast<float>(iterator->frameNo - rit->frameNo);

		t = GetYFromXOnBezier(t, iterator->p1, iterator->p2, 12);

		_animateRotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(rit->quaternion, iterator->quaternion, t));
		XMStoreFloat3(&_animatePosition, XMVectorLerp(animatePosition, XMLoadFloat3(&iterator->offset), t));
	}
	else
	{
		_animateRotation = XMMatrixRotationQuaternion(rit->quaternion);
	}
}

float BoneNode::GetYFromXOnBezier(float x, XMFLOAT2& a, XMFLOAT2& b, uint8_t n)
{
	if (a.x == a.y && b.x == b.y)
	{
		return x;
	}

	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x;
	const float k1 = 3 * b.x - 6 * a.x;
	const float k2 = 3 * a.x;

	constexpr float epsilon = 0.0005f;

	for(int i = 0; i < n; ++i)
	{
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;


		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}

		t -= ft / 2;
	}

	auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}

void BoneNode::UpdateLocalTransform()
{
	XMMATRIX scale = XMMatrixIdentity();

	XMMATRIX rotation = _animateRotation;

	XMVECTOR t = XMLoadFloat3(&_animatePosition) + XMLoadFloat3(&_position);

	XMMATRIX translate = XMMatrixTranslationFromVector(t);

	_localTransform = scale * rotation * translate;
}

void BoneNode::UpdateGlobalTransform()
{
	if(_parentBoneNode == nullptr)
	{
		_globalTransform = _localTransform;
	}
	else
	{
		_globalTransform = _localTransform * _parentBoneNode->GetGlobalTransform();
	}

	for (BoneNode* child : _childrenNodes)
	{
		child->UpdateGlobalTransform();
	}
}

unsigned int BoneNode::GetMaxFrameNo() const 
{
	if (_motionKeys.size() <= 0)
	{
		return 0;
	}

	return _motionKeys.back().frameNo;
}