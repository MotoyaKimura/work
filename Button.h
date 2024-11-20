#pragma once
#include "Application.h"


class Button
{
private:
	HWND hBTN = nullptr;
	POINT cursorPos;
	LPRECT rect = new RECT();
	HMENU _id;
public:
	void Create(LPCWSTR name, int left, int top, int width, int height, HMENU id);
	void Show();
	void Hide();
	void Enable();
	void Disable();
	void Destroy();
	bool IsHover();
	bool IsClicked();
	HWND GetHwnd() { return hBTN; }
	Button();
	~Button();
};
