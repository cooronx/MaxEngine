#include "D3DBasicSetUp.h"

#include <dxgitype.h>
#include <handleapi.h>
#include <synchapi.h>
#include <winbase.h>
#include <winnt.h>

#include <iostream>

#include "D3D12Util.h"

using Microsoft::WRL::ComPtr;
using namespace Util;

D3DBasicSetUp::D3DBasicSetUp() {}

bool D3DBasicSetUp::Initialize()
{
    if (handle_mainWnd_ == nullptr)
        return false;
    if (!InitializeD3D())
        return false;
    return true;
}

D3DBasicSetUp::~D3DBasicSetUp() {}

void D3DBasicSetUp::SetHWND(HWND hd)
{
    handle_mainWnd_ = hd;
}

void D3DBasicSetUp::CreateCommandObjects()
{
    /// 创建命令队列
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    /// 使用GPU直接可以执行的命令，与之相对的是一组打包的绘制命令
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(d3d_device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue_)));
    /// 创建命令分配器
    ThrowIfFailed(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                      IID_PPV_ARGS(&direct_allocator_)));
    /// 创建命令列表
    ThrowIfFailed(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 direct_allocator_.Get(), nullptr,
                                                 IID_PPV_ARGS(&direct_list_)));

    direct_list_->Close();
}

void D3DBasicSetUp::CreateSwapChain()
{
    // 因为我们可能多次调用这个方法（修改swap
    // chain），所以我们要先把之前的交换链释放了
    swap_chain_.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.RefreshRate.Numerator = 60; /// 刷新率 只不过是以分母和分子的形式展示的
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = back_buffer_format_;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = MSAA4x_state_ ? 4 : 1;
    sd.SampleDesc.Quality = MSAA4x_state_ ? (MSAA4x_state_ - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = swap_chain_buffer_cnt_;
    sd.OutputWindow = handle_mainWnd_;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(
        dxgi_factory_->CreateSwapChain(command_queue_.Get(), &sd, swap_chain_.GetAddressOf()));
}

void D3DBasicSetUp::CreateDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = swap_chain_buffer_cnt_;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(
        d3d_device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtv_heap_.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(
        d3d_device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsv_heap_.GetAddressOf())));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DBasicSetUp::CurrentBackBufferDesc() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtv_heap_->GetCPUDescriptorHandleForHeapStart(),
                                         current_back_buffer_idx_, rtv_size_);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DBasicSetUp::DepthStencilDesc() const
{
    return dsv_heap_->GetCPUDescriptorHandleForHeapStart();
}

void D3DBasicSetUp::FlushCommandQueue()
{
    ++current_fence_;
    /// 设置GPU中的围栏值为current_fence_，这条命令并不会立刻执行，而是和其他命令一样，按顺序执行
    ThrowIfFailed(command_queue_->Signal(fence_.Get(), current_fence_));

    /// 获取当前GPU中围栏值
    if (fence_->GetCompletedValue() < current_fence_)
    {
        HANDLE event_handle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        /// 设置当达到了current_fence_之后，触发指定event
        ThrowIfFailed(fence_->SetEventOnCompletion(current_fence_, event_handle));
        WaitForSingleObject(event_handle, INFINITE);
        CloseHandle(event_handle);
    }
}

void D3DBasicSetUp::OnResize(UINT width, UINT height)
{
    // 先等待GPU把之前的命令执行完
    FlushCommandQueue();
    client_width_ = width;
    client_height_ = height;

    ThrowIfFailed(direct_list_->Reset(direct_allocator_.Get(), nullptr));

    // Release the previous resources we will be recreating.
    for (int i = 0; i < swap_chain_buffer_cnt_; ++i)
        swap_chain_buffer_[i].Reset();
    depth_stencil_buffer_.Reset();

    // Resize the swap chain.
    ThrowIfFailed(swap_chain_->ResizeBuffers(swap_chain_buffer_cnt_, client_width_, client_height_,
                                             back_buffer_format_,
                                             DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    current_back_buffer_idx_ = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtv_heap_->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < swap_chain_buffer_cnt_; i++)
    {
        ThrowIfFailed(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&swap_chain_buffer_[i])));
        d3d_device_->CreateRenderTargetView(swap_chain_buffer_[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, rtv_size_);
    }

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = client_width_;
    depthStencilDesc.Height = client_height_;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

    // Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to
    // read from the depth buffer.  Therefore, because we need to create two views
    // to the same resource:
    //   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    //   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
    // we need to create the depth buffer resource with a typeless format.
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = MSAA4x_state_ ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = MSAA4x_state_ ? (MSAA4x_quality_level_ - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = depth_stencil_format_;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(d3d_device_->CreateCommittedResource(
        &heap_property, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON,
        &optClear, IID_PPV_ARGS(depth_stencil_buffer_.GetAddressOf())));

    // Create descriptor to mip level 0 of entire resource using the format of the
    // resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = depth_stencil_format_;
    dsvDesc.Texture2D.MipSlice = 0;
    d3d_device_->CreateDepthStencilView(depth_stencil_buffer_.Get(), &dsvDesc, DepthStencilDesc());

    // Transition the resource from its initial state to be used as a depth
    // buffer.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        depth_stencil_buffer_.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    direct_list_->ResourceBarrier(1, &barrier);

    // Execute the resize commands.
    ThrowIfFailed(direct_list_->Close());
    ID3D12CommandList *cmdsLists[] = {direct_list_.Get()};
    command_queue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until resize is complete.
    FlushCommandQueue();

    // Update the viewport transform to cover the client area.
    screen_viewport_.TopLeftX = 0;
    screen_viewport_.TopLeftY = 0;
    screen_viewport_.Width = static_cast<float>(client_width_);
    screen_viewport_.Height = static_cast<float>(client_height_);
    screen_viewport_.MinDepth = 0.0f;
    screen_viewport_.MaxDepth = 1.0f;

    scissor_rect_ = {0, 0, static_cast<LONG>(client_width_), static_cast<LONG>(client_height_)};
}

bool D3DBasicSetUp::InitializeD3D()
{
    /// 首先创建Debug层
#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory_));
    HRESULT res = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&d3d_device_));
    if (res < 0)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        dxgi_factory_->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_1,
                                        IID_PPV_ARGS(&d3d_device_)));
    }
    /// 创建屏障
    ThrowIfFailed(d3d_device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
    /// 获取描述符大小
    rtv_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsv_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    cbv_srv_uav_size_ =
        d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    /// 检测对4xMSAA的支持
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels;
    levels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    levels.SampleCount = 4;
    levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    levels.NumQualityLevels = 0;
    ThrowIfFailed(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                                   &levels, sizeof(levels)));
    MSAA4x_quality_level_ = levels.NumQualityLevels;
    if (MSAA4x_quality_level_ <= 0)
    {
        std::cerr << "Unexpected quality levels in MSAA" << "\n";
    }

    /// 创建命令队列和命令列表
    CreateCommandObjects();
    /// 创建交换链
    CreateSwapChain();
    /// 创建描述符堆
    CreateDescriptorHeaps();

    return true;
}

ID3D12Resource *D3DBasicSetUp::CurrentBackBuffer() const
{
    return swap_chain_buffer_[current_back_buffer_idx_].Get();
}