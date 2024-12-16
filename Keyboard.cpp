#include "Keyboard.h"
#include "Model.h"
#include "Application.h"
#include "Camera.h"

//モデルの移動、カメラの移動、衝突判定を行うクラス
using namespace DirectX;

//キーボードクラス
Keyboard::Keyboard(HWND hwnd, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Model>>models) :
	_hwnd(hwnd), _camera(camera), _models(models)
{
}

Keyboard::~Keyboard()
{
}

//初期化
void Keyboard::Init()
{
	//座標
	_pos = _models[modelID]->GetPos();
	pos = XMLoadFloat3(_pos);
	_rotate = _models[modelID]->GetRotate();
	_eyePos = _camera->GetEyePos();
	eyePos = XMLoadFloat3(_eyePos);
	_targetPos = _camera->GetTargetPos();

	//カメラの向き
	eyeToTarget = XMVectorSubtract(XMLoadFloat3(_targetPos), XMLoadFloat3(_eyePos));
	vFront = XMVectorSetY(eyeToTarget, 0);
	vFront = XMVector3Normalize(vFront);
	vBack = XMVectorNegate(vFront);
	vUp = XMVectorSet(0, 1, 0, 0);
	vDown = XMVectorNegate(vUp);
	vRight = XMVector3Cross(vUp, vFront);
	vLeft = XMVectorNegate(vRight);
}

//外部から呼び出される更新関数
void Keyboard::Update()
{
	if (!isActive()) return;
	MoveModel();
	MoveCamera();
}

//ウィンドウがアクティブになったとき、カーソルを中心に戻す
bool Keyboard::isActive()
{
	if (_hwnd != GetForegroundWindow())
	{
		isActiveFirst = true;
		return false;
	}
	if (isActiveFirst)
	{
		SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);
		_startTime = timeGetTime() - elapsedTime;
		isActiveFirst = false;
	}
	return true;
}

//モデルの移動
void Keyboard::MoveModel()
{
	//メニュー、ポーズ、遊び方シーンの場合は時間を止める、落下防止
	if(isMenu || isPause || isHowToPlay)
	{
		_startTime = timeGetTime() - elapsedTime;
		isPause = false;
		isMenu = false;
		isHowToPlay = false;
	}
	CalcMove();
	SetPos();
}

//カメラの更新
void Keyboard::MoveCamera()
{
	if(isMoveMouse())
	{
		CalcMoveDir();
		RotateCameraAroundModel();
	}
	CollisionCamera();
}

//カメラのめり込み防止のための衝突判定
void Keyboard::CollisionCamera()
{
	//間隔
	float diff = 0.1f;
	//カメラ半径
	float radius = 10.0f;
	if (eyePos.m128_f32[1] > -100)
	{
		while (true)
		{
			XMFLOAT3 center;
			center = _models[modelID]->GetAABBCenter();
			//モデルの中心からカメラ向きに出たレイを少しずつ伸ばしていく
			XMVECTOR rayPt = XMLoadFloat3(&center) - XMVector3Normalize(eyeToTarget) * diff;
			//レイのAABB
			aabb rayAabb = {
				rayPt.m128_f32[0] - 0.01f,
				rayPt.m128_f32[0] + 0.01f,
				rayPt.m128_f32[1] - 0.01f,
				rayPt.m128_f32[1] + 0.01f,
				rayPt.m128_f32[2] - 0.01f,
				rayPt.m128_f32[2] + 0.01f,
			};
			//衝突したらレイの位置にカメラを移動
			if (isCollision(rayAabb))
			{
				XMStoreFloat3(_eyePos, rayPt);
				break;
			}

			diff += 0.01f;
			if (diff >= radius)
			{
				XMStoreFloat3(_eyePos, XMLoadFloat3(_targetPos) - XMVector3Normalize(eyeToTarget) * radius);
				break;
			}
		}
	}
}

//マウスの移動量を計算
bool Keyboard::isMoveMouse()
{
	GetCursorPos(&cursorPos);
	diff_x = cursorPos.x - Application::GetCenter().x;
	diff_y = cursorPos.y - Application::GetCenter().y;

	if (diff_x == 0 && diff_y == 0) 
		return false;

	SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);

	return true;
}

//モデルとカメラの位置更新
void Keyboard::SetPos()
{
	_pos->x = XMVectorGetX(pos);
	_pos->y = XMVectorGetY(pos);
	_pos->z = XMVectorGetZ(pos);
	_targetPos->x = XMVectorGetX(pos);
	_targetPos->y = XMVectorGetY(pos) + 2.5f;
	_targetPos->z = XMVectorGetZ(pos);
	_eyePos->x = XMVectorGetX(eyePos);
	_eyePos->y = XMVectorGetY(eyePos);
	_eyePos->z = XMVectorGetZ(eyePos);
}

