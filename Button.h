#pragma once
#include "Application.h"


class Button
{
private:
	HWND hBTN = nullptr;
public:
	void Create(LPCWSTR name, int left, int top, int width, int height, HMENU id);
	void Show();
	void Hide();
	void Enable();
	void Disable();
	void Destroy();
	Button();
	~Button();
};
