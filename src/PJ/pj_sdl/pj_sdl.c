//
// Created by Charles Wardlaw on 2022-10-02.
//

#include <SDL.h>

#include "stdtypes.h"

#include "pj_sdl.h"

/*--------------------------------------------------------------*/
SDL_Surface* s_surface = NULL;
SDL_Window* window	   = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture   = NULL;

/*--------------------------------------------------------------*/
int pj_sdl_get_video_size(LONG* width, LONG* height)
{
	if (!s_surface) {
		*width	= -1;
		*height = -1;
		return -1;
	}

	int tex_width, tex_height;
	int result = SDL_QueryTexture(texture, NULL, NULL, &tex_width, &tex_height);

	if (result == 0) {
		*width = tex_width;
		*height = tex_height;
	}

	return result;
}
