#ifndef MYD3D_H
#define MYD3D_H

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <combaseapi.h>
#include <d3d12.h>
#include <memory>
#include <minwinbase.h>
#include <stdlib.h>
#include <windef.h>

#include "BasicWindow.h"
#include "D3D12Util.h"
#include "D3DBasicSetUp.h"
#include "UpLoadBuffer.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "wrl/client.h"

using namespace Util;
using namespace DirectX;

struct TransMatrix
{
    /// mvp 矩阵
    XMFLOAT4X4 mvp_mat;
};

class MYD3D : public D3DBasicSetUp
{
    public:
    MYD3D() : D3DBasicSetUp()
    {
    }
    ~MYD3D()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    /**
     * @brief 创建Imgui的描述符堆（用在imgui自己的渲染）
     *
     */
    void createImguiSRVHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC srv_desc;
        srv_desc.NumDescriptors = 1;
        srv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; /// 必须要设置为shader visible
        srv_desc.NodeMask = 0;
        ThrowIfFailed(d3d_device_->CreateDescriptorHeap(
            &srv_desc, IID_PPV_ARGS(imgui_srv_heap_.GetAddressOf())));
    }
    /**
     * @brief 初始化Imgui
     *
     */
    void InitImgui()
    {
        /// 加入imgui初始化
        /// 设置imgui上下文
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        ImGui_ImplWin32_Init(handle_mainWnd_);
        ImGui_ImplDX12_Init(d3d_device_.Get(), swap_chain_buffer_cnt_, back_buffer_format_,
                            imgui_srv_heap_.Get(),
                            imgui_srv_heap_->GetCPUDescriptorHandleForHeapStart(),
                            imgui_srv_heap_->GetGPUDescriptorHandleForHeapStart());
    }
    void CreateCBVHeaps()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;
        ThrowIfFailed(
            d3d_device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(cbv_heap_.GetAddressOf())));
    }
    void CreateConstBuffer()
    {
    }
    virtual bool Initialize() override
    {
        if (!D3DBasicSetUp::Initialize())
            return false;
        createImguiSRVHeap();
        InitImgui();
        CreateCBVHeaps();

        return true;
    }
    /**
     * @brief 当窗口大小发生变化时
     *
     * @param width 宽度
     * @param height 高度
     */
    void Resize(LONG width, LONG height)
    {
        client_width_ = width;
        client_height_ = height;
        D3DBasicSetUp::OnResize();
    }
    /**
     * @brief 绘制
     *
     */
    void Draw(const GameTimer &) override
    {
        // 复用命令列表和分配器
        ThrowIfFailed(direct_allocator_->Reset());
        ThrowIfFailed(direct_list_->Reset(direct_allocator_.Get(), nullptr));

        /// imgui渲染
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        // {
        //     static int counter = 0;
        //     ImGui::Begin(
        //         "Hello, world!"); // Create a window called "Hello, world!" and append into it.
        //     ImGui::Text("This is some useful text."); // Display some text (you can use a format
        //                                               // strings too)
        //     ImGui::Checkbox("Demo Window",
        //                     &show_demo_window); // Edit bools storing our window open/close state
        //     ImGui::SliderFloat("float", &test_input, 0.1f,
        //                        1.0f);    // Edit 1 float using a slider from 0.0f to 1.0f
        //     if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return
        //                                  // true when edited/activated)
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);
        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        //                 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //     ImGui::End();
        // }

        // Indicate a state transition on the resource usage.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        direct_list_->ResourceBarrier(1, &barrier);

        // Set the viewport and scissor rect.  This needs to be reset whenever the
        // command list is reset.
        direct_list_->RSSetViewports(1, &screen_viewport_);
        direct_list_->RSSetScissorRects(1, &scissor_rect_);

        // Clear the back buffer and depth buffer.
        direct_list_->ClearRenderTargetView(CurrentBackBufferDesc(), Colors::LightSteelBlue, 0,
                                            nullptr);
        direct_list_->ClearDepthStencilView(DepthStencilDesc(),
                                            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f,
                                            0, 0, nullptr);

        // Specify the buffers we are going to render to.
        auto back_buffer_desc = CurrentBackBufferDesc();
        auto depth_desc = DepthStencilDesc();
        direct_list_->OMSetRenderTargets(1, &back_buffer_desc, true, &depth_desc);
        /// imgui绘制
        ID3D12DescriptorHeap *heaps[] = {imgui_srv_heap_.Get()};
        direct_list_->SetDescriptorHeaps(_countof(heaps), heaps);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), direct_list_.Get());
        //  Indicate a state transition on the resource usage.
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        direct_list_->ResourceBarrier(1, &barrier);

        // Done recording commands.
        ThrowIfFailed(direct_list_->Close());

        // Add the command list to the queue for execution.
        ID3D12CommandList *cmdsLists[] = {direct_list_.Get()};
        command_queue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // swap the back and front buffers
        ThrowIfFailed(swap_chain_->Present(0, 0));
        current_back_buffer_idx_ = (current_back_buffer_idx_ + 1) % swap_chain_buffer_cnt_;

        // Wait until frame commands are complete.  This waiting is inefficient and
        // is done for simplicity.  Later we will show how to organize our rendering
        // code so we do not have to wait per frame.
        FlushCommandQueue();
    }
    /**
     * @brief 获取Imgui上下文
     *
     * @return ImGuiContext* 上下文指针
     */
    ImGuiContext *GetImguiCtx()
    {
        return ImGui::GetCurrentContext();
    }

    private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> imgui_srv_heap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbv_heap_;
};

#endif