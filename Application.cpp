#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Renderer.h"
#include "Keyboard.h"
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

Application::Application() : hwnd(nullptr)
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

	_pera.reset(new Pera(_dx));
	if (!_pera->Init())
	{
		DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_keyboard.reset(new Keyboard());

	_renderer.reset(new Renderer(_dx, _pera, _keyboard));
	if(!_renderer->Init())
	{
		DebugOutputFormatString("レンダラー周りの初期化エラー\n ");
		return false;
	}

	//_model.reset(new Model(_dx));
	//if (!_model->LoadModel("modelData/teapot.tkm")) return false;
	//
	//if (!_model->Init())
	//{
	//	DebugOutputFormatString("モデルの初期化エラー\n ");
	//	return false;
	//}
	//_model->Move(-10, 10, -90);
	//_model->Rotate(-DirectX::XM_PIDIV2, 0, 0);
	//_renderer->AddModel(_model);

	_model2.reset(new Model(_dx));
	//if (!_model2->Load("modelData/bunny/bunny.obj")) return false;
	if (!_model2->Load("modelData/teapot/teapot.obj")) return false;
	//if (!_model2->Load("modelData/erato/erato.obj")) return false;
	if (!_model2->Init())
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_renderer->AddModel(_model2);

	/*_model2 = std::make_shared<Model>(_dx);
	if (!_model2->LoadModel("modelData/teapot.tkm")) return false;
	if (!_model2->Init())
	{
		DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model2->Move(100, -50, 0);
	_model2->Rotate(-DirectX::XM_PIDIV2, 0, 0);
	_renderer->AddModel(_model2);*/

	
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

