#include "Application.h"
#include "Wrapper.h"
#include "Model.h"
#include "Renderer.h"
#include <Windows.h>
#include <tchar.h>

#ifdef _DEBUG
#include <iostream>
#endif

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void DebugOutputFormatString(const char* format, ...)
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

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = _T("study");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w);

	RECT wrc = { 0, 0, window_width, window_height };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(w.lpszClassName,
		_T("studyテスト"),
		WS_OVERLAPPEDWINDOW,
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
	//auto result = CoInitializeEx(0, COINIT_MULTITHREADED);
	CreateGameWindow(hwnd, w);
	
	_dx.reset(new Wrapper(hwnd));
	if(!_dx->Init())
	{
		DebugOutputFormatString("DX12周りの初期化エラー\n ");
		return false;
	}

	_model.reset(new Model(_dx));
	if (!_model->Init("modelData/teapot.tkm"))
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}

	_renderer.reset(new Renderer(_dx));
	if(!_renderer->Init())
	{
		DebugOutputFormatString("レンダラー周りの初期化エラー\n ");
		return false;
	}

	_renderer->AddModel(_model);
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
		_model->Update();
		_dx->Update();

		_renderer->BeforeDraw();
		_dx->BeginDraw();
		_dx->Draw();
		_renderer->Draw();
		_dx->EndDraw();
		_dx->Flip();
	}
}
void Application::Terminate()
{
	UnregisterClass(w.lpszClassName, w.hInstance);
}
Application::~Application()
{
}

