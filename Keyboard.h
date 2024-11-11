#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <iostream>
#include <vector>
class Camera;
class Model;
class Keyboard
{
private:
	std::shared_ptr<Camera> _camera;
	std::vector<std::shared_ptr<Model>> _models;
	HWND _hwnd;
	BYTE keycode[256];

	POINT cursorPos;
	POINT center;
	bool isActiveFirst = true;
public:
	Keyboard(HWND hwnd, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Model>>models);
	void Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate, DirectX::XMFLOAT3* _eyePos, DirectX::XMFLOAT3* _targetPos);
	~Keyboard();
};