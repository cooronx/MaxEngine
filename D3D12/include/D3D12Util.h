#ifndef D3D12UTIL_H
#define D3D12UTIL_H

#include "d3dx12.h"
#include <WTypesbase.h>
#include <comdef.h>

#include <cstddef>
#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <exception>
#include <intsafe.h>
#include <string>
#include <wrl.h>

namespace Util
{

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                                           \
    {                                                                                              \
        HRESULT hr__ = (x);                                                                        \
        std::string fn__ = __FILE__;                                                               \
        if (FAILED(hr__))                                                                          \
        {                                                                                          \
            throw DxException{#x, fn__, __LINE__, hr__};                                           \
        }                                                                                          \
    }
#endif
class DxException : public std::exception
{
    private:
    std::string function_name_;
    std::string file_name_;
    std::string full_error_msg_{};
    int line_number_;
    HRESULT error_code_;

    public:
    DxException(std::string function_name, std::string filename, int line_num, HRESULT err_code);
    const char *what() const override;
};

class HelperFuncs
{
    public:
    /**
     * @brief 创建一个默认缓冲区，用一个上传堆来完成这件事情
     *
     * @param device
     * @param cmdList
     * @param initData
     * @param byteSize
     * @param uploadBuffer
     * @return Microsoft::WRL::ComPtr<ID3D12Resource> 默认缓冲区
     */
    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device *device, ID3D12GraphicsCommandList *cmdList, const void *initData,
        UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource> &uploadBuffer)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

        // Create the actual default buffer resource.
        auto property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        ThrowIfFailed(device->CreateCommittedResource(&property, D3D12_HEAP_FLAG_NONE, &desc,
                                                      D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                      IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

        // In order to copy CPU memory data into our default buffer, we need to create
        // an intermediate upload heap.
        property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        ThrowIfFailed(device->CreateCommittedResource(&property, D3D12_HEAP_FLAG_NONE, &desc,
                                                      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                      IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

        // Describe the data we want to copy into the default buffer.
        D3D12_SUBRESOURCE_DATA subResourceData = {};
        subResourceData.pData = initData;
        subResourceData.RowPitch = byteSize;
        subResourceData.SlicePitch = subResourceData.RowPitch;

        // Schedule to copy the data to the default buffer resource.  At a high level, the helper
        // function UpdateSubresources will copy the CPU memory into the intermediate upload heap.
        // Then, using ID3D12CommandList::CopySubresourceRegion, the intermediate upload heap data
        // will be copied to mBuffer.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        cmdList->ResourceBarrier(1, &barrier);
        UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1,
                              &subResourceData);
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        cmdList->ResourceBarrier(1, &barrier);

        // Note: uploadBuffer has to be kept alive after the above function calls because
        // the command list has not been executed yet that performs the actual copy.
        // The caller can Release the uploadBuffer after it knows the copy has been executed.

        return defaultBuffer;
    }
    /**
     * @brief 计算常量缓冲区对齐后的大小，因为hlsl要求对齐两个字节
     *
     * @param real_size 当前缓冲区对象的实际大小
     * @return UINT 对齐后的大小
     */
    static UINT CalculateConstantBufferByteSize(UINT real_size)
    {
        /// 常量缓冲区 要求大小为256的整数倍
        /// 所以我们在这里进行转换
        return (real_size + 255) & ~255;
    }
    /**
     * @brief 编译着色器
     *
     * @param filename 着色器文件地址
     * @param defines 不知道有啥用
     * @param entrypoint 入口函数的名字
     * @param target 指定要编译的着色器目标或着色器功能集
     * @return Microsoft::WRL::ComPtr<ID3DBlob> 编译完成的着色器字节码
     */
    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring &filename,
                                                          const D3D_SHADER_MACRO *defines,
                                                          const std::string &entrypoint,
                                                          const std::string &target)
    {
        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        HRESULT hr = S_OK;

        Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;
        hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode,
                                &errors);
        if (errors != nullptr)
            OutputDebugStringA((char *)errors->GetBufferPointer());

        ThrowIfFailed(hr);

        return byteCode;
    }
};

} // namespace Util
#endif