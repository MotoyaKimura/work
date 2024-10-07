#pragma once
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif


class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;
class Application
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Pera> _pera;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<Model> _model;
	std::shared_ptr<Model> _model2;
	std::shared_ptr<Model> _model3;
	std::shared_ptr<Model> _model4;

	std::shared_ptr<Keyboard> _keyboard;

	const unsigned int window_width = 1280;							//ウィンドウ幅
	const unsigned int window_height = 720;							//ウィンドウ高

	WNDCLASSEX w = {};
	HWND hwnd;

	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& w);
public:
	SIZE GetWindowSize() const;
	//Applicationのシングルトンインスタンスを得る
	static Application& Instance();
	//初期化
	bool Init();
	//ループ起動
	void Run();
	//後処理
	void Terminate();
	~Application();
};
