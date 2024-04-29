#ifndef UPLOADBUFFER_H
#define UPLOADBUFFER_H
#include <combaseapi.h>
#include <d3d12.h>
#include <minwindef.h>

#include <cstring>
#include <memory>

#include "D3D12Util.h"
#include "d3dx12.h"
#include "wrl/client.h"

using namespace Util;

template <class DataType>
class UpLoadBuffer {
 public:
  UpLoadBuffer(ID3D12Device* device, UINT element_count, bool is_constant)
      : is_constant_(is_constant) {
    element_byte_size_ = sizeof(DataType);
    if (is_constant_) {
      element_byte_size_ =
          HelperFuncs::CalculateConstantBufferByteSize(element_byte_size_);
    }
    /// 创建一个上传堆
    auto property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resource =
        CD3DX12_RESOURCE_DESC::Buffer(element_count * element_byte_size_);
    ThrowIfFailed(device->CreateCommittedResource(
        &property, D3D12_HEAP_FLAG_NONE, &resource,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(buffer_.GetAddressOf())));
    /// 将上传堆的指针映射到内存中
    buffer_->Map(0, nullptr,
                 std::reinterpret_pointer_cast<void**>(&mapped_data_));
  }

  ~UpLoadBuffer() {
    if (buffer_ != nullptr) {
      buffer_->Unmap(0, nullptr);
    }
    mapped_data_ = nullptr;
  }
  ID3D12Resource* Resource() const { return buffer_.Get(); }
  void CopyData(UINT element_start_idx, DataType* copy_data, UINT data_size) {
    memcpy(&mapped_data_[element_start_idx * element_byte_size_], copy_data,
           data_size);
  }

 private:
  Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
  /// 标识当前是否为常量缓冲区，因为常量缓冲区需要有特殊的对齐方式
  bool is_constant_ = false;
  BYTE* mapped_data_ = nullptr;
  UINT element_byte_size_;
};
#endif