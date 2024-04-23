# CMakeList.txt: test 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

project ("test")
# 配置win32程序环境
set(CMAKE_WIN32_EXECUTABLE ON)


add_executable (test "test.cpp" "MyD3D.h")

target_link_libraries(test MyD3D12)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
