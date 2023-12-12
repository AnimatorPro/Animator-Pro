
#project(imgui_text_editor LANGUAGES OBJC C)


add_library(imgui_text_editor STATIC TextEditor.cpp TextEditor.h)
target_include_directories(imgui_text_editor PUBLIC
        INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)
