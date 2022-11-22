project(PJ_SDL LANGUAGES OBJC C)


set(PJ_SDL_SOURCES
		./pj_sdl.c
		./sdl_mac.m

		./pj_sdl.h
)



add_library(pj_sdl STATIC "${PJ_SDL_SOURCES}")
target_include_directories(pj_sdl PUBLIC
		INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)
