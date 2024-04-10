#include "D3DApp.h"

using Microsoft::WRL::ComPtr;
using namespace Util;

D3DApp::D3DApp(HINSTANCE hi) : instance_{hi} {}

bool D3DApp::initialize() {
  if (!InitializeD3D()) return false;
  return false;
}

D3DApp::~D3DApp() {}

void D3DApp::CreateCommandObjects() {
  /// �����������
  D3D12_COMMAND_QUEUE_DESC queueDesc{};
  /// ʹ��GPUֱ�ӿ���ִ�е������֮��Ե���һ�����Ļ�������
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  ThrowIfFailed(d3d_device_->CreateCommandQueue(&queueDesc,
                                                IID_PPV_ARGS(&command_queue_)));
  /// �������������
  ThrowIfFailed(d3d_device_->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&direct_allocator_)));
  /// ���������б�
  ThrowIfFailed(d3d_device_->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_DIRECT, direct_allocator_.Get(), nullptr,
      IID_PPV_ARGS(&direct_list_)));

  direct_list_->Close();
}

void D3DApp::CreateSwapChain() {
  // ��Ϊ���ǿ��ܶ�ε�������������޸�swap
  // chain������������Ҫ�Ȱ�֮ǰ�Ľ������ͷ���
  swap_chain_.Reset();

  DXGI_SWAP_CHAIN_DESC sd;
  sd.BufferDesc.Width = client_width_;
  sd.BufferDesc.Height = client_height_;
  sd.BufferDesc.RefreshRate.Numerator =
      60;  /// ˢ���� ֻ�������Է�ĸ�ͷ��ӵ���ʽչʾ��
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

  ThrowIfFailed(dxgi_factory_->CreateSwapChain(command_queue_.Get(), &sd,
                                               swap_chain_.GetAddressOf()));
}

void D3DApp::CreateDescriptorHeaps() {
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
  rtvHeapDesc.NumDescriptors = swap_chain_buffer_cnt_;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtvHeapDesc.NodeMask = 0;
  ThrowIfFailed(d3d_device_->CreateDescriptorHeap(
      &rtvHeapDesc, IID_PPV_ARGS(rtv_heap_.GetAddressOf())));

  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  dsvHeapDesc.NodeMask = 0;
  ThrowIfFailed(d3d_device_->CreateDescriptorHeap(
      &dsvHeapDesc, IID_PPV_ARGS(dsv_heap_.GetAddressOf())));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferDesc() const {
  return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      rtv_heap_->GetCPUDescriptorHandleForHeapStart(), current_back_buffer_idx_,
      rtv_size_);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilDesc() const {
  return dsv_heap_->GetCPUDescriptorHandleForHeapStart();
}

bool D3DApp::InitializeD3D() {
  /// ���ȴ���Debug��
#if defined(_DEBUG)
  {
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
  }
#endif
  CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory_));
  HRESULT res = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1,
                                  IID_PPV_ARGS(&d3d_device_));
  if (res < 0) {
    ComPtr<IDXGIAdapter> warpAdapter;
    dxgi_factory_->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
    ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_1,
                                    IID_PPV_ARGS(&d3d_device_)));
  }
  /// ��������
  ThrowIfFailed(d3d_device_->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                         IID_PPV_ARGS(&fence_)));
  /// ��ȡ��������С
  rtv_size_ = d3d_device_->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  dsv_size_ = d3d_device_->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  cbv_srv_uav_size_ = d3d_device_->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  /// ����4xMSAA��֧��
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels;
  levels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  levels.SampleCount = 4;
  levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  levels.NumQualityLevels = 0;
  ThrowIfFailed(d3d_device_->CheckFeatureSupport(
      D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels)));
  if (levels.NumQualityLevels <= 0) {
    std::cerr << "Unexpected quality levels in MSAA"
              << "\n";
  }

  /// ����������к������б�
  CreateCommandObjects();
  /// ����������
  CreateSwapChain();

  return false;
}