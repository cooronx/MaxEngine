# CMakeList.txt: test 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

project ("test")
# 配置win32程序环境
set(CMAKE_WIN32_EXECUTABLE ON)

get_filename_component(PARENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
include_directories(
	${PARENT_FOLDER_ABSOLUTE}/imgui
	${PARENT_FOLDER_ABSOLUTE}/imgui/backends
)

file(GLOB COMMON_IMGUI_SRC
${PARENT_FOLDER_ABSOLUTE}/imgui/*.h
${PARENT_FOLDER_ABSOLUTE}/imgui/*.cpp
)

file(GLOB SP_IMGUI_SRC
${PARENT_FOLDER_ABSOLUTE}/imgui/backends/imgui_impl_win32.*
${PARENT_FOLDER_ABSOLUTE}/imgui/backends/imgui_impl_dx12.*
)

add_executable (test "test.cpp" "MyD3D.h" ${SP_IMGUI_SRC} ${COMMON_IMGUI_SRC})

target_link_libraries(test MAX_COMMON MyD3D12)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
