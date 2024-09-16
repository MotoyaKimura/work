#include "Application.h"
#include "Wrapper.h"
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

void CheckResult(HRESULT result)
{
	if (FAILED(result))
	{
		DebugOutputFormatString("エラーが発生しました\n");
		exit(1);
	}
}

struct SHeader
{
	std::uint8_t version;
	std::uint8_t isFlatShading;
	std::uint16_t numMeshParts;
};

struct SMeshePartsHeader
{
	std::uint32_t numMaterial;
	std::uint32_t numVertex;
	std::uint8_t indexSize;
	std::uint8_t pad[3];
};

struct SVertex
{
	float pos[3];
	float normal[3];
	float uv[2];
	float weights[4];
	std::int16_t indices[4];
};

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
	CreateGameWindow(hwnd, w);
	
	_dx.reset(new Wrapper(hwnd));
	if(!_dx->Init())
	{
		return false;
	}
	_renderer.reset(new Renderer(*_dx));
	if(!_renderer->Init())
	{
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

		_dx->BeginDraw();
		_renderer->BeforeDraw();
		_dx->Draw();
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

