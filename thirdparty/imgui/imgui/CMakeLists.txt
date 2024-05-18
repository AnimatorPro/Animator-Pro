cmake_minimum_required(VERSION 3.26)

project(dear_imgui LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


set(
    SOURCES
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
)


if(APPLE)
    list(APPEND
        SOURCES
        backends/imgui_impl_metal.mm
        backends/imgui_impl_metal.h
        backends/imgui_impl_sdl2.cpp
        backends/imgui_impl_sdl2.h
    )
endif()


add_compile_options(-fobjc-arc)


add_library(
        imgui
        "${SOURCES}"
)

target_include_directories(imgui PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends"
)
