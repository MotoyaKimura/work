#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Renderer.h"
#include "Keyboard.h"
#include "SceneManager.h"
#include "Scene.h"
#include <Windows.h>
#include <tchar.h>

#ifdef _DEBUG
#include <iostream>
#endif

std::shared_ptr<Wrapper> Application::_dx = nullptr;
HWND Application::hwnd = nullptr;

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	
	if(msg == WM_ACTIVATE)
	{
		if (wParam == WA_ACTIVE){}
		else if (wParam == WA_CLICKACTIVE){}
		else if (wParam == WA_INACTIVE)
		{
			while (ShowCursor(true) < 0);
			ClipCursor(NULL);
		}
	}
	

	if(msg == WM_EXITSIZEMOVE)
	{
		LPRECT rect = new RECT();
		GetWindowRect(hwnd, rect);
		SetCursorPos((rect->left + rect->right) / 2, 
			(rect->top + rect->bottom) / 2);
	}
	
	if(msg == WM_MOUSEMOVE)
	{
		while (ShowCursor(false) >= 0);
	}

	if (msg == WM_NCMOUSEMOVE)
	{
		while (ShowCursor(true) < 0);
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

	
	window_width = GetSystemMetrics(SM_CXSCREEN);
	window_height = GetSystemMetrics(SM_CYSCREEN);

	RECT wrc = { 0, 0, window_width, window_height };

	AdjustWindowRect(&wrc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);

	hwnd = CreateWindow(w.lpszClassName,
		_T("work"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	
}

SIZE Application::GetWindowSize() const
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
		DebugOutputFormatString("DX12Žü‚è‚Ì‰Šú‰»ƒGƒ‰[\n ");
		return false;
	}

	
	
	_sceneManager.reset(new SceneManager());
	_scene.reset(new Scene(_dx, hwnd));
	_sceneManager->InitializeSceneManager();
	_sceneManager->JumpScene(_scene->SetupTestScene);
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

		if (msg.message == WM_SYSKEYUP ||
			msg.message == WM_SYSKEYDOWN)
		{
			if (msg.wParam == VK_RETURN)
			{
				
				_sceneManager->UpdateSceneManager();
			}
		}

	
		
		_sceneManager->RenderSceneManager();

		
		
	}
}
void Application::Terminate()
{
	UnregisterClass(w.lpszClassName, w.hInstance);
}
Application::~Application()
{
}

