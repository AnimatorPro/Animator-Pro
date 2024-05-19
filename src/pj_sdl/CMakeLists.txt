
project(PJ_SDL)

set(PJ_SDL_SOURCES
		./pj_sdl.c
		./pj_sdl.h
)

if (WIN32)
	set(PJ_SDL_PLATFORM_SOURCES
			./sdl_win.c
	)
endif(WIN32)

if (APPLE)
	set(PJ_SDL_PLATFORM_SOURCES
			./sdl_mac.m
	)
endif(APPLE)

add_library(pj_sdl STATIC ${PJ_SDL_SOURCES} ${PJ_SDL_PLATFORM_SOURCES})
target_include_directories(pj_sdl PUBLIC
		INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)
