#include "IKSolver.h"
#include <limits>

//IKの計算を行うクラス
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

//外部から呼び出されるIK計算関数
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

	//全てのチェーンボーンを初期化
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
		//目標ボーン
		XMVECTOR targetPosition = _targetNode->GetGlobalTransform().r[3];
		//末端ボーン
		XMVECTOR ikPosition = _ikNode->GetGlobalTransform().r[3];
		float dist = XMVector3Length(targetPosition - ikPosition).m128_f32[0];

		//目標と末端ボーンの距離縮まったら計算続行、回転行列を保存
		if(dist < maxDistance)
		{
			maxDistance = dist;
			for (IKChain& chain : _ikChains)
			{
				XMStoreFloat4(&chain.saveIKRotation, XMQuaternionRotationMatrix(chain.boneNode->GetIKRotation()));
			}
		}
		//目標との距離が縮まらなかったら保存した回転行列を全てのボーンに適用
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

//全てのチェーンボーンのIK回転を計算する
void IKSolver::SolveCore(unsigned int iteration)
{
	//末端ボーン
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
			//特定の軸に制限がある場合はその軸に対して計算
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
		//目標ボーン
		XMVECTOR targetPosition = _targetNode->GetGlobalTransform().r[3];

		XMVECTOR det = XMMatrixDeterminant(chain.boneNode->GetGlobalTransform());
		XMMATRIX inverseChain = XMMatrixInverse(&det, chain.boneNode->GetGlobalTransform());
		//末端ボーンの座標をチェーンボーンの座標系に変換
		XMVECTOR chainIKPosition = XMVector3Transform(ikPosition, inverseChain);
		//目標ボーンの座標をチェーンボーンの座標系に変換
		XMVECTOR chainTargetPosition = XMVector3Transform(targetPosition, inverseChain);

		//原点が同じベクトルを作る
		XMVECTOR chainIKVector = XMVector3Normalize(chainIKPosition);
		XMVECTOR chainTargetVector = XMVector3Normalize(chainTargetPosition);

		float dot = XMVector3Dot(chainTargetVector, chainIKVector).m128_f32[0];
		dot = std::clamp(dot, -1.0f, 1.0f);

		//２つのベクトルの角度がほぼ０の時なら計算しない
		float angle = acos(dot);
		float angleDegree = XMConvertToDegrees(angle);
		if (chainTargetVector.m128_f32[0] - chainIKVector.m128_f32[0] < 1.0e-3f &&
			chainTargetVector.m128_f32[1] - chainIKVector.m128_f32[1] < 1.0e-3f &&
			chainTargetVector.m128_f32[2] - chainIKVector.m128_f32[2] < 1.0e-3f)
		{
			continue;
		}
		if (angleDegree < 1.0e-3f)
		{
			continue;
		}

		//そうでなければ回転行列を計算
		angle = std::clamp(angle, -_ikLimitAngle, _ikLimitAngle);
		XMVECTOR cross = XMVector3Normalize(XMVector3Cross(chainTargetVector, chainIKVector));
		XMMATRIX rotation = XMMatrixRotationAxis(cross, angle);

		XMMATRIX chainRotation = rotation * chainNode->GetAnimateRotation() * chainNode->GetIKRotation();

		if(chain.enableAxisLimit == true)
		{
			XMFLOAT3 rotXYZ = Decompose(chainRotation, chain.prevAngle);

			//角度制限
			XMFLOAT3 clampXYZ
			= XMFLOAT3(std::clamp(rotXYZ.x, chain.limitMin.x, chain.limitMax.x),
				std::clamp(rotXYZ.y, chain.limitMin.y, chain.limitMax.y),
				std::clamp(rotXYZ.z, chain.limitMin.z, chain.limitMax.z));
			float invLimitAngle = -_ikLimitAngle;
			XMVECTOR vecSub = XMVectorSubtract(XMLoadFloat3(&clampXYZ), XMLoadFloat3(&chain.prevAngle));
			XMFLOAT3 fSub;
			XMStoreFloat3(&fSub, vecSub);
			clampXYZ
			= XMFLOAT3(std::clamp(fSub.x, invLimitAngle, _ikLimitAngle),
				std::clamp(fSub.y, invLimitAngle, _ikLimitAngle),
				std::clamp(fSub.z, invLimitAngle, _ikLimitAngle));
				 
			XMVECTOR vecAdd = XMVectorAdd(XMLoadFloat3(&clampXYZ), XMLoadFloat3(&chain.prevAngle));
			XMFLOAT3 fAdd;
			XMStoreFloat3(&fAdd, vecAdd);
			clampXYZ = fAdd;

			//回転行列を保存
			chainRotation = XMMatrixRotationRollPitchYaw(clampXYZ.x, clampXYZ.y, clampXYZ.z);
			chain.prevAngle = clampXYZ;
		}

		XMVECTOR det1 = XMMatrixDeterminant(chain.boneNode->GetAnimateRotation());
		XMMATRIX inverseAnimate = XMMatrixInverse(&det1, chain.boneNode->GetAnimateRotation());

		//UpdateLocalTransformでアニメーション回転の計算を行うため、ここではその分を引いておく
		XMMATRIX ikRotation = inverseAnimate * chainRotation;
		chain.boneNode->SetIKRotation(ikRotation);

		chain.boneNode->UpdateLocalTransform();
		chain.boneNode->UpdateGlobalTransform();
	}
}

