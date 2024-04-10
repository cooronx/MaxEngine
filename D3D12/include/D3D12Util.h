#ifndef D3D12UTIL_H
#define D3D12UTIL_H

#include <exception>
#include <string>
#include <WTypesbase.h>
#include "comdef.h"

namespace Util {

class DxException : public std::exception {
private:
	std::string function_name_;
	std::string file_name_;
	std::string full_error_msg_{};
	int line_number_;
	HRESULT error_code_;
public:
	DxException(std::string function_name,std::string filename,int line_num,HRESULT err_code);
	const char* what() const override;
};
}  // namespace Util

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)											\
{																	\
	HRESULT hr__ = (x);												\
	std::string fn__ = __FILE__;									\
	if(hr__ < 0){													\
		throw DxException{#x,fn__,__LINE__,hr__};					\
	}																\
}																			
#endif


#endif