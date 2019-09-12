add_executable(cli "main.cpp")

set_target_properties(cli PROPERTIES OUTPUT_NAME "mi")

target_include_directories(cli PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(cli PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

target_link_libraries(cli PRIVATE base core assimp)

add_subdirectory(options)
