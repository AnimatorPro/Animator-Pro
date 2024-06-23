
project(imgui_text_editor)

set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 17)

add_library(imgui_text_editor STATIC TextEditor.cpp TextEditor.h)
target_link_libraries(imgui_text_editor PUBLIC imgui)
target_include_directories(imgui_text_editor PUBLIC
        INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)
