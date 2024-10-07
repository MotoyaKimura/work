#pragma once
#include <Windows.h>
#include <DirectXMath.h>
class Keyboard
{
private:
	BYTE keycode[256];

public:
	Keyboard();
	void Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate);
	~Keyboard();
};