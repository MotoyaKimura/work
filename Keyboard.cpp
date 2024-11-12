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
	SetCursorPos(Application::GetCenter().x, Application::GetCenter().y);
	diff_x = cursorPos.x - Application::GetCenter().x;
	diff_y = cursorPos.y - Application::GetCenter().y;
	if (diff_x == 0 && diff_y == 0) 
		return false;
	return true;
}

void Keyboard::SetPos()
{
	_pos->x = XMVectorGetX(pos);
	_pos->y = XMVectorGetY(pos);
	_pos->z = XMVectorGetZ(pos);
	_targetPos->x = XMVectorGetX(pos);
	_targetPos->y = XMVectorGetY(pos) + 20;
	_targetPos->z = XMVectorGetZ(pos);
	_eyePos->x = XMVectorGetX(eyePos);
	_eyePos->y = XMVectorGetY(eyePos);
	_eyePos->z = XMVectorGetZ(eyePos);
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
		* XMMatrixRotationAxis(yAxis, -diff_x * 0.005)
		* XMMatrixRotationAxis(vRight, -diff_y * 0.005)
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
	if (keycode['W'] & 0x80) {
		pos = XMVectorAdd(pos, vFront);
		eyePos = XMVectorAdd(eyePos, vFront);
		isMove = true;
	}
	if (keycode['A'] & 0x80) {
		pos = XMVectorAdd(pos, vLeft);
		eyePos = XMVectorAdd(eyePos, vLeft);
		isMove = true;
	}
	if (keycode['S'] & 0x80) {
		pos = XMVectorAdd(pos, vBack);
		eyePos = XMVectorAdd(eyePos, vBack);
		isMove = true;
	}
	if (keycode['D'] & 0x80) {
		pos = XMVectorAdd(pos, vRight);
		eyePos = XMVectorAdd(eyePos, vRight);
		isMove = true;
	}
	if (keycode[VK_SPACE] & 0x80)
	{
		pos = XMVectorAdd(pos, vUp);
		eyePos = XMVectorAdd(eyePos, vUp);
		isMove = true;
	}
	if (keycode[VK_SHIFT] & 0x80)
	{
		pos = XMVectorAdd(pos, vDown);
		eyePos = XMVectorAdd(eyePos, vDown);
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
	return isMove;
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

