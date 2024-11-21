#pragma once
#include "Application.h"


class Button
{
private:
	HWND hBTN = nullptr;
	POINT cursorPos;
	LPRECT rect = new RECT();
	HMENU _id;
	bool isActive = false;
public:
	void Create(
		LPCWSTR name,
		int left, int top,
		int width, int height, 
		HMENU id
	);
	void Show();
	void Hide();
	void Enable();
	void Disable();
	void Destroy();
	void Update();
	bool IsHover();
	bool IsClicked();
	bool IsActive() { return isActive; }
	void SetInActive() { isActive = false; }
	HWND GetHwnd() { return hBTN; }
	HMENU GetID() { return _id; }
	Button();
	~Button();
};
