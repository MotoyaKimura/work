#include "Keyboard.h"
#include "Model.h"
#include "Application.h"
#include "Camera.h"


using namespace DirectX;

void Keyboard::Init()
{
	_pos = _models[modelID]->GetPos();
	_rotate = _models[modelID]->GetRotate();
	_eyePos = _camera->GetEyePos();
	_targetPos = _camera->GetTargetPos();

	eyeToTarget = XMVectorSubtract(XMLoadFloat3(_pos), XMLoadFloat3(_eyePos));
	vFront = XMVectorSetY(eyeToTarget, 0);
	vFront = XMVector3Normalize(vFront);
	vBack = XMVectorNegate(vFront);
	vUp = XMVectorSet(0, 1, 0, 0);
	vDown = XMVectorNegate(vUp);
	vRight = XMVector3Cross(vUp, vFront);
	vLeft = XMVectorNegate(vRight);

	pos = XMLoadFloat3(_pos);
	eyePos = XMLoadFloat3(_eyePos);

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
		isActiveFirst = false;
	}
	return true;
}

void Keyboard::Move()
{
	if (!isActive()) return;
	MoveModel();
	MoveCamera();
}

void Keyboard::MoveModel()
{
	int modelNum = _models.size();
	int key = 0x60;
	GetKeyboardState(keycode);
	for (int i = 0; i < modelNum; i++)
	{
		if (keycode[key + i] & 0x80) {
			modelID = (key + i) & 0x0f;
			ChangeTarget();
		}
	}
	if (isGetKeyState())
		SetPos();
}

void Keyboard::MoveCamera()
{
	if(isMoveMouse())
	{
		CalcMoveDir();
		RotateCameraAroundModel();
	}
}

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



void Keyboard::RotateCameraAroundModel()
{
	FXMVECTOR yAxis = XMVectorSet(0, 1, 0, 0);
	XMMATRIX eyeMat =
		XMMatrixTranslation(-_pos->x, -_pos->y, -_pos->z)
		* XMMatrixRotationAxis(yAxis, diff_x * 0.005)
		* XMMatrixRotationAxis(vRight, diff_y * 0.005)
		* XMMatrixTranslation(_pos->x, _pos->y, _pos->z);

	FXMVECTOR eyeVec = XMLoadFloat3(_eyePos);
	FXMVECTOR eyeTrans = XMVector3Transform(eyeVec, eyeMat);
	
	eyePos = eyeTrans;
	_eyePos->x = XMVectorGetX(eyeTrans);
	_eyePos->y = XMVectorGetY(eyeTrans);
	_eyePos->z = XMVectorGetZ(eyeTrans);
}



