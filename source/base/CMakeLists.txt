add_library(base "") 

target_include_directories(base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/include/>)

add_subdirectory(chrono)
add_subdirectory(flags)
add_subdirectory(math)
add_subdirectory(memory)
