#pragma once
#include <DirectXMath.h>
#include <Windows.h>
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
	int modelID = 0;
	POINT cursorPos;
	POINT center;
	bool isActiveFirst = true;
	bool isActive();
	float diff_x;
	float diff_y;

	DirectX::XMFLOAT3* _pos;
	DirectX::XMFLOAT3* _rotate;
	DirectX::XMFLOAT3* _eyePos;
	DirectX::XMFLOAT3* _targetPos;
														  
	DirectX::XMVECTOR eyeToTarget;
	DirectX::XMVECTOR vFront;
	DirectX::XMVECTOR vBack;
	DirectX::XMVECTOR vUp;
	DirectX::XMVECTOR vDown;
	DirectX::XMVECTOR vRight;
	DirectX::XMVECTOR vLeft;
												
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR eyePos;

	unsigned int keyCount = 0;
	bool isMove = false;
	DWORD _startTime = 0;
	float velocity = 0;
	float gravity = 9.8f / 1000000.0f;
public:
	void Init();
	Keyboard(HWND hwnd, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Model>>models);
	void Move();
	void MoveModel();
	void MoveCamera();
	void ChangeTarget();
	bool isGetKeyState();
	bool isMoveMouse();
	void CalcMoveDir();
	void SetPos();
	void SetDir(DirectX::XMVECTOR dir);
	void RotateCameraAroundModel();
	void Collision(DirectX::XMVECTOR dir);
	bool CollisionY();
	void isCollision(DirectX::XMVECTOR dir);
	~Keyboard();
};