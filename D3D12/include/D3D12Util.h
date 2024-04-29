#ifndef D3D12UTIL_H
#define D3D12UTIL_H

#include <WTypesbase.h>
#include <comdef.h>

#include <exception>
#include <string>

namespace Util {

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                       \
  {                                                                            \
    HRESULT hr__ = (x);                                                        \
    std::string fn__ = __FILE__;                                               \
    if (hr__ < 0) {                                                            \
      throw DxException{#x, fn__, __LINE__, hr__};                             \
    }                                                                          \
  }
#endif

class HelperFuncs {
public:
  static UINT CalculateConstantBufferByteSize(UINT real_size) {
    /// 常量缓冲区 要求大小为256的整数倍
    /// 所以我们在这里进行转换
    return (real_size + 255) & ~255;
  }
};

class DxException : public std::exception {
private:
  std::string function_name_;
  std::string file_name_;
  std::string full_error_msg_{};
  int line_number_;
  HRESULT error_code_;

public:
  DxException(std::string function_name, std::string filename, int line_num,
              HRESULT err_code);
  const char *what() const override;
};
} // namespace Util
#endif