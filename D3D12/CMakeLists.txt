#D3D12

project(MyD3D12)

#windows系统动态库生成lib文件的命令
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(${PROJECT_NAME} SHARED)

add_subdirectory(include)


file(GLOB_RECURSE SOURCES "*.cpp")

target_sources( ${PROJECT_NAME}
	PRIVATE
		${SOURCES}
)


target_link_libraries(${PROJECT_NAME} d3d12.lib dxgi.lib d3dcompiler.lib)

get_filename_component(PARENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

install(FILES ${PARENT_FOLDER_ABSOLUTE}/build/D3D12/${PROJECT_NAME}.dll
DESTINATION ${PARENT_FOLDER_ABSOLUTE}/build/test)