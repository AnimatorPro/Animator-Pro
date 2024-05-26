/* idr_sdl.c */
#include "idr_sdl.h"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "aaconfig.h"
#include "errcodes.h"
#include "idriver.h"
#include "jimk.h"
#include "pj_sdl.h"

static Errcode sdl_idr_detect(Idriver* idr);
static Errcode sdl_idr_inquire(Idriver* idr);
static Errcode sdl_idr_input(Idriver* idr);
static Errcode sdl_idr_setclip(Idriver* idr, short channel, long clipmax);

static Idr_library sdl_idr_library = {
	sdl_idr_detect,
	sdl_idr_inquire,
	sdl_idr_input,
	sdl_idr_setclip,
};

/*--------------------------------------------------------------*/

/* Function: pj_clock_1000
 *
 *  Return the number of elapsed milliseconds since the program was started.
 */

uint32_t pj_clock_1000(void)
{
	return SDL_GetTicks();
}

static int16_t sdl_key_event_to_ascii(const SDL_Event* ev)
{
	static const int16_t fnkey[12] = {FKEY1, FKEY2, FKEY3, FKEY4,  FKEY5,  FKEY6,
									  FKEY7, FKEY8, FKEY9, FKEY10, FKEY11, FKEY12};

	assert(ev->type == SDL_EVENT_KEY_DOWN);

	if (SDLK_F1 <= ev->key.keysym.sym && ev->key.keysym.sym <= SDLK_F12) {
		return fnkey[ev->key.keysym.sym - SDLK_F1];
	}

	switch (ev->key.keysym.sym) {
		case SDLK_UP:
			return UARROW;
		case SDLK_DOWN:
			return DARROW;
		case SDLK_LEFT:
			return LARROW;
		case SDLK_RIGHT:
			return RARROW;

		case SDLK_INSERT:
			return INSERTKEY;
		case SDLK_DELETE:
			return DELKEY;
		case SDLK_HOME:
			return HOMEKEY;
		case SDLK_END:
			return ENDKEY;
		case SDLK_PAGEUP:
			return PAGEUP;
		case SDLK_PAGEDOWN:
			return PAGEDN;

		default:
			break;
	}

	return (int16_t)ev->key.keysym.sym;
}

int16_t dos_wait_key(void)
{
	for (;;) {
		SDL_Event ev;

		if (!SDL_WaitEvent(&ev)) {
			return 0;
		}

		if (ev.type == SDL_EVENT_KEY_DOWN) {
			return sdl_key_event_to_ascii(&ev);
		}
	}
}

int pj_key_is(void)
{
	return 0;
}

int pj_key_in(void)
{
	return 0;
}

int dos_key_shift(void)
{
	return 0;
}

/*--------------------------------------------------------------*/
/* sdl_idr_library.                                             */
/*--------------------------------------------------------------*/

static Errcode sdl_idr_detect(Idriver* idr)
{
	(void)idr;
	return Success;
}

static Errcode sdl_idr_inquire(Idriver* idr)
{
	idr->button_count = 2;
	idr->channel_count = 3;

	return Success;
}

