#pragma once
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif


class Wrapper;
class Pera;
class Model;
class Renderer;
class Application
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<Model> _model;
	std::shared_ptr<Model> _model2;

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
