
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(poco LANGUAGES CXX)

set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE STREQUAL "debug")
    add_compile_definitions(-DDEVELOPMENT=1 -DDEBUG=1)
endif()


set(POCO_GUI_SOURCE_FILES
        poco_gui.mm
)

list(TRANSFORM
        POCO_GUI_SOURCE_FILES
        PREPEND
        "${CMAKE_CURRENT_SOURCE_DIR}/"
)

add_executable(poco_gui ${POCO_GUI_SOURCE_FILES})
target_include_directories(libpoco PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../inc")
target_link_libraries(poco_gui libpoco trdfile ffi_static hashmap)

INSTALL(TARGETS poco_gui DESTINATION ./)

