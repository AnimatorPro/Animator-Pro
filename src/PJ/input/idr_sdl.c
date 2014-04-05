/* idr_sdl.c */

#include <assert.h>
#include <SDL/SDL.h>
#include "jimk.h"
#include "errcodes.h"
#include "idr_sdl.h"
#include "idriver.h"

static Errcode sdl_idr_detect(Idriver *idr);
static Errcode sdl_idr_inquire(Idriver *idr);
static Errcode sdl_idr_input(Idriver *idr);
static Errcode sdl_idr_setclip(Idriver *idr, short channel, long clipmax);

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
ULONG
pj_clock_1000(void)
{
	return SDL_GetTicks();
}

static SHORT
sdl_key_event_to_ascii(const SDL_Event *ev)
{
	static const SHORT fnkey[12] = {
		FKEY1,  FKEY2, FKEY3, FKEY4, FKEY5,
		FKEY6,  FKEY7, FKEY8, FKEY9, FKEY10,
		FKEY11, FKEY12
	};

	assert(ev->type == SDL_KEYDOWN);

	if (SDLK_F1 <= ev->key.keysym.sym && ev->key.keysym.sym <= SDLK_F12)
		return fnkey[ev->key.keysym.sym - SDLK_F1];

	switch (ev->key.keysym.sym) {
		case SDLK_UP:       return UARROW;
		case SDLK_DOWN:     return DARROW;
		case SDLK_LEFT:     return LARROW;
		case SDLK_RIGHT:    return RARROW;

		case SDLK_INSERT:   return INSERTKEY;
		case SDLK_DELETE:   return DELKEY;
		case SDLK_HOME:     return HOMEKEY;
		case SDLK_END:      return ENDKEY;
		case SDLK_PAGEUP:   return PAGEUP;
		case SDLK_PAGEDOWN: return PAGEDN;

		default: break;
	}

	return ev->key.keysym.unicode;
}

SHORT
dos_wait_key(void)
{
	for (;;) {
		SDL_Event ev;

		if (!SDL_WaitEvent(&ev))
			return 0;

		if (ev.type == SDL_KEYDOWN)
			return sdl_key_event_to_ascii(&ev);
	}
}

int
pj_key_is(void)
{
	return 0;
}

int
pj_key_in(void)
{
	return 0;
}

int
dos_key_shift(void)
{
	return 0;
}

/*--------------------------------------------------------------*/
/* sdl_idr_library.                                             */
/*--------------------------------------------------------------*/

static Errcode
sdl_idr_detect(Idriver *idr)
{
	(void)idr;
	return Success;
}

static Errcode
sdl_idr_inquire(Idriver *idr)
{
	idr->button_count = 2;
	idr->channel_count = 2;

	return Success;
}

static Errcode
sdl_idr_input(Idriver *idr)
{
	SDL_Event ev;
	unsigned int mb;

	SDL_PumpEvents();
	idr->key_code = 0;

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				idr->key_code = sdl_key_event_to_ascii(&ev);
				return Success;

			case SDL_MOUSEMOTION:
				idr->pos[0] = ev.motion.x;
				idr->pos[1] = ev.motion.y;
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mb = (ev.button.button == SDL_BUTTON_LEFT) ? 1
					: (ev.button.button == SDL_BUTTON_RIGHT) ? 2
					: 0;

				if (ev.type == SDL_MOUSEBUTTONDOWN)
					idr->buttons |= mb;
				else if (ev.type == SDL_MOUSEBUTTONUP)
					idr->buttons &= ~mb;
		}
	}

	return Success;
}

static Errcode
sdl_idr_setclip(Idriver *idr, short channel, long clipmax)
{
	if ((USHORT)channel > idr->channel_count)
		return Err_bad_input;

	if ((ULONG)clipmax > (ULONG)idr->max[channel])
		idr->clipmax[channel] = idr->max[channel];
	else
		idr->clipmax[channel] = clipmax;

	return Success;
}

/*--------------------------------------------------------------*/

static Errcode
sdl_idr_open(Idriver *idr)
{
	sdl_idr_inquire(idr);

	if (idr->channel_count > 0) {
		const SDL_Surface *surface = SDL_GetVideoSurface();
		const int naxes = idr->channel_count;
		assert(surface != NULL);
		assert(naxes == 2);

		idr->pos = malloc(naxes * sizeof(idr->pos[0]));
		assert(idr->pos != NULL);
		idr->pos[0] = 0;
		idr->pos[1] = 0;

		idr->min = malloc(naxes * sizeof(idr->min[0]));
		assert(idr->min != NULL);
		idr->min[0] = 0;
		idr->min[1] = 0;

		idr->max = malloc(naxes * sizeof(idr->max[0]));
		assert(idr->max != NULL);
		idr->max[0] = surface->w - 1;
		idr->max[1] = surface->h - 1;

		idr->clipmax = malloc(naxes * sizeof(idr->clipmax[0]));
		assert(idr->clipmax != NULL);
		idr->clipmax[0] = idr->max[0];
		idr->clipmax[1] = idr->max[1];

		idr->aspect = NULL;

		idr->flags = malloc(naxes * sizeof(idr->flags[0]));
		assert(idr->flags != NULL);
		idr->flags[0] = RELATIVE;
		idr->flags[1] = RELATIVE;
	}

	return Success;
}

static Errcode
sdl_idr_close(Idriver *idr)
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

Errcode
init_sdl_idriver(Idriver *idr)
{
	idr->hdr.init = sdl_idr_open;
	idr->hdr.cleanup = sdl_idr_close;
	idr->lib = &sdl_idr_library;
	idr->options = NULL;
	idr->does_keys = TRUE;
	return Success;
}