//回転行列の回転値のみを抽出
DirectX::XMFLOAT3 IKSolver::Decompose(const DirectX::XMMATRIX& m, const DirectX::XMFLOAT3& before)
{
	XMFLOAT3 r;
	float sy = -m.r[0].m128_f32[2];
	const float e = 1.0e-6;

	//ジンバルロックのチェック
	//ジンバルロックが発生した場合はsinxとsinzが計算できない
	if((1.0f - std::abs(sy)) < e)
	{
		r.y = std::asin(sy);
		//以前の回転値から計算
		float sx = std::sin(before.x);
		float sz = std::sin(before.z);
		//打消しを防ぐため、回転値が大きいほうを採用
		if(std::abs(sx) < std::abs(sz))
		{
			float cx = std::cos(before.x);
			if(cx > 0)
			{
				r.x = 0;
				r.z = std::asin(-m.r[1].m128_f32[0]);
			}
			else
			{
				r.x = XM_PI;
				r.z = std::asin(m.r[1].m128_f32[0]);
			}
		}
		else
		{
			float cz = std::cos(before.z);
			if (cz > 0)
			{
				r.z = 0;
				r.x = std::asin(-m.r[2].m128_f32[1]);
			}
			else
			{
				r.z = XM_PI;
				r.x = std::asin(m.r[2].m128_f32[1]);
			}
		}
	}
	//ジンバルロックが発生していない場合
	else
	{
		r.x = std::atan2(m.r[1].m128_f32[2], m.r[2].m128_f32[2]);
		r.y = std::asin(-m.r[0].m128_f32[2]);
		r.z = std::atan2(m.r[0].m128_f32[1], m.r[0].m128_f32[0]);
	}

	const float pi = XM_PI;
	XMFLOAT3 tests[] =
	{
		{r.x + pi, pi - r.y, r.z + pi},
		{r.x + pi, pi - r.y, r.z - pi},
		{r.x + pi, -pi - r.y, r.z + pi},
		{r.x + pi, -pi - r.y, r.z - pi},
		{r.x - pi, pi - r.y, r.z + pi},
		{r.x - pi, pi - r.y, r.z - pi},
		{r.x - pi, -pi - r.y, r.z + pi},
		{r.x - pi, -pi - r.y, r.z - pi},
	};

	float errX = std::abs(DiffAngle(r.x, before.x));
	float errY = std::abs(DiffAngle(r.y, before.y));
	float errZ = std::abs(DiffAngle(r.z, before.z));
	float minErr = errX + errY + errZ;
	//最も誤差が小さいものを選択
	for(const auto test : tests)
	{
		float err = std::abs(DiffAngle(test.x, before.x))
		+ std::abs(DiffAngle(test.y, before.y))
		+ std::abs(DiffAngle(test.z, before.z));
		if (err < minErr)
		{
			minErr = err;
			r = test;
		}
	}
	return r;
}

//角度を正規化
float IKSolver::NormalizeAngle(float angle)
{
	float ret = angle;
	while (ret >= XM_2PI)
	{
		ret -= XM_2PI;
	}
	while (ret < 0)
	{
		ret += XM_2PI;
	}
	return ret;
}

//角度の差を計算
float IKSolver::DiffAngle(float a, float b)
{
	float diff = NormalizeAngle(a) - NormalizeAngle(b);
	if (diff > XM_PI)
	{
		return diff - XM_2PI;
	}
	else if (diff < -XM_PI)
	{
		return diff + XM_2PI;
	}
	return diff;
}


