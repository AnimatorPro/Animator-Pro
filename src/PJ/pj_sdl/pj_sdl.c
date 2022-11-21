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


#ifdef _WIN32
	const char* SEP = "\\";
#else
	const char* SEP = "/";
#endif


/*--------------------------------------------------------------*/
int pj_sdl_get_video_size(LONG* width, LONG* height)
{
	if (!s_surface) {
		*width	= -1;
		*height = -1;
		return 0;
	}

	*width	= s_surface->w;
	*height = s_surface->h;
	return 1;
}


/*--------------------------------------------------------------*/
int pj_sdl_get_window_size(int* width, int* height) {
	if (!window) {
		*width	= -1;
		*height = -1;
		return 0;
	}

	SDL_GetWindowSize(window, width, height);
	return 1;
}

/*--------------------------------------------------------------*/
int pj_sdl_get_window_scale(float* x, float* y) {
	LONG video_w, video_h;
	int window_w, window_h;

	*x = 1.0f;
	*y = 1.0f;

	if (!pj_sdl_get_video_size(&video_w, &video_h)) {
		fprintf(stderr, "Unable to get video size!\n");
		return 0;
	}

	if (!pj_sdl_get_window_size(&window_w, &window_h)) {
		fprintf(stderr, "Unable to get window size!\n");
		return 0;
	}

	float video_wf  = video_w;
	float video_hf  = video_h;
	float window_wf = window_w;
	float window_hf = window_h;

	*x = window_wf / video_wf;
	*y = window_hf / video_hf;

	return 1;
}


/*--------------------------------------------------------------*/
LONG pj_sdl_get_display_scale() {
	return 4;
}

