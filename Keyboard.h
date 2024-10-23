#pragma once
#include <Windows.h>
#include <DirectXMath.h>
class Keyboard
{
private:
	BYTE keycode[256];

	POINT cursorPos;
	int x = 0;
	int y = 0;
	int latitude = 0;
public:
	Keyboard();
	void Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate, DirectX::XMFLOAT3* _eyePos, DirectX::XMFLOAT3* _targetPos);
	~Keyboard();
};