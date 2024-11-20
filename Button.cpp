#include "Button.h"
#include <tchar.h>


void Button::Create(LPCWSTR name, int left, int top, int width, int height, HMENU id)
{
	hBTN = CreateWindowEx(0, _T("BUTTON"), name,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		left, top, width, height,
		Application::GetHwnd(),
		id, Application::GetW().hInstance, nullptr
	);
}

void Button::Show()
{
	ShowWindow(hBTN, SW_SHOW);
}

void Button::Hide()
{
	ShowWindow(hBTN, SW_HIDE);
}

void Button::Enable()
{
	EnableWindow(hBTN, true);
}

void Button::Disable()
{
	EnableWindow(hBTN, false);
}

void Button::Destroy()
{
	DestroyWindow(hBTN);
	hBTN = nullptr;
}

Button::Button()
{
}

Button::~Button()
{
}