bool Keyboard::isGetKeyState()
{
	bool isMove = false;
	GetKeyboardState(keycode);
	if ((keycode['W'] & 0x80) && (keycode['D'] & 0x80)) {
		keyCount++;
		if(keyCount > 3)
		{
			XMVECTOR v = XMVector3Normalize(vRight + vFront);
			SetDir(v);
			pos = XMVectorAdd(pos, v * 0.1);
			eyePos = XMVectorAdd(eyePos, v * 0.1);
			isMove = true;
			keyCount = 0;
			
			_models[modelID]->GetAABB()->_xMax += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (v * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (v * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				v = XMVECTOR({ v.m128_f32[0], 0, 0});
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				v = XMVECTOR({ 0, 0, v.m128_f32[2] });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if ((keycode['W'] & 0x80) && (keycode['A'] & 0x80))
	{
		keyCount++;
		if(keyCount > 3)
		{
			XMVECTOR v = XMVector3Normalize(vLeft + vFront);
			SetDir(v);
			pos = XMVectorAdd(pos, v * 0.1);
			eyePos = XMVectorAdd(eyePos, v * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (v * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (v * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				v = XMVECTOR({ v.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				v = XMVECTOR({ 0, 0, v.m128_f32[2] });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if ((keycode['W'] & 0x80) && (keycode['S'] & 0x80))
	{
		isMove = true;
		return isMove;
	}
	if ((keycode['A'] & 0x80) && (keycode['S'] & 0x80))
	{
		keyCount++;
		if (keyCount > 3)
		{
			XMVECTOR v = XMVector3Normalize(vLeft + vBack);
			SetDir(v);
			pos = XMVectorAdd(pos, v * 0.1);
			eyePos = XMVectorAdd(eyePos, v * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (v * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (v * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				v = XMVECTOR({ v.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				v = XMVECTOR({ 0, 0, v.m128_f32[2] });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if ((keycode['D'] & 0x80) && (keycode['S'] & 0x80))
	{
		keyCount++;
		if (keyCount > 3)
		{
			XMVECTOR v = XMVector3Normalize(vRight + vBack);
			SetDir(v);
			pos = XMVectorAdd(pos, v * 0.1);
			eyePos = XMVectorAdd(eyePos, v * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (v * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (v * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (v * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				v = XMVECTOR({ v.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				v = XMVECTOR({ 0, 0, v.m128_f32[2] });
				pos = XMVectorSubtract(pos, v * 0.1);
				eyePos = XMVectorSubtract(eyePos, v * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (v * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (v * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (v * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if (keycode['W'] & 0x80) {
		keyCount++;
		if (keyCount > 3)
		{
			SetDir(vFront);
			pos = XMVectorAdd(pos, vFront * 0.1);
			eyePos = XMVectorAdd(eyePos, vFront * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (vFront * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (vFront * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (vFront * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (vFront * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				vFront = XMVECTOR({ vFront.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, vFront * 0.1);
				eyePos = XMVectorSubtract(eyePos, vFront * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vFront * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vFront * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vFront * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vFront * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				vFront = XMVECTOR({ 0, 0, vFront.m128_f32[2] });
				pos = XMVectorSubtract(pos, vFront * 0.1);
				eyePos = XMVectorSubtract(eyePos, vFront * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vFront * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vFront * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vFront * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vFront * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if (keycode['A'] & 0x80) {
		keyCount++;
		if (keyCount > 3)
		{
			SetDir(vLeft);
			pos = XMVectorAdd(pos, vLeft * 0.1);
			eyePos = XMVectorAdd(eyePos, vLeft * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (vLeft * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (vLeft * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (vLeft * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (vLeft * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				vLeft = XMVECTOR({ vLeft.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, vLeft * 0.1);
				eyePos = XMVectorSubtract(eyePos, vLeft * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vLeft * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vLeft * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vLeft * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vLeft * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				vLeft = XMVECTOR({ 0, 0, vLeft.m128_f32[2] });
				pos = XMVectorSubtract(pos, vLeft * 0.1);
				eyePos = XMVectorSubtract(eyePos, vLeft * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vLeft * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vLeft * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vLeft * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vLeft * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if (keycode['S'] & 0x80) {
		keyCount++;
		if (keyCount > 3)
		{
			SetDir(vBack);
			pos = XMVectorAdd(pos, vBack * 0.1);
			eyePos = XMVectorAdd(eyePos, vBack * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (vBack * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (vBack * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (vBack * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (vBack * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				vBack = XMVECTOR({ vBack.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, vBack * 0.1);
				eyePos = XMVectorSubtract(eyePos, vBack * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vBack * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vBack * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vBack * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vBack * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				vBack = XMVECTOR({ 0, 0, vBack.m128_f32[2] });
				pos = XMVectorSubtract(pos, vBack * 0.1);
				eyePos = XMVectorSubtract(eyePos, vBack * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vBack * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vBack * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vBack * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vBack * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if (keycode['D'] & 0x80) {
		keyCount++;
		if (keyCount > 3)
		{
			SetDir(vRight);
			pos = XMVectorAdd(pos, vRight * 0.1);
			eyePos = XMVectorAdd(eyePos, vRight * 0.1);
			isMove = true;
			keyCount = 0;
			_models[modelID]->GetAABB()->_xMax += (vRight * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_xMin += (vRight * 0.1).m128_f32[0];
			_models[modelID]->GetAABB()->_zMax += (vRight * 0.1).m128_f32[2];
			_models[modelID]->GetAABB()->_zMin += (vRight * 0.1).m128_f32[2];
			if (isCollision() == "X")
			{
				vRight = XMVECTOR({ vRight.m128_f32[0], 0, 0 });
				pos = XMVectorSubtract(pos, vRight * 0.1);
				eyePos = XMVectorSubtract(eyePos, vRight * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vRight * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vRight * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vRight * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vRight * 0.1).m128_f32[2];
			}
			else if (isCollision() == "Z")
			{
				vRight = XMVECTOR({ 0, 0, vRight.m128_f32[2] });
				pos = XMVectorSubtract(pos, vRight * 0.1);
				eyePos = XMVectorSubtract(eyePos, vRight * 0.1);
				_models[modelID]->GetAABB()->_xMax -= (vRight * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_xMin -= (vRight * 0.1).m128_f32[0];
				_models[modelID]->GetAABB()->_zMax -= (vRight * 0.1).m128_f32[2];
				_models[modelID]->GetAABB()->_zMin -= (vRight * 0.1).m128_f32[2];
			}
		}
		return isMove;
	}
	if (keycode[VK_SPACE] & 0x80)
	{
		pos = XMVectorAdd(pos, vUp * 0.1);
		eyePos = XMVectorAdd(eyePos, vUp * 0.1);
		isMove = true;
	}
	if (keycode[VK_SHIFT] & 0x80)
	{
		pos = XMVectorAdd(pos, vDown * 0.1);
		eyePos = XMVectorAdd(eyePos, vDown * 0.1);
		isMove = true;
	}
	if (keycode[VK_SHIFT] & keycode['W'] & 0x80)
	{
	}
	if (keycode[VK_SHIFT] & keycode['S'] & 0x80)
	{
	}
	if (keycode['R'] & 0x80)
	{
		_rotate->y += 0.1f;
		isMove = true;
	}
	if (keycode['L'] & 0x80)
	{
		_rotate->y -= 0.1f;
		isMove = true;
	}

	keyCount = 0;
	return isMove;
}

std::string Keyboard::isCollision()
{
	for (int i = 0; i < _models.size(); i++)
	{
		std::cout << "models[" << i << "] " << _models[i]->GetAABB()->_xMax << " " << _models[i]->GetAABB()->_xMin << " " << _models[i]->GetAABB()->_zMax << " " << _models[i]->GetAABB()->_zMin << std::endl;
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

		if (abs(abs(centerX1 - centerX2) - (edgeX1 + edgeX2) / 2) < abs(abs(centerZ1 - centerZ2) - (edgeZ1 + edgeZ2) / 2))
		return "X";
		else 
			return "Z";
	}
	return "";
}




void Keyboard::ChangeTarget()
{
	_pos = _models[modelID]->GetPos();
	_rotate = _models[modelID]->GetRotate();
	_eyePos = _camera->GetEyePos();
	_targetPos = _camera->GetTargetPos();
	pos = XMLoadFloat3(_pos);
	eyePos = XMLoadFloat3(_eyePos);
}

Keyboard::Keyboard(HWND hwnd, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Model>>models) :
	_hwnd(hwnd), _camera(camera), _models(models)
{
}


Keyboard::~Keyboard()
{
}

