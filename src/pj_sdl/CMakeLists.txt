
project(PJ_SDL)

set(PJ_SDL_SOURCES
		./pj_sdl.c
		./pj_sdl.h
		./fs_sdl.c
)

if (WIN32)
	set(PJ_SDL_PLATFORM_SOURCES
		## not yet needed? Thank you, SDL_Filesystem
#		./sdl_win.cc
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
