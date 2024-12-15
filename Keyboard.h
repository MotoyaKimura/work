#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <iostream>
#include "Model.h"
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
	bool isMenu = false;
	bool isHowToPlay = false;
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

	bool isPause = false;
	DWORD elapsedTime = 0;
	DWORD _startTime = 0;
	float velocity = 0;
	float gravity = 7.0f/10;
public:
	void Init();
	Keyboard(HWND hwnd, std::shared_ptr<Camera> camera, std::vector<std::shared_ptr<Model>>models);
	void Update();
	void MoveModel();
	void MoveCamera();
	

	void CalcMove();
	bool isMoveMouse();
	void CalcMoveDir();
	void SetPos();
	void SetDir(DirectX::XMVECTOR dir);
	void RotateCameraAroundModel();
	void AutoRotateCamera();
	void Collision(DirectX::XMVECTOR dir);
	bool CollisionY();
	void CalcGravity();
	void CollisionCamera();
	bool isCollision(aabb aabb);
	void SetIsMenu(bool _isMenu) { isMenu = _isMenu; }
	void SetIsHowToPlay(bool _isHowToPlay) { isHowToPlay = _isHowToPlay; }
	void SetIsPause(bool _isPause) { isPause =  _isPause; }
	~Keyboard();
};