//モデルが移動する方向に回転
void Keyboard::SetDir(XMVECTOR dir)
{
	FXMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
	float dot = XMVector3Dot(_models[modelID]->GetEye(), dir).m128_f32[0];
	dot = std::clamp(dot, -1.0f, 1.0f);
	if (dot < 0.99)
	{
		XMVECTOR cross = XMVector3Normalize(XMVector3Cross(_models[modelID]->GetEye(), dir));
		XMMATRIX rotation;
		if (cross.m128_f32[1] > 0)
		{
			_rotate->y += 0.1f;
			rotation = XMMatrixRotationAxis(yAxis, 0.1f);
		}
		else
		{
			_rotate->y -= 0.1f;
			rotation = XMMatrixRotationAxis(yAxis, -0.1f);
		}
		_models[modelID]->SetEye(XMVector3Transform(_models[modelID]->GetEye(), rotation));
	}
}

//カメラから見たモデルの移動方向を計算
void Keyboard::CalcMoveDir()
{
	eyeToTarget = XMVectorSubtract(XMLoadFloat3(_pos), XMLoadFloat3(_eyePos));
	vFront = XMVectorSetY(eyeToTarget, 0);
	vFront = XMVector3Normalize(vFront);
	vBack = XMVectorNegate(vFront);
	vUp = XMVectorSet(0, 1, 0, 0);
	vDown = XMVectorNegate(vUp);
	vRight = XMVector3Cross(vUp, vFront);
	vLeft = XMVectorNegate(vRight);
}


//カメラをモデルを中心に回転
void Keyboard::RotateCameraAroundModel()
{
	XMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
	XMMATRIX eyeMat =
		XMMatrixTranslation(-_pos->x, -_pos->y, -_pos->z)
		* XMMatrixRotationAxis(yAxis, diff_x * 0.005)
		* XMMatrixRotationAxis(vRight, diff_y * 0.005)
		* XMMatrixTranslation(_pos->x, _pos->y, _pos->z);

	XMVECTOR eyeVec = XMLoadFloat3(_eyePos);
	XMVECTOR eyeTrans = XMVector3Transform(eyeVec, eyeMat);

	if (isAngleLimit(eyeTrans))
	{
		eyeMat = XMMatrixTranslation(-_pos->x, -_pos->y, -_pos->z)
			* XMMatrixRotationAxis(yAxis, diff_x * 0.005)
			* XMMatrixTranslation(_pos->x, _pos->y, _pos->z);
		eyeTrans = XMVector3Transform(eyeVec, eyeMat);
	}
	eyePos = eyeTrans;
	_eyePos->x = XMVectorGetX(eyeTrans);
	_eyePos->y = XMVectorGetY(eyeTrans);
	_eyePos->z = XMVectorGetZ(eyeTrans);
}

//カメラの緯度に制限をかける
bool Keyboard::isAngleLimit(XMVECTOR eyePos)
{
	XMVECTOR eyeToTarget = XMVectorSubtract(XMLoadFloat3(_targetPos), eyePos);
	eyeToTarget = XMVector3Normalize(eyeToTarget);
	FXMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
	float dot = XMVector3Dot(eyeToTarget, yAxis).m128_f32[0];
	if(dot < 0.0f)
	{
		dot = XMVector3Dot(eyeToTarget, -yAxis).m128_f32[0];
	}
	if (dot > 0.65f)
	{
		return true;
	}
	return false;
}

//カメラの自動回転
void Keyboard::AutoRotateCamera()
{
	FXMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
	XMMATRIX eyeMat =
		XMMatrixTranslation(-_pos->x, -_pos->y, -_pos->z)
		* XMMatrixRotationAxis(yAxis,  0.005)
		* XMMatrixTranslation(_pos->x, _pos->y, _pos->z);
	FXMVECTOR eyeVec = XMLoadFloat3(_eyePos);
	FXMVECTOR eyeTrans = XMVector3Transform(eyeVec, eyeMat);
	_eyePos->x = XMVectorGetX(eyeTrans);
	_eyePos->y = XMVectorGetY(eyeTrans);
	_eyePos->z = XMVectorGetZ(eyeTrans);
}

