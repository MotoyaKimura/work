#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <iostream>
class Keyboard
{
private:
	HWND _hwnd;
	BYTE keycode[256];

	POINT cursorPos;
	POINT center;
	bool isActiveFirst = true;
public:
	Keyboard(HWND hwnd);
	void Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate, DirectX::XMFLOAT3* _eyePos, DirectX::XMFLOAT3* _targetPos);
	void SetCursorCenter();
	~Keyboard();
};