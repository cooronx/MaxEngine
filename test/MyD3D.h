#ifndef MYD3D_H
#define MYD3D_H

#include <DirectXColors.h>

#include "D3D12Util.h"
#include "D3DBasicSetUp.h"

using namespace Util;
using namespace DirectX;

class MYD3D : public D3DBasicSetUp {
 public:
  MYD3D(HWND wd) : D3DBasicSetUp(wd) {}
  virtual bool Initialize() override {
    if (D3DBasicSetUp::Initialize()) return true;
    return false;
  }
  void Draw(const GameTimer&) override {
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished
    // execution on the GPU.
    ThrowIfFailed(direct_allocator_->Reset());
    // A command list can be reset after it has been added to the command
    // queue via ExecuteCommandList. Reusing the command list reuses memory.
    ThrowIfFailed(direct_list_->Reset(direct_allocator_.Get(), nullptr));

    // Indicate a state transition on the resource usage.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    direct_list_->ResourceBarrier(1, &barrier);

    // Set the viewport and scissor rect.  This needs to be reset whenever the
    // command list is reset.
    direct_list_->RSSetViewports(1, &screen_viewport_);
    direct_list_->RSSetScissorRects(1, &scissor_rect_);

    // Clear the back buffer and depth buffer.
    direct_list_->ClearRenderTargetView(CurrentBackBufferDesc(),
                                        Colors::LightSteelBlue, 0, nullptr);
    direct_list_->ClearDepthStencilView(
        DepthStencilDesc(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    auto back_buffer_desc = CurrentBackBufferDesc();
    auto depth_desc = DepthStencilDesc();
    direct_list_->OMSetRenderTargets(1, &back_buffer_desc, true, &depth_desc);

    // Indicate a state transition on the resource usage.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    direct_list_->ResourceBarrier(1, &barrier);

    // Done recording commands.
    ThrowIfFailed(direct_list_->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = {direct_list_.Get()};
    command_queue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // swap the back and front buffers
    ThrowIfFailed(swap_chain_->Present(0, 0));
    current_back_buffer_idx_ =
        (current_back_buffer_idx_ + 1) % swap_chain_buffer_cnt_;

    // Wait until frame commands are complete.  This waiting is inefficient and
    // is done for simplicity.  Later we will show how to organize our rendering
    // code so we do not have to wait per frame.
    FlushCommandQueue();
  }
};

#endif