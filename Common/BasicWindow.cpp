
#include "BasicWindow.h"
#include <minwindef.h>
#include <windef.h>
#include <winuser.h>

using MaxEngine::Common::BasicWindow;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

LRESULT CALLBACK BasicWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
        return true;
    BasicWindow *window = nullptr;
    /// 创建窗口的时候发送这个消息,并且会阻塞创建函数，我们可以从这里拿到我们附带在窗口上面的信息
    if (message == WM_CREATE)
    {
        window = reinterpret_cast<BasicWindow *>(((LPCREATESTRUCT)lParam)->lpCreateParams);
        window->setHWND(hwnd);
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<long long>(window));
    }
    window = reinterpret_cast<BasicWindow *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window)
        switch (message)
        {
        case WM_CREATE:
            return 0;
        case WM_SIZE:
            window->setWidth(LOWORD(lParam));
            window->setHeight(HIWORD(lParam));
            if (window->IsResizing())
            {
                /// do nothing
            }
            if (wParam == SIZE_MAXIMIZED)
            {
                window->OnResize(window->getWidth(), window->getHeight());
            }
            if (wParam == SIZE_RESTORED)
            {
                if (window->IsFirstShow() == true)
                {
                    window->isFirstShow = false;
                    /// do nothing
                }
                else
                {
                    window->OnResize(window->getWidth(), window->getHeight());
                }
            }
            return 0;
        case WM_ENTERSIZEMOVE:
            window->setIsResizing(true);
            return 0;
        case WM_EXITSIZEMOVE:
            window->setIsResizing(false);
            window->OnResize(window->getWidth(), window->getHeight());
            return 0;
        case WM_KEYDOWN:
            return 0;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        default:
            break;
        }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

BasicWindow::BasicWindow(HINSTANCE hi, UINT wid, UINT hei, const std::string &title)
    : instance_handle_(hi), window_width_(wid), window_height_(hei), window_title_(title)
{
    InitWindow();
}

bool BasicWindow::InitWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = BasicWindow::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance_handle_;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, "RegisterClass Failed.", 0, 0);
        return false;
    }

    // 我们这里设置的800x600的大小实际是包括上面的标题框的，我们要在这里计算出准确的中间的大小
    RECT R = {0, 0, window_width_, window_height_};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;
    window_handle_ = CreateWindow(wc.lpszClassName, window_title_.c_str(), WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0,
                                  instance_handle_, reinterpret_cast<LPVOID>(this));
    if (!window_handle_)
    {
        MessageBox(0, "CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(window_handle_, SW_SHOW);
    UpdateWindow(window_handle_);

    return true;
}

int BasicWindow::Run()
{

    MSG msg{};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            CustomHandler();
        }
    }
    return (int)msg.wParam;
}

void BasicWindow::setHWND(HWND wd)
{
    window_handle_ = wd;
}

void BasicWindow::setWidth(UINT wt)
{
    window_width_ = wt;
}
void BasicWindow::setHeight(UINT ht)
{
    window_height_ = ht;
}
void BasicWindow::setIsResizing(bool st)
{
    isResizing = st;
}
void BasicWindow::SetImguiContext(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);
}
HWND BasicWindow::getHWND() const
{
    return window_handle_;
}
/**
 * @brief 获取窗口长宽(只计算用户区，也就是没有计算标题栏和菜单栏等大小A)
 *
 * @return UINT
 */
UINT BasicWindow::getHeight() const
{
    return window_height_;
}
UINT BasicWindow::getWidth() const
{
    return window_width_;
}
bool BasicWindow::IsResizing() const
{
    return isResizing;
}
bool BasicWindow::IsFirstShow() const
{
    return isFirstShow;
}