static Errcode sdl_idr_input(Idriver* idr)
{
	SDL_Event ev;
	unsigned int mb;
	float winscale_x, winscale_y;

	SDL_PumpEvents();
	idr->key_code = 0;

	int window_w, window_h;
	pj_sdl_get_window_size(&window_w, &window_h);
	SDL_FRect rect = pj_sdl_fit_surface(s_surface, window_w, window_h);
	pj_sdl_get_window_scale(&winscale_x, &winscale_y);

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_EVENT_KEY_DOWN:
				idr->key_code = sdl_key_event_to_ascii(&ev);

			case SDL_EVENT_MOUSE_MOTION:
				idr->pos[0] = ev.motion.x - rect.x;
				idr->pos[1] = ev.motion.y - rect.y;
				/*
				 * Scale the mouse motion by the video buffer size.
				 * Remember that the aspect scaling of the buffer
				 * changes depending on whether the x or y axis
				 * matches between buffer and window, so scale mouse
				 * coords accordingly.
				 */
				if (rect.x == 0) {
					idr->pos[0] /= winscale_x;
					idr->pos[1] /= winscale_x;
				} else {
					idr->pos[0] /= winscale_y;
					idr->pos[1] /= winscale_y;
				}
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				mb = (ev.button.button == SDL_BUTTON_LEFT)    ? 1
					 : (ev.button.button == SDL_BUTTON_RIGHT) ? 2
															  : 0;
				if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
					idr->buttons |= mb;
				}
				else if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP) {
					idr->buttons &= ~mb;
				}
				break;

			//!TODO: figure out a better event loop for SDL where
			//       not everything is done inside IDR?

			case SDL_EVENT_WINDOW_RESIZED:
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				// Update the target surface so drawing works!
				//! TODO: Adjust screen resolution?
				break;

			default:
				break;
		}
	}

	return Success;
}

static Errcode sdl_idr_setclip(Idriver* idr, short channel, long clipmax)
{
	if ((uint16_t)channel > idr->channel_count) {
		return Err_bad_input;
	}

	if ((uint32_t)clipmax > (uint32_t)idr->max[channel]) {
		idr->clipmax[channel] = idr->max[channel];
	}
	else {
		idr->clipmax[channel] = clipmax;
	}

	return Success;
}

/*--------------------------------------------------------------*/

static Errcode sdl_idr_open(Idriver* idr)
{
	sdl_idr_inquire(idr);
''
	if (idr->channel_count > 0) {
		int screen_width, screen_height;
		pj_sdl_get_video_size(&screen_width, &screen_height);

		const int naxes = idr->channel_count;
		assert(s_surface != NULL);
		assert(naxes >= 2);

		idr->pos = malloc(naxes * sizeof(idr->pos[0]));
		assert(idr->pos != NULL);
		idr->pos[0] = 0;
		idr->pos[1] = 0;
		idr->pos[2] = 0xFF;

		idr->min = malloc(naxes * sizeof(idr->min[0]));
		assert(idr->min != NULL);
		idr->min[0] = 0;
		idr->min[1] = 0;
		idr->min[2] = 0;

		idr->max = malloc(naxes * sizeof(idr->max[0]));
		assert(idr->max != NULL);
		idr->max[0] = screen_width - 1;
		idr->max[1] = screen_height - 1;
		idr->max[2] = 0xFF;

		idr->clipmax = malloc(naxes * sizeof(idr->clipmax[0]));
		assert(idr->clipmax != NULL);
		idr->clipmax[0] = idr->max[0];
		idr->clipmax[1] = idr->max[1];
		idr->clipmax[2] = idr->max[2];

		idr->aspect = NULL;

		idr->flags = malloc(naxes * sizeof(idr->flags[0]));
		assert(idr->flags != NULL);
		idr->flags[0] = 0;
		idr->flags[1] = 0;
		idr->flags[2] = PRESSURE;
	}

	return Success;
}

static Errcode sdl_idr_close(Idriver* idr)
{
	free(idr->pos);
	idr->pos = NULL;

	free(idr->min);
	idr->min = NULL;

	free(idr->max);
	idr->max = NULL;

	free(idr->clipmax);
	idr->clipmax = NULL;

	free(idr->aspect);
	idr->aspect = NULL;

	free(idr->flags);
	idr->flags = NULL;

	return Success;
}

Errcode init_sdl_idriver(Idriver* idr)
{
	idr->hdr.init = sdl_idr_open;
	idr->hdr.cleanup = sdl_idr_close;
	idr->lib = &sdl_idr_library;
	idr->options = NULL;
	idr->does_keys = true;
	return Success;
}

Errcode init_input(void)
{
	//	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	//	SDL_ShowCursor(SDL_DISABLE);

	init_idriver(NULL, vconfg.idr_modes, 0);

	reset_input();
	enable_textboxes();

	return Success;
}
