//
// Created by Charles Wardlaw on 2022-10-02.
//

#include <stdbool.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

#include "stdtypes.h"

#include "pj_sdl.h"


/*--------------------------------------------------------------*/
SDL_Surface* s_surface		  = NULL;
SDL_Surface* s_buffer		  = NULL;
SDL_Window* window			  = NULL;
SDL_Surface* s_window_surface = NULL;
SDL_Renderer* renderer		  = NULL;
SDL_Texture* render_target    = NULL;

/*--------------------------------------------------------------*/
int pj_sdl_get_video_size(LONG* width, LONG* height)
{
	if (!s_surface) {
		*width = -1;
		*height = -1;
		return 0;
	}

	*width = s_surface->w;
	*height = s_surface->h;
	return 1;
}

/*--------------------------------------------------------------*/
int pj_sdl_get_window_size(int* width, int* height)
{
	if (!window) {
		*width = -1;
		*height = -1;
		return 0;
	}

	SDL_GetWindowSize(window, width, height);
	return 1;
}

/*--------------------------------------------------------------*/
int pj_sdl_get_window_scale(float* x, float* y)
{
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

	float video_wf = video_w;
	float video_hf = video_h;
	float window_wf = window_w;
	float window_hf = window_h;

	*x = window_wf / video_wf;
	*y = window_hf / video_hf;

	return 1;
}

/*--------------------------------------------------------------*/
LONG pj_sdl_get_display_scale()
{
	return 4;
}

/*--------------------------------------------------------------*/
static SDL_FRect pj_sdl_rect_convert(const SDL_Rect* rect) {
	SDL_FRect result = { rect->x, rect->y, rect->w, rect->h };
	return result;
}

/*--------------------------------------------------------------*/
SDL_FRect pj_sdl_fit_surface(const SDL_Surface* source, const int target_w, const int target_h)
{
	SDL_Rect result_rect = {.x = 0, .y = 0, .w = source->w, .h = source->h};

	const double scale_x = ((double)target_w) / ((double)source->w);
	const double scale_y = ((double)target_h) / ((double)source->h);

	const bool can_scale_x = (source->h * scale_x) < target_h;

	if (can_scale_x) {
		result_rect.w = target_w;
		result_rect.h = source->h * scale_x;
		result_rect.y = (target_h - result_rect.h) / 2;
	} else {
		result_rect.h = target_h;
		result_rect.w = source->w * scale_y;
		result_rect.x = (target_w - result_rect.w) / 2;
	}

	return pj_sdl_rect_convert(&result_rect);
}

/*--------------------------------------------------------------*/
void pj_sdl_flip_window_surface()
{
//	/* SDL_BlitScaled doesn't work from 8 bit to screen,
//	 * so I'm copying to a second buffer first and then
//	 * doing my stretched blit. */
//	SDL_BlitSurface(s_surface, NULL, s_buffer, NULL);
//
//	/* With window scaling, we want to center the target rect. */
//	SDL_Rect target_rect = pj_sdl_fit_surface(s_buffer, s_window_surface);
//
//	/* Draw to the window surface, scaled */
//	//! FIXME: Only clear the parts that aren't drawn from the scaled blit
//	SDL_FillSurfaceRect(s_window_surface, NULL, 0x000000);
//	SDL_BlitSurface(s_buffer, &s_buffer->clip_rect, s_window_surface, &target_rect);
//	SDL_UpdateWindowSurface(window);
	//	SDL_RenderClear(renderer);
	//	SDL_RenderTexture(renderer, texture, NULL, NULL);
	//	SDL_RenderPresent(renderer);

	void* pixels = NULL;
	int pitch = 0;

	if (SDL_LockTexture(render_target, NULL, &pixels, &pitch) != 0) {
		SDL_Log("Could not lock texture: %s", SDL_GetError());
		SDL_DestroyTexture(render_target);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	/* SDL_BlitScaled doesn't work from 8 bit to screen,
	 * so I'm copying to a second buffer first and then
	 * doing my stretched blit. */
	SDL_BlitSurface(s_surface, NULL, s_buffer, NULL);

	// Copy pixel data from surface to texture
	memcpy(pixels, s_buffer->pixels, s_buffer->h * s_buffer->pitch);

	// Unlock the texture
	SDL_UnlockTexture(render_target);

	const SDL_FRect source_rect = pj_sdl_rect_convert(&s_surface->clip_rect);
	int window_w, window_h;
	pj_sdl_get_window_size(&window_w, &window_h);
	const SDL_FRect target_rect = pj_sdl_fit_surface(s_surface, window_w, window_h);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, render_target, &source_rect, &target_rect);
	SDL_RenderPresent(renderer);
}