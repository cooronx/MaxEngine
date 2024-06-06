file(GLOB_RECURSE SOURCES "*.cpp")

target_sources(${PROJECT_NAME}
    PRIVATE
    ${SOURCES}
)