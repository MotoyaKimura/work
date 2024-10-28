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
	int x = 0;
	int y = 0;
	int frameCount = 0;
	
public:
	Keyboard(HWND hwnd);
	void Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate, DirectX::XMFLOAT3* _eyePos, DirectX::XMFLOAT3* _targetPos);
	void SetCursor();
	~Keyboard();
};