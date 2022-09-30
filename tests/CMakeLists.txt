
cmake_minimum_required(VERSION 3.16)
project(pj_tests C)

set(SOURCES
		sdl_test.cc
)

add_executable(sdl_test "${SOURCES}")
target_link_libraries(sdl_test SDL2-static SDL2main m)

