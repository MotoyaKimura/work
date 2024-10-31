#pragma once
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif


class Wrapper;
class Pera;
class Model;
class Renderer;
class Keyboard;
class Scene;
class SceneManager;
class Application
{
private:
	
	static std::shared_ptr<SceneManager> _sceneManager;
	std::shared_ptr<Scene> _scene;

	static UINT window_width;							//ウィンドウ幅
	static UINT window_height;							//ウィンドウ高
	static const UINT windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	WNDCLASSEX w = {};

	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& w);
	static void SetCursorCenterFirst();
	static void ToggleFullscreenWindow(Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain);
	static bool fullscreenMode;
	static bool isMinimized;
	static bool isActiveFirst;

protected:
	static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	static std::shared_ptr<Wrapper> _dx;
	static HWND hwnd;
	static LPRECT wrc;
	static POINT center;
	static SIZE GetWindowSize();
	//Applicationのシングルトンインスタンスを得る
	static Application& Instance();
	static void DebugOutputFormatString(const char* format, ...);
	//初期化
	bool Init();
	//ループ起動
	void Run();
	//後処理
	void Terminate();
	~Application();
};