//カメラの衝突判定
bool Keyboard::isCollision(aabb aabb)
{
	for (int i = 0; i < _models.size(); i++)
	{
		if (i == modelID) continue;
		if (aabb._xMax <= _models[i]->GetAABB()->_xMin) continue;
		if (aabb._xMin >= _models[i]->GetAABB()->_xMax) continue;
		if (aabb._yMax <= _models[i]->GetAABB()->_yMin) continue;
		if (aabb._yMin >= _models[i]->GetAABB()->_yMax) continue;
		if (aabb._zMax <= _models[i]->GetAABB()->_zMin) continue;
		if (aabb._zMin >= _models[i]->GetAABB()->_zMax) continue;
		return true;
	}
	return false;
}


//重力計算、ジャンプ、移動
void Keyboard::CalcMove()
{
	CalcGravity();
	
	//移動
		//斜め右
	if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('D') & 0x8000)) {
		Collision(vRight + vFront);
		return;
	}
		//斜め左
	if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('A') & 0x8000))
	{
		Collision(vLeft + vFront);
		return;
	}

	if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('S') & 0x8000))
	{
		return;
	}
	if ((GetAsyncKeyState('A') & 0x8000) && (GetAsyncKeyState('D') & 0x8000))
	{
		return;
	}
		//斜め左後ろ
	if ((GetAsyncKeyState('A') & 0x8000) && (GetAsyncKeyState('S') & 0x8000))
	{
		Collision(vLeft + vBack);
		return;
	}
		//斜め右後ろ
	if ((GetAsyncKeyState('D') & 0x8000) && (GetAsyncKeyState('S') & 0x8000))
	{
		Collision(vRight + vBack);
		return;
	}
		//前
	if (GetAsyncKeyState('W') & 0x8000) {
		Collision(vFront);
		return;
	}
		//左
	if (GetAsyncKeyState('A') & 0x8000) {
		Collision(vLeft);
		return;
	}
		//後ろ
	if (GetAsyncKeyState('S') & 0x8000) {
		Collision(vBack);
		return;
	}
		//右
	if (GetAsyncKeyState('D') & 0x8000) {
		Collision(vRight);
		return;
	}
}

//y軸の衝突判定
bool Keyboard::CollisionY()
{
	for (int i = 0; i < _models.size(); i++)
	{
		if (i == modelID) continue;
		if (_models[modelID]->GetAABB()->_xMax <= _models[i]->GetAABB()->_xMin) continue;
		if (_models[modelID]->GetAABB()->_xMin >= _models[i]->GetAABB()->_xMax) continue;
		if (_models[modelID]->GetAABB()->_zMax <= _models[i]->GetAABB()->_zMin) continue;
		if (_models[modelID]->GetAABB()->_zMin >= _models[i]->GetAABB()->_zMax) continue;
		if (_models[modelID]->GetAABB()->_yMax <= _models[i]->GetAABB()->_yMin) continue;
		if (_models[modelID]->GetAABB()->_yMin >= _models[i]->GetAABB()->_yMax) continue;
		float centerY1 = (_models[modelID]->GetAABB()->_yMax + _models[modelID]->GetAABB()->_yMin) / 2;
		float edgeY1 = abs(_models[modelID]->GetAABB()->_yMax - _models[modelID]->GetAABB()->_yMin);
		float centerY2 = (_models[i]->GetAABB()->_yMax + _models[i]->GetAABB()->_yMin) / 2;
		float edgeY2 = abs(_models[i]->GetAABB()->_yMax - _models[i]->GetAABB()->_yMin);
		if (abs(centerY1 - centerY2) - (edgeY1 + edgeY2) / 2.0f < 0.0f)
		{
			_startTime = timeGetTime();
			return true;
		}
	}
	return false;
}

