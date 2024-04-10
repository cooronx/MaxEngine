#include "D3D12Util.h"

using namespace Util;

DxException::DxException(std::string function_name, std::string filename, int line_num, HRESULT err_code)
	:
	function_name_{function_name_},
	file_name_{filename},
	line_number_{line_num},
	error_code_{err_code}
{
	_com_error err{ error_code_ };
	std::string msg = err.ErrorMessage();
	full_error_msg_ = function_name_ + " failed in " + file_name_ + " line " + std::to_string(line_number_) + "  " + msg;
}

const char* DxException::what() const
{
	return full_error_msg_.data();
}
