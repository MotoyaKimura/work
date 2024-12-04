#include "IKSolver.h"
#include <limits>

using namespace DirectX;

enum class SolveAxis
{
	X,
	Y,
	Z
};

IKSolver::IKSolver(BoneNode* node, BoneNode* targetNode, unsigned int iterationCount, float limitAngle) :
	_ikNode(node),
	_targetNode(targetNode),
	_ikIterationCount(iterationCount),
	_ikLimitAngle(limitAngle),
	_enable(true)
{
}

void IKSolver::Solve()
{
	if (_enable == false)
	{
		return;
	}

	if (_ikNode == nullptr || _targetNode == nullptr)
	{
		return;
	}

	for(IKChain& chain : _ikChains)
	{
		chain.prevAngle = XMFLOAT3(0.0f, 0.0f, 0.0f);
		chain.boneNode->SetIKRotation(XMMatrixIdentity());
		chain.planeModeAngle = 0.0f;

		chain.boneNode->UpdateLocalTransform();
		chain.boneNode->UpdateGlobalTransform();
	}

	float maxDistance = (std::numeric_limits<float>::max)();
	for(unsigned int i = 0; i < _ikIterationCount; i++)
	{
		SolveCore(i);

		XMVECTOR targetPosition = _targetNode->GetGlobalTransform().r[3];
		XMVECTOR ikPosition = _ikNode->GetGlobalTransform().r[3];
		float dist = XMVector3Length(targetPosition - ikPosition).m128_f32[0];

		if(dist < maxDistance)
		{
			maxDistance = dist;
			for (IKChain& chain : _ikChains)
			{
				XMStoreFloat4(&chain.saveIKRotation, XMQuaternionRotationMatrix(chain.boneNode->GetIKRotation()));
			}
		}
		else
		{
			for (IKChain& chain : _ikChains)
			{
				chain.boneNode->SetIKRotation(XMMatrixRotationQuaternion(XMLoadFloat4(&chain.saveIKRotation)));
				chain.boneNode->UpdateLocalTransform();
				chain.boneNode->UpdateGlobalTransform();
			}
			break;
		}
	}
}

void IKSolver::SolveCore(unsigned int iteration)
{
	XMVECTOR ikPosition = _ikNode->GetGlobalTransform().r[3];
	for(unsigned int chainIndex = 0; chainIndex < _ikChains.size(); chainIndex++)
	{
		IKChain& chain = _ikChains[chainIndex];
		BoneNode* chainNode = chain.boneNode;
		if (chainNode == nullptr)
		{
			continue;
		}

		if(chain.enableAxisLimit == true)
		{
			if((chain.limitMin.x != 0 || chain.limitMax.x != 0) &&
				(chain.limitMin.y == 0 || chain.limitMax.y == 0)&&
				(chain.limitMin.z == 0 || chain.limitMax.z == 0))
			{
				SolvePlane(iteration, chainIndex, SolveAxis::X);
				continue;
			}
			else if ((chain.limitMin.x == 0 || chain.limitMax.x == 0) &&
				(chain.limitMin.y != 0 || chain.limitMax.y != 0) &&
				(chain.limitMin.z == 0 || chain.limitMax.z == 0))
			{
				SolvePlane(iteration, chainIndex, SolveAxis::Y);
				continue;
			}
			else if ((chain.limitMin.x == 0 || chain.limitMax.x == 0) &&
				(chain.limitMin.y == 0 || chain.limitMax.y == 0) &&
				(chain.limitMin.z != 0 || chain.limitMax.z != 0))
			{
				SolvePlane(iteration, chainIndex, SolveAxis::Z);
				continue;
			}
		}

		XMVECTOR targetPosition = _targetNode->GetGlobalTransform().r[3];

		XMVECTOR det = XMMatrixDeterminant(chain.boneNode->GetGlobalTransform());
		XMMATRIX inverseChain = XMMatrixInverse(&det, chain.boneNode->GetGlobalTransform());

		XMVECTOR chainIKPosition = XMVector3Transform(ikPosition, inverseChain);
		XMVECTOR chainTargetPosition = XMVector3Transform(targetPosition, inverseChain);

		XMVECTOR chainIKVector = XMVector3Normalize(chainIKPosition);
		XMVECTOR chainTargetVector = XMVector3Normalize(chainTargetPosition);

		float dot = XMVector3Dot(chainTargetVector, chainIKVector).m128_f32[0];
		dot = std::clamp(dot, -1.0f, 1.0f);

		float angle = acosf(dot);
		float angleDegree = XMConvertToDegrees(angle);
		if (angleDegree < 1.0e-3f)
		{
			continue;
		}

		angle = std::clamp(angle, -_ikLimitAngle, _ikLimitAngle);
		XMVECTOR cross = XMVector3Normalize(XMVector3Cross(chainTargetVector, chainIKVector));
		XMMATRIX rotation = XMMatrixRotationAxis(cross, angle);

		XMMATRIX chainRotation = rotation * chainNode->GetAnimateRotation() * chainNode->GetIKRotation();
		if(chain.enableAxisLimit == true)
		{
			XMFLOAT3 rotXYZ = Decompose(chainRotation, chain.prevAngle);

			XMFLOAT3 clampXYZ = std::clamp(rotXYZ, chain.limitMin, chain.limitMax);
			float invLimitAngle = -_ikLimitAngle;
			XMVECTOR vecSub = XMVectorSubtract(XMLoadFloat3(&clampXYZ), XMLoadFloat3(&chain.prevAngle));
			XMFLOAT3 fSub;
			XMStoreFloat3(&fSub, vecSub);
			clampXYZ = std::clamp(fSub, XMFLOAT3(invLimitAngle, invLimitAngle, invLimitAngle), 
				XMFLOAT3(_ikLimitAngle, _ikLimitAngle, _ikLimitAngle));
			XMVECTOR vecAdd = XMVectorAdd(XMLoadFloat3(&clampXYZ), XMLoadFloat3(&chain.prevAngle));
			XMFLOAT3 fAdd;
			XMStoreFloat3(&fAdd, vecAdd);
			clampXYZ = fAdd;
			
			chainRotation = XMMatrixRotationRollPitchYaw(clampXYZ.x, clampXYZ.y, clampXYZ.z);
			chain.prevAngle = clampXYZ;
		}

		XMVECTOR det1 = XMMatrixDeterminant(chain.boneNode->GetAnimateRotation());
		XMMATRIX inverseAnimate = XMMatrixInverse(&det1, chain.boneNode->GetAnimateRotation());

		XMMATRIX ikRotation = inverseAnimate * chainRotation;
		chain.boneNode->SetIKRotation(ikRotation);

		chain.boneNode->UpdateLocalTransform();
		chain.boneNode->UpdateGlobalTransform();
	}
}
