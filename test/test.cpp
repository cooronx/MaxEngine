﻿#include <Eigen/Core>
#include <Eigen/Dense>
#include <UtilType.h>
#include <Windows.h>
#include <minwindef.h>
#include <winbase.h>
#include <winuser.h>

#include <iostream>
#include <memory>

#include "D3D12Util.h"
#include "MyD3D.h"

#include "BasicWindow.h"

using namespace MaxEngine::Common;

class D3DApp : public BasicWindow
{
public:
    D3DApp(HINSTANCE hi, UINT wid, UINT hei, const std::string &title)
        : BasicWindow(hi, wid, hei, title)
    {
        render_.SetHWND(this->getHWND());
        render_.Initialize();
        OnResize(wid, hei);
        BasicWindow::SetImguiContext(render_.GetImguiCtx());
    }
    void OnResize(LONG new_width, LONG new_height) override
    {
        render_.Resize(new_width, new_height);
    }
    /**
     * @brief 每帧调用这个函数
     *
     */
    void CustomHandler() override
    {
        render_.Update();
        render_.Draw({});
    }

private:
    MYD3D render_;
};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    try
    {
        // AllocConsole();
        // freopen("CONOUT$", "w", stdout);
        // std::cout << "hello world" << "\n";
        D3DApp app{hInstance, 800, 600, "test"};
        return app.Run();
    }
    catch (DxException &e)
    {
        MessageBox(nullptr, e.what(), "HR Failed", MB_OK);
        return 0;
    }
}
