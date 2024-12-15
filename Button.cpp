#include "Button.h"
#include <tchar.h>

//�{�^���N���X
Button::Button(std::string name) : _name(name)
{
}

Button::~Button()
{
}

//�{�^���̍쐬
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

//�{�^���̕\��
void Button::Show()
{
	ShowWindow(hBTN, SW_SHOW);
}

//�{�^���̔�\��
void Button::Hide()
{
	ShowWindow(hBTN, SW_HIDE);
}

//�{�^���̗L����
void Button::Enable()
{
	EnableWindow(hBTN, true);
}

//�{�^���̖�����
void Button::Disable()
{
	EnableWindow(hBTN, false);
}

//�{�^���̔j��
void Button::Destroy()
{
	DestroyWindow(hBTN);
	hBTN = nullptr;
}

//�{�^���̃z�o�[����
bool Button::IsHover()
{
	GetCursorPos(&cursorPos);
	GetWindowRect(hBTN, rect);
	if (PtInRect(rect, cursorPos))return true;
	return false;
}

//�{�^���̃N���b�N����
bool Button::IsClicked()
{
	if (Application::GetButtonID() == _id) {
		Application::SetButtonID(nullptr);
		return true;
	}
	return false;
}

//�{�^���̍X�V
void Button::Update()
{
	if(IsClicked())
	{
		isActive = !isActive;
	}
}

