
cmake_minimum_required(VERSION 3.16)
project(TestApp OBJC CXX)

set(SOURCES
		sdl_test.mm
)

add_executable(${PROJECT_NAME} "${SOURCES}")
target_link_libraries(${PROJECT_NAME} SDL2-static SDL2main m)

set_target_properties(
		${PROJECT_NAME}
	PROPERTIES
		MACOSX_BUNDLE YES
		XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.vpaint.test-app"
)

## pre-build required or signing will fail
add_custom_command(
	TARGET ${PROJECT_NAME}
	PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources"
)

