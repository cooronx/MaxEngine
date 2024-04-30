#D3D12

project(MAX_COMMON)

#windowsϵͳ��̬������lib�ļ�������
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Eigen3 CONFIG REQUIRED)
add_library(${PROJECT_NAME} SHARED)
target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
add_subdirectory(include)

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


file(GLOB_RECURSE SOURCES "*.cpp")

target_sources( ${PROJECT_NAME}
	PRIVATE
		${SOURCES}
		${COMMON_IMGUI_SRC}
		${SP_IMGUI_SRC}
)


get_filename_component(PARENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

install(FILES ${PARENT_FOLDER_ABSOLUTE}/build/Common/${PROJECT_NAME}.dll
DESTINATION ${PARENT_FOLDER_ABSOLUTE}/build/test)