#ifndef D3DBASICSETUP_H
#define D3DBASICSETUP_H

#ifndef EXPORTTING
#define DECLSPEC __declspec(dllimport)
#else
#define DECLSPEC __declspec(dllexport)
#endif  // EXPORTTING

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>

#include "GameTimer.h"
#include "dxgi.h"
#include "dxgi1_4.h"
#include "minwindef.h"

class D3DBasicSetUp {
 protected:
  /// 工厂对象
  Microsoft::WRL::ComPtr<IDXGIFactory4> dxgi_factory_;
  /// 交换链对象
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
  /// 硬件对象
  Microsoft::WRL::ComPtr<ID3D12Device> d3d_device_;
  /// CPU和GPU之间的屏障对象
  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  /// 命令队列
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_;
  /// 命令列表
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> direct_list_;
  /// 命令分配器
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> direct_allocator_;
  /// rtv堆
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap_;
  /// dsv堆
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap_;
  /// 描述符的大小
  /// RenderTargetView 只写的渲染对象
  UINT rtv_size_{};
  /// DepthStencilView 只读的深度缓冲对象
  UINT dsv_size_{};
  /// UnorderedAccessView ConstantBufferView ShaderResourceView
  /// 可以随机读写的对象 这三个大小是一致的
  UINT cbv_srv_uav_size_{};
  /// 控制是否开启抗锯齿
  bool MSAA4x_state_ = false;
  /// 抗锯齿等级
  int MSAA4x_quality_level_ = false;
  /// 记录在内存中的屏障值
  unsigned long long current_fence_;
  /// 应用程序的实例句柄
  HINSTANCE instance_ = nullptr;
  /// 主窗口的实例句柄
  HWND handle_mainWnd_ = nullptr;
  /// 使用几个缓冲区
  static const int swap_chain_buffer_cnt_ = 2;
  Microsoft::WRL::ComPtr<ID3D12Resource>
      swap_chain_buffer_[swap_chain_buffer_cnt_];
  Microsoft::WRL::ComPtr<ID3D12Resource> depth_stencil_buffer_;
  /// 当前是哪个缓冲区（以便于我们知道现在该绘制哪一个）
  int current_back_buffer_idx_ = 0;
  /// 窗口宽高
  UINT client_width_ = 800;
  UINT client_height_ = 600;
  /// 后台缓冲区的数据格式
  DXGI_FORMAT back_buffer_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
  /// 深度缓冲区的数据格式
  DXGI_FORMAT depth_stencil_format_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
  /// 视口
  D3D12_VIEWPORT screen_viewport_;
  /// 裁剪矩形
  D3D12_RECT scissor_rect_;

 protected:
  /**
   * @brief 当前显示的缓冲区
   *
   * @return ID3D12Resource* 指向缓冲区的指针
   */
  ID3D12Resource* CurrentBackBuffer() const;
  /**
   * @brief 初始化D3D12.
   *
   * \return 是否初始化成功
   */
  bool InitializeD3D();
  /**
   * @brief 创建命令队列，命令列表和命令分配器.
   */
  void CreateCommandObjects();
  /**
   * @brief 创建交换链.
   *
   */
  void CreateSwapChain();
  /**
   * @brief 创建描述符堆.
   *
   */
  void CreateDescriptorHeaps();
  /**
   * @brief 刷新命令队列，本质是等待CPU和GPU同步
   *
   */
  void FlushCommandQueue();
  /**
   * @brief 获得当前缓冲区描述符
   *
   * @return D3D12_CPU_DESCRIPTOR_HANDLE
   */
  D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferDesc() const;
  /**
   * @brief 获得深度缓冲区描述符
   *
   * @return D3D12_CPU_DESCRIPTOR_HANDLE
   */
  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDesc() const;
  /**
   * @brief 其实具体干啥我也不知道 2024.4.22
   *
   */
  void OnResize();
  /**
   * @brief 整体初始化
   *
   * @return true 是否初始化成功
   * @return false
   */
  bool initialize();
  /**
   * @brief 渲染函数
   *
   */
  virtual void Draw(const GameTimer&) = 0;

 public:
  D3DBasicSetUp(HWND);
  ~D3DBasicSetUp();
};

#endif