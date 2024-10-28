#include "Keyboard.h"


void Keyboard::Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate, DirectX::XMFLOAT3* _eyePos, DirectX::XMFLOAT3* _targetPos)
{
	DirectX::XMVECTOR eyeToTarget =DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(_pos), DirectX::XMLoadFloat3(_eyePos));
	DirectX::XMVECTOR vFront = DirectX::XMVectorSetY(eyeToTarget, 0);
	vFront = DirectX::XMVector3Normalize(vFront);
	DirectX::XMVECTOR vBack = DirectX::XMVectorNegate(vFront);
	DirectX::XMVECTOR vUp = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMVECTOR vDown = DirectX::XMVectorNegate(vUp);
	DirectX::XMVECTOR vRight = DirectX::XMVector3Cross(vUp, vFront);
	DirectX::XMVECTOR vLeft = DirectX::XMVectorNegate(vRight);

	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(_pos);
	DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(_eyePos);


	GetKeyboardState(keycode);
	if (keycode['W'] & 0x80) {
		pos = DirectX::XMVectorAdd(pos, vFront);
		eyePos = DirectX::XMVectorAdd(eyePos, vFront);
	}
	if (keycode['A'] & 0x80) {
		pos = DirectX::XMVectorAdd(pos, vLeft);
		eyePos = DirectX::XMVectorAdd(eyePos, vLeft);
	
	}
	if (keycode['S'] & 0x80) {
		pos = DirectX::XMVectorAdd(pos, vBack);
		eyePos = DirectX::XMVectorAdd(eyePos, vBack);

	}
	if (keycode['D'] & 0x80) {
		pos = DirectX::XMVectorAdd(pos, vRight);
		eyePos = DirectX::XMVectorAdd(eyePos, vRight);
	
	}
	if (keycode[VK_SPACE] & 0x80)
	{
		pos = DirectX::XMVectorAdd(pos, vUp);
		eyePos = DirectX::XMVectorAdd(eyePos, vUp);
	
	}
	if (keycode[VK_SHIFT] & 0x80)
	{
		pos = DirectX::XMVectorAdd(pos, vDown);
		eyePos = DirectX::XMVectorAdd(eyePos, vDown);
	
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
	}
	if (keycode['L'] & 0x80)
	{
		_rotate->y -= 0.1f;
	}
	_pos->x = DirectX::XMVectorGetX(pos);
	_pos->y = DirectX::XMVectorGetY(pos);
	_pos->z = DirectX::XMVectorGetZ(pos);
	_targetPos->x = DirectX::XMVectorGetX(pos);
	_targetPos->y = DirectX::XMVectorGetY(pos) + 20;
	_targetPos->z = DirectX::XMVectorGetZ(pos);
	_eyePos->x = DirectX::XMVectorGetX(eyePos);
	_eyePos->y = DirectX::XMVectorGetY(eyePos);
	_eyePos->z = DirectX::XMVectorGetZ(eyePos);


	LPRECT rect = new RECT();
	GetWindowRect(_hwnd, rect);
	//std::cout << rect->left << " " << rect->right << " " << rect->top << " " << rect->bottom << std::endl;
	//GetClientRect(_hwnd, rect);
	//ClipCursor(rect);
	//std::cout << rect->left << " " << rect->right << " " << rect->top << " " << rect->bottom << std::endl;
	GetCursorPos(&cursorPos);

	SetCursorPos((rect->left + rect->right) / 2, (rect->top + rect->bottom) / 2);
	x = (rect->left + rect->right) / 2;
	y = (rect->top + rect->bottom) / 2;

	

	float diff_x = cursorPos.x - x;
	float diff_y = cursorPos.y - y;



	DirectX::FXMVECTOR yAxis = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMMATRIX eyeMat =
		DirectX::XMMatrixTranslation(-_pos->x, -_pos->y, -_pos->z)
		* DirectX::XMMatrixRotationAxis(yAxis, -diff_x * 0.005)
		* DirectX::XMMatrixRotationAxis(vRight, -diff_y * 0.005)
		* DirectX::XMMatrixTranslation(_pos->x, _pos->y, _pos->z);
	
	DirectX::FXMVECTOR eyeVec = XMLoadFloat3(_eyePos);
	DirectX::FXMVECTOR eyeTrans = DirectX::XMVector3Transform(eyeVec, eyeMat);
	_eyePos->x = DirectX::XMVectorGetX(eyeTrans);
	_eyePos->y = DirectX::XMVectorGetY(eyeTrans);
	_eyePos->z = DirectX::XMVectorGetZ(eyeTrans);
}


Keyboard::Keyboard(HWND hwnd) : _hwnd(hwnd)
{
}

void Keyboard::SetCursor()
{
	LPRECT rect = new RECT();
	GetWindowRect(_hwnd, rect);
	SetCursorPos((rect->left + rect->right) / 2, (rect->top + rect->bottom) / 2);
	GetCursorPos(&cursorPos);
}

Keyboard::~Keyboard()
{
}

