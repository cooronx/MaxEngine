file( GLOB_RECURSE SOURCES "*.h")


target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})