//XZ軸の衝突判定
void Keyboard::Collision(DirectX::XMVECTOR dir)
{
	float velocity = 0.2;
	XMVECTOR v = XMVector3Normalize(dir);
	SetDir(v);
	//aabbを移動
	pos = XMVectorAdd(pos, v * velocity);
	eyePos = XMVectorAdd(eyePos, v * velocity);
	_models[modelID]->GetAABB()->_xMax += (v * velocity).m128_f32[0];
	_models[modelID]->GetAABB()->_xMin += (v * velocity).m128_f32[0];
	_models[modelID]->GetAABB()->_zMax += (v * velocity).m128_f32[2];
	_models[modelID]->GetAABB()->_zMin += (v * velocity).m128_f32[2];

	//衝突判定
	for (int i = 0; i < _models.size(); i++)
	{
		if (i == modelID) continue;
		if (_models[modelID]->GetAABB()->_xMax <= _models[i]->GetAABB()->_xMin) continue;
		if (_models[modelID]->GetAABB()->_xMin >= _models[i]->GetAABB()->_xMax) continue;
		if (_models[modelID]->GetAABB()->_zMax <= _models[i]->GetAABB()->_zMin) continue;
		if (_models[modelID]->GetAABB()->_zMin >= _models[i]->GetAABB()->_zMax) continue;
		if (_models[modelID]->GetAABB()->_yMax <= _models[i]->GetAABB()->_yMin) continue;
		if (_models[modelID]->GetAABB()->_yMin >= _models[i]->GetAABB()->_yMax) continue;
		float centerX1 = (_models[modelID]->GetAABB()->_xMax + _models[modelID]->GetAABB()->_xMin) / 2;
		float centerZ1 = (_models[modelID]->GetAABB()->_zMax + _models[modelID]->GetAABB()->_zMin) / 2;
		float edgeX1 = abs(_models[modelID]->GetAABB()->_xMax - _models[modelID]->GetAABB()->_xMin);
		float edgeZ1 = abs(_models[modelID]->GetAABB()->_zMax - _models[modelID]->GetAABB()->_zMin);
		float centerX2 = (_models[i]->GetAABB()->_xMax + _models[i]->GetAABB()->_xMin) / 2;
		float centerZ2 = (_models[i]->GetAABB()->_zMax + _models[i]->GetAABB()->_zMin) / 2;
		float edgeX2 = abs(_models[i]->GetAABB()->_xMax - _models[i]->GetAABB()->_xMin);
		float edgeZ2 = abs(_models[i]->GetAABB()->_zMax - _models[i]->GetAABB()->_zMin);

		//小さいほうを選択し、元に戻す
		if (abs(abs(centerX1 - centerX2) - (edgeX1 + edgeX2) / 2) < abs(abs(centerZ1 - centerZ2) - (edgeZ1 + edgeZ2) / 2))
		{
			XMVECTOR vec = XMVECTOR({ v.m128_f32[0], 0, 0 });
			pos = XMVectorSubtract(pos, vec * velocity);
			eyePos = XMVectorSubtract(eyePos, vec * velocity);
			_models[modelID]->GetAABB()->_xMax -= (vec * velocity).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin -= (vec * velocity).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax -= (vec * velocity).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin -= (vec * velocity).m128_f32[2];
		}
		else
		{
			XMVECTOR vec = XMVECTOR({ 0, 0, v.m128_f32[2] });
			pos = XMVectorSubtract(pos, vec * velocity);
			eyePos = XMVectorSubtract(eyePos, vec * velocity);
			_models[modelID]->GetAABB()->_xMax -= (vec * velocity).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin -= (vec * velocity).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax -= (vec * velocity).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin -= (vec * velocity).m128_f32[2];
		}
	}
}

//重力計算
void Keyboard::CalcGravity()
{
	if (_startTime <= 0)
	{
		_startTime = timeGetTime();
	}

	elapsedTime = timeGetTime() - _startTime;
	unsigned int t = 30 * (elapsedTime / 1000.0f);
	float second = elapsedTime / 1000.0f;

	velocity = velocity + gravity * second;
	float x = velocity * second + gravity * second * second * 0.5;
	pos = XMVectorAdd(pos, vDown * x);
	//カメラはy = -100以下には行かない
	if (eyePos.m128_f32[1] > -100)
	{
		eyePos = XMVectorAdd(eyePos, vDown * x);
	}
	_models[modelID]->GetAABB()->_yMax += (vDown * x).m128_f32[1];
	_models[modelID]->GetAABB()->_yMin += (vDown * x).m128_f32[1];

	//ジャンプ
	if (GetAsyncKeyState(VK_SPACE) & 0x8000 && CollisionY())
	{
		velocity = -2.5;
		pos = XMVectorAdd(pos, vUp * 0.01);
		eyePos = XMVectorAdd(eyePos, vUp * 0.01);
		_models[modelID]->GetAABB()->_yMax += (vUp * 0.01).m128_f32[1];
		_models[modelID]->GetAABB()->_yMin += (vUp * 0.01).m128_f32[1];
	}

	//地面についたら速度を0にする
	if (CollisionY()) {
		velocity = 0;
		pos = XMVectorSubtract(pos, vDown * x);
		eyePos = XMVectorSubtract(eyePos, vDown * x);
		_models[modelID]->GetAABB()->_yMax -= (vDown * x).m128_f32[1];
		_models[modelID]->GetAABB()->_yMin -= (vDown * x).m128_f32[1];
	}
}


