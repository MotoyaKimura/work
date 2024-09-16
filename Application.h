#pragma once
#include <Windows.h>

class Wrapper;
class Renderer;
class Application
{
private:
	Wrapper& _dx;
	Renderer& _renderer;

	const unsigned int window_width = 1280;							//�E�B���h�E��
	const unsigned int window_height = 720;							//�E�B���h�E��

	WNDCLASSEX w = {};
	HWND hwnd;

	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& w);
public:
	SIZE GetWindowSize() const;
	//Application�̃V���O���g���C���X�^���X�𓾂�
	static Application& Instance();
	//������
	bool Init();
	//���[�v�N��
	void Run();
	//�㏈��
	void Terminate();
	~Application();
};
