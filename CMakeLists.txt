cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(animator-pro LANGUAGES C CXX)

## thirdparty dependencies
add_subdirectory(thirdparty/sdl2)
include_directories(thirdparty/sdl2/include)

## main
add_subdirectory(src/PJ)


## tests
add_subdirectory(tests)