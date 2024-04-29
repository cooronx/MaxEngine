#include <Windows.h>
#include <minwindef.h>
#include <winbase.h>
#include <winuser.h>

#include <iostream>
#include <memory>

#include "D3D12Util.h"
#include "MyD3D.h"

#include "BasicWindow.h"

// HWND ghMainWnd = nullptr;
// /**
//  * @brief 主窗口消息处理函数
//  *
//  * @param hwnd 主窗口句柄
//  * @param message 消息类型
//  * @param wParam 消息所带的参数
//  * @param lParam 不知道有啥用
//  * @return LRESULT
//  */
// LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
//     LPARAM lParam)
// {
//     if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
//         return true;
//     switch (message) {
//     case WM_SIZE:
//         // 在这里修改size
//     case WM_KEYDOWN:
//         if (wParam == VK_ESCAPE) {
//             DestroyWindow(hwnd);
//         }
//         return 0;
//     case WM_DESTROY:
//         PostQuitMessage(0);
//         return 0;
//     default:
//         break;
//     }
//     return DefWindowProc(hwnd, message, wParam, lParam);
// }

// bool InitWindowsApp(HINSTANCE histance, int show)
// {
//     WNDCLASS wc;
//     wc.style = CS_HREDRAW | CS_VREDRAW;
//     wc.lpfnWndProc = WndProc;
//     wc.cbClsExtra = 0;
//     wc.cbWndExtra = 0;
//     wc.hInstance = histance;
//     wc.hIcon = LoadIcon(0, IDI_APPLICATION);
//     wc.hCursor = LoadCursor(0, IDC_ARROW);
//     wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
//     wc.lpszMenuName = 0;
//     wc.lpszClassName = "MainWnd";

//     if (!RegisterClass(&wc)) {
//         MessageBox(0, "RegisterClass Failed.", 0, 0);
//         return false;
//     }

//     // 我们这里设置的800x600的大小实际是包括上面的标题框的，我们要在这里计算出准确的中间的大小
//     RECT R = { 0, 0, 800, 600 };
//     AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
//     int width = R.right - R.left;
//     int height = R.bottom - R.top;

//     ghMainWnd = CreateWindow("MainWnd", nullptr,
//         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, histance, 0);
//     if (!ghMainWnd) {
//         MessageBox(0, "CreateWindow Failed.", 0, 0);
//         return false;
//     }

//     ShowWindow(ghMainWnd, SW_SHOW);
//     UpdateWindow(ghMainWnd);

//     return true;
// }

class TestWindow : public BasicWindow
{
    public:
    TestWindow(HINSTANCE hi, UINT wid, UINT hei, const std::string &title)
        : BasicWindow(hi, wid, hei, title)
    {
    }
    void OnResize(LONG new_width, LONG new_height) override
    {
    }
    void CustomHandler() override
    {
    }
};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    D3DApp app{hInstance, 800, 600, "test"};

    return app.Run();
}
