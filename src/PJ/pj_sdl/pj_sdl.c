//
// Created by Charles Wardlaw on 2022-10-02.
//

#include <SDL.h>

#include "stdtypes.h"

#include "pj_sdl.h"

/*--------------------------------------------------------------*/
SDL_Surface* s_surface		  = NULL;
SDL_Surface* s_buffer		  = NULL;
SDL_Window* window			  = NULL;
SDL_Surface* s_window_surface = NULL;
SDL_Renderer* renderer		  = NULL;

/*--------------------------------------------------------------*/
int pj_sdl_get_video_size(LONG* width, LONG* height)
{
	if (!s_surface) {
		*width	= -1;
		*height = -1;
		return -1;
	}

	*width	= s_surface->w;
	*height = s_surface->h;
	return 0;
}


/*--------------------------------------------------------------*/
LONG pj_sdl_get_display_scale() {
	return 4;
}

