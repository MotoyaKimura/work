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

void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

Application::Application() : hwnd(nullptr)
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
		DebugOutputFormatString("DX12周りの初期化エラー\n ");
		return false;
	}

	_pera.reset(new Pera(_dx));
	if (!_pera->Init())
	{
		DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_keyboard.reset(new Keyboard(hwnd));

	_renderer.reset(new Renderer(_dx, _pera, _keyboard));
	if(!_renderer->Init())
	{
		DebugOutputFormatString("レンダラー周りの初期化エラー\n ");
		return false;
	}

	_model.reset(new Model(_dx));
	if(!_model->Load("modelData/bunny/bunny.obj")) return false;
	
	if (!_model->Init())
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model->Move(30, 0, 30);

	_renderer->AddModel(_model);

	_model2 = std::make_shared<Model>(_dx);
	if (!_model2->Load("modelData/RSMScene/floor/floor.obj")) return false;
	if (!_model2->Init())
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model2->Move(30, 0, 30);
	_renderer->AddModel(_model2);

	_model3 = std::make_shared<Model>(_dx);
	if (!_model3->Load("modelData/RSMScene/wall/wall_red.obj")) return false;
	if (!_model3->Init())
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model3->Move(30, 30, 0);
	_renderer->AddModel(_model3);

	_model4 = std::make_shared<Model>(_dx);
	if (!_model4->Load("modelData/RSMScene/wall/wall_green.obj")) return false;
	if (!_model4->Init())
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model4->Move(0, 30, 30);
	_renderer->AddModel(_model4);

	
	_sceneManager.reset(new SceneManager());
	_scene.reset(new Scene());
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
				_dx->ResizeBackBuffers();
				_renderer->ResizeBuffers();
			}
		}

	
		
		_renderer->Move();
		_renderer->Update();

		_dx->BeginDrawShade();
		_renderer->BeforeDrawShade();
		_renderer->DrawShade();
		_dx->EndDrawShade();

		_dx->BeginDrawTeapot();
		_renderer->BeforeDrawTeapot();
		_renderer->DrawTeapot();
		_dx->EndDrawTeapot();

		_dx->BeginDrawSSAO();
		_renderer->BeforeDrawSSAO();
		_renderer->DrawSSAO();
		_dx->EndDrawSSAO();

		_dx->BeginDrawPera();
		_renderer->BeforeDrawPera();
		_renderer->DrawPera();
		_dx->EndDrawPera();
		_dx->ExecuteCommand();
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

