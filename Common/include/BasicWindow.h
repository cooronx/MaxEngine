#ifndef BASICWINDOW_H
#define BASICWINDOW_H

#include "imgui.h"
#include <Windows.h>
#include <minwindef.h>
#include <string>
#include <winbase.h>
#include <winnt.h>
#include <winuser.h>

/**
 * @brief 要想正确使用Imgui的功能，要传递上下文指针，上下文无法在不同模块（dll）中共享
 *
 */
class BasicWindow
{
    private:
    /// 窗口所属的实例句柄
    HINSTANCE instance_handle_ = nullptr;
    /// 窗口宽高
    LONG window_width_;
    LONG window_height_;
    /// 窗口标题
    std::string window_title_;
    /// 窗口句柄
    HWND window_handle_ = nullptr;
    /// 当前是否进行大小变化
    bool isResizing = false;

    private:
    bool InitWindow();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    protected:
    /// 具体处理函数，应该由子类重写
    virtual void OnResize(LONG new_width, LONG new_height) = 0;
    virtual void CustomHandler() = 0;

    public:
    explicit BasicWindow(HINSTANCE, UINT, UINT, const std::string &);
    /// 主循环
    int Run();
    /// 读取函数
    void setHWND(HWND);
    void setHeight(UINT);
    void setWidth(UINT);
    void setIsResizing(bool);
    void SetImguiContext(ImGuiContext *);
    HWND getHWND() const;
    UINT getHeight() const;
    UINT getWidth() const;
    bool IsResizing() const;
};

#endif