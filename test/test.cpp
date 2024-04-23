#include <Windows.h>
#include <minwindef.h>
#include <winbase.h>
#include <winuser.h>

#include "MyD3D.h"

HWND ghMainWnd = nullptr;

/**
 * @brief 主窗口消息处理函数
 *
 * @param hwnd 主窗口句柄
 * @param message 消息类型
 * @param wParam 消息所带的参数
 * @param lParam 不知道有啥用
 * @return LRESULT
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
    case WM_LBUTTONDOWN:
      MessageBox(nullptr, "Hello World!", "Hello", MB_OK);
      return 0;
    case WM_KEYDOWN:
      if (wParam == VK_ESCAPE) {
        DestroyWindow(hwnd);
      }
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      break;
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}
bool InitWindowsApp(HINSTANCE histance, int show) {
  WNDCLASS wc;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = histance;
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = "BasicWindow";
  if (!RegisterClass(&wc)) {
    MessageBox(nullptr, "Window Registration Failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
  }
  ghMainWnd = CreateWindow("BasicWindow", "Win32Basic", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, nullptr, nullptr, histance, nullptr);
  ShowWindow(ghMainWnd, show);
  UpdateWindow(ghMainWnd);
  return true;
}

int Run() {
  MSG msg{};
  while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      /// do nothing
    }
  }

  return (int)msg.wParam;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nShowCmd) {
  if (!InitWindowsApp(hInstance, nShowCmd)) return -1;
  MYD3D app{ghMainWnd};
  return Run();
}
