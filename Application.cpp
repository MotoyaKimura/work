#include "Application.h"
#include "Wrapper.h"
#include "SceneManager.h"
#include <Windows.h>
#include <tchar.h>

#ifdef _DEBUG
#include <iostream>
#endif

std::shared_ptr<SceneManager> Application::_sceneManager = nullptr;
std::shared_ptr<Wrapper> Application::_dx = nullptr;
HWND Application::hwnd = nullptr;
bool Application::fullscreenMode = false;
bool Application::isPause = false;
WNDCLASSEX Application::w;
LPRECT Application::wrc;
POINT Application::center;
UINT Application::window_width = GetSystemMetrics(SM_CXSCREEN);
UINT Application::window_height = GetSystemMetrics(SM_CYSCREEN);

LRESULT CALLBACK Application::WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	
	if(msg == WM_ACTIVATE)
	{
		if(wParam == WA_ACTIVE){}
		else if (wParam == WA_CLICKACTIVE){}
		else if (wParam == WA_INACTIVE)
		{
			while (ShowCursor(true) < 0);
			ClipCursor(NULL);
		}
		return 0;
	}

	if (msg == WM_KEYDOWN)
	{
		if ((wParam == VK_ESCAPE) & (lParam >> 30))
		{}
		else
		{
			isPause = !isPause;
			return 0;
		}
	}

	if(msg == WM_CREATE || msg == WM_EXITSIZEMOVE)
	{
		GetWindowRect(hwnd, wrc);
		center = { (wrc->left + wrc->right) / 2,
			(wrc->top + wrc->bottom) / 2 };
		return 0;
	}
	
	if(msg == WM_MOUSEMOVE)
	{
		while (ShowCursor(false) >= 0);
		return 0;
	}


	if (msg == WM_NCMOUSEMOVE)
	{
		while (ShowCursor(true) < 0);
		return 0;
	}

	if(msg == WM_SYSKEYDOWN)
	{
		if (wParam == VK_RETURN && 1 << 29)
		{
			_sceneManager->ResizeSceneManager();
			ToggleFullscreenWindow(_dx->GetSwapChain());
			return 0;
		}
	}

	

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Application::DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

Application::Application() 
{
}
void Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& w)
{
	if (hwnd != nullptr)return;
	
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = _T("work");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w);

	wrc = new RECT();
	wrc->left = 0;
	wrc->top = 0;
	wrc->right = static_cast<LONG>(window_width);
	wrc->bottom = static_cast<LONG>(window_height);

	AdjustWindowRect(wrc, windowStyle, false);

	hwnd = CreateWindow(w.lpszClassName,
		_T("work"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc->right - wrc->left,
		wrc->bottom - wrc->top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);
}

void Application::ToggleFullscreenWindow(Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain)
{
	if (fullscreenMode)
	{
		
		SetWindowLong(hwnd, GWL_STYLE, windowStyle);
		SetWindowPos(
			hwnd,
			HWND_NOTOPMOST,
			0,
			0,
			GetWindowSize().cx,
			GetWindowSize().cy,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_NORMAL);
	}
	else
	{
		GetWindowRect(hwnd, wrc);
		SetWindowLong(hwnd, GWL_STYLE,
			windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));
		RECT fullscreenWindowRect;
		Microsoft::WRL::ComPtr<IDXGIOutput> pOutput;
		swapChain->GetContainingOutput(&pOutput);
		DXGI_OUTPUT_DESC Desc;
		pOutput->GetDesc(&Desc);
		fullscreenWindowRect = Desc.DesktopCoordinates;
		
		SetWindowPos(
			hwnd,
			HWND_TOPMOST,
			fullscreenWindowRect.left,
			fullscreenWindowRect.top,
			fullscreenWindowRect.right,
			fullscreenWindowRect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_MAXIMIZE);
	}
	fullscreenMode = !fullscreenMode;
}

SIZE Application::GetWindowSize()
{
	SIZE winSize;
	winSize.cx = window_width;
	winSize.cy = window_height;
	return winSize;
}
Application& Application::Instance()
{
	static Application instance;
	return instance;
}
bool Application::Init()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);
	CreateGameWindow(hwnd, w);
	
	_dx.reset(new Wrapper(hwnd));
	if(!_dx->Init())
	{
		DebugOutputFormatString("DX12周りの初期化エラー\n ");
		return false;
	}

	_sceneManager.reset(new SceneManager());
	if(!_sceneManager->InitializeSceneManager())
	{
		DebugOutputFormatString("管理シーンの初期化エラー\n ");
		return false;
	}
}

void Application::Run()
{
	DebugOutputFormatString("Show window test.\n ");

	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}

		_sceneManager->UpdateSceneManager();
		_sceneManager->RenderSceneManager();

	}
}
void Application::Terminate()
{
	_sceneManager->FinalizeSceneManager();
	_sceneManager.reset();
	CoUninitialize();
	UnregisterClass(w.lpszClassName, w.hInstance);
}
Application::~Application()
{
}

