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

		auto backBufferIndex = _swapchain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER barrierDesc = {};
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDesc.Transition.pResource = backBuffers[backBufferIndex];
		barrierDesc.Transition.Subresource = 0;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		_cmdList->ResourceBarrier(1, &barrierDesc);



		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += backBufferIndex *
			_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(
			1,
			&rtvH,
			true,
			nullptr);
		float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		_cmdList->SetPipelineState(_pipelinestate);
		_cmdList->SetGraphicsRootSignature(rootsignature);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);

		_cmdList->DrawInstanced(3, 1, 0, 0);



		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &barrierDesc);

		_cmdList->Close();

		ID3D12CommandList* cmdLists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdLists);

		_cmdQueue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		auto result = _cmdAllocator->Reset();
		CheckResult(result);
		_cmdList->Reset(_cmdAllocator, nullptr);

		_swapchain->Present(1, 0);
	}
}
void Application::Terminate()
{
	UnregisterClass(w.lpszClassName, w.hInstance);
}
Application::~Application()
{
}

