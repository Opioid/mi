add_library(core "") 

target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/include/>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../assimp/include/>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/assimp/include/>)

target_link_libraries(core PRIVATE base)

add_subdirectory(mesh)
