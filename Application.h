#pragma once
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif


class Wrapper;
class SceneManager;
class Application
{
private:
	
	static std::shared_ptr<SceneManager> _sceneManager;
	static UINT window_width;							//�E�B���h�E��
	static UINT window_height;							//�E�B���h�E��
	static const UINT windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	static WNDCLASSEX w;
	static LPRECT wrc;
	static POINT center;

	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
	static void CreateGameWindow(HWND& hwnd, WNDCLASSEX& w);
	static void ToggleFullscreenWindow(Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain);
	static bool fullscreenMode;
	static HWND hwnd;
	static bool isPause;
	static bool isMenu;
	static HMENU ButtonID;
	static bool isMoveKeyDown;
	static bool isMoveKeyUp;
	static bool isKeyJump;
	static bool isShowHowToPlay;
protected:
public:
	static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static std::shared_ptr<Wrapper> _dx;
	static SIZE GetWindowSize();
	//Application�̃V���O���g���C���X�^���X�𓾂�
	static Application& Instance();
	static void DebugOutputFormatString(const char* format, ...);
	static POINT GetCenter() { return center; }
	static HWND GetHwnd() { return hwnd; }
	static WNDCLASSEX GetW() { return w; }
	static bool GetPause() { return isPause; }
	static bool GetMenu() { return isMenu; }
	static bool GetIsShowHowToPlay() { return isShowHowToPlay; }
	static void SetMenu() { isMenu = false; }
	static HMENU GetButtonID() { return ButtonID; }
	static void SetButtonID(HMENU id) { ButtonID = id; }
	static void SetIsMoveKeyDown(bool flag) { isMoveKeyDown = flag; }
	static void SetIsMoveKeyUp(bool flag) { isMoveKeyUp = flag; }
	static void SetIsKeyJump(bool flag) { isKeyJump = flag; }
	static void SetIsShowHowToPlay(bool flag) { isShowHowToPlay = flag; }

	static bool GetIsMoveKeyDown() { return isMoveKeyDown; }
	static bool GetIsMoveKeyUp() { return isMoveKeyUp; }
	static bool GetIsKeyJump() { return isKeyJump; }
	
	//������
	bool Init();
	//���[�v�N��
	void Run();
	//�㏈��
	void Terminate();
	~Application();
};
