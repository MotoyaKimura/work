#include "Button.h"
#include <tchar.h>

//ボタンクラス
Button::Button(std::string name) : _name(name)
{
}

Button::~Button()
{
}

//ボタンの作成
void Button::Create(LPCWSTR name, int left, int top, int width, int height, HMENU id)
{
	hBTN = CreateWindowEx(0, _T("BUTTON"), name,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		left, top, width, height,
		Application::GetHwnd(),
		id, Application::GetW().hInstance, nullptr
	);
	_id = id;
}

//ボタンの表示
void Button::Show()
{
	ShowWindow(hBTN, SW_SHOW);
}

//ボタンの非表示
void Button::Hide()
{
	ShowWindow(hBTN, SW_HIDE);
}

//ボタンの有効化
void Button::Enable()
{
	EnableWindow(hBTN, true);
}

//ボタンの無効化
void Button::Disable()
{
	EnableWindow(hBTN, false);
}

//ボタンの破棄
void Button::Destroy()
{
	DestroyWindow(hBTN);
	hBTN = nullptr;
}

//ボタンのホバー判定
bool Button::IsHover()
{
	GetCursorPos(&cursorPos);
	GetWindowRect(hBTN, rect);
	if (PtInRect(rect, cursorPos))return true;
	return false;
}

//ボタンのクリック判定
bool Button::IsClicked()
{
	if (Application::GetButtonID() == _id) {
		Application::SetButtonID(nullptr);
		return true;
	}
	return false;
}

//ボタンの更新
void Button::Update()
{
	if(IsClicked())
	{
		isActive = !isActive;
	}
}

