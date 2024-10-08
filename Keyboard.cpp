#include "Keyboard.h"

void Keyboard::Move(DirectX::XMFLOAT3* _pos, DirectX::XMFLOAT3* _rotate)
{
	GetKeyboardState(keycode);
	if (keycode['W'] & 0x80) {
		_pos->z += 0.1f;
	}
	if (keycode['A'] & 0x80) {
		_pos->x -= 0.1f;
	}
	if (keycode['S'] & 0x80) {
		_pos->z -= 0.1f;
	}
	if (keycode['D'] & 0x80) {
		_pos->x += 0.1f;
	}
	if (keycode[VK_SPACE] & 0x80)
	{
		_pos->y += 0.1f;
	}
	if (keycode[VK_SHIFT] & 0x80)
	{
		_pos->y -= 0.1f;
	}
	if (keycode[VK_SHIFT] & keycode['W'] & 0x80)
	{
		_pos->z += 0.5f;
	}
	if (keycode[VK_SHIFT] & keycode['S'] & 0x80)
	{
		_pos->z -= 0.5f;
	}
	if (keycode['R'] & 0x80)
	{
		_rotate->y += 0.1f;
	}
	if (keycode['L'] & 0x80)
	{
		_rotate->y -= 0.1f;
	}
}

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
}