//特定の軸に対して回転行列を計算
void IKSolver::SolvePlane(unsigned int iteration, unsigned int chainIndex, SolveAxis solveAxis)
{
	XMFLOAT3 rotateAxis;
	float limitMinAngle;
	float limitMaxAngle;

	IKChain& chain = _ikChains[chainIndex];

	switch (solveAxis)
	{
	case SolveAxis::X:
		limitMinAngle = chain.limitMin.x;
		limitMaxAngle = chain.limitMax.x;
		rotateAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
		break;
	case SolveAxis::Y:
		limitMinAngle = chain.limitMin.y;
		limitMaxAngle = chain.limitMax.y;
		rotateAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
		break;
	case SolveAxis::Z:
		limitMinAngle = chain.limitMin.z;
		limitMaxAngle = chain.limitMax.z;
		rotateAxis = XMFLOAT3(0.0f, 0.0f, 1.0f);
		break;
	}
	XMVECTOR ikPosition = _ikNode->GetGlobalTransform().r[3];
	XMVECTOR targetPosition = _targetNode->GetGlobalTransform().r[3];

	XMVECTOR det = XMMatrixDeterminant(chain.boneNode->GetGlobalTransform());
	XMMATRIX inverseChain = XMMatrixInverse(&det, chain.boneNode->GetGlobalTransform());

	XMVECTOR chainIKPosition = XMVector3Transform(ikPosition, inverseChain);
	XMVECTOR chainTargetPosition = XMVector3Transform(targetPosition, inverseChain);

	XMVECTOR chainIKVector = XMVector3Normalize(chainIKPosition);
	XMVECTOR chainTargetVector = XMVector3Normalize(chainTargetPosition);

	float dot = XMVector3Dot(chainTargetVector, chainIKVector).m128_f32[0];
	dot = std::clamp(dot, -1.0f, 1.0f);

	float angle = acos(dot);
	angle = std::clamp(angle, -_ikLimitAngle, _ikLimitAngle);

	XMVECTOR rotation1 = XMQuaternionRotationAxis(XMLoadFloat3(&rotateAxis), angle);
	XMVECTOR targetVector1 = XMVector3Rotate(chainTargetVector, rotation1);
	float dot1 = XMVector3Dot(targetVector1, chainIKVector).m128_f32[0];

	XMVECTOR rotation2 = XMQuaternionRotationAxis(XMLoadFloat3(&rotateAxis), -angle);
	XMVECTOR targetVector2 = XMVector3Rotate(chainTargetVector, rotation2);
	float dot2 = XMVector3Dot(targetVector2, chainIKVector).m128_f32[0];

	float newAngle = chain.planeModeAngle;
	//目標ベクトルと回転後のベクトルの内積を比較して回転方向を決定
	//内積が大きい方が回転角度が小さい、つまり目標ベクトルに近い
	if (dot1 > dot2)
	{
		newAngle += angle;
	}
	else
	{
		newAngle -= angle;
	}

	//最初だけ最大回転角度の処理を行う
	//制限角度を超える可能性がある場合、符号を反転させて範囲内に収める
	//それでもだめなら中間値を求めて符号反転の可否を判定する
	if(iteration == 0)
	{
		if (newAngle < limitMinAngle || newAngle > limitMaxAngle)
		{
			if (-newAngle > limitMinAngle && -newAngle < limitMaxAngle)
			{
				newAngle *= -1;
			}
			else
			{
				float halfRadian = (limitMinAngle + limitMaxAngle) * 0.5f;
				if(abs(halfRadian - newAngle) > abs(halfRadian + newAngle))
				{
					newAngle *= -1;
				}
			}
		}
	}

	newAngle = std::clamp(newAngle, limitMinAngle, limitMaxAngle);
	chain.planeModeAngle = newAngle;

	XMVECTOR det1 = XMMatrixDeterminant(chain.boneNode->GetAnimateRotation());
	XMMATRIX inverseAnimate = XMMatrixInverse(&det1, chain.boneNode->GetAnimateRotation());

	XMMATRIX ikRotation = inverseAnimate * XMMatrixRotationAxis(XMLoadFloat3(&rotateAxis), newAngle);
	//回転行列を保存
	chain.boneNode->SetIKRotation(ikRotation);
	//回転行列を適用
	chain.boneNode->UpdateLocalTransform();
	chain.boneNode->UpdateGlobalTransform();
}
