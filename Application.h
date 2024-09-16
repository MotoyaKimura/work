#pragma once
#include <Windows.h>

class Wrapper;
class Renderer;
class Application
{
private:
	Wrapper& _dx;
	Renderer& _renderer;

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
