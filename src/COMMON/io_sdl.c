/* io_sdl.c */

#include <SDL/SDL.h>
#include "io_.h"
#include "io_sdl.h"

static SDL_Surface *s_surface;

int
set_vmode(void)
{
	s_surface = SDL_SetVideoMode(320, 200, 8, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (s_surface == NULL)
		return 0;

	vf.p = s_surface->pixels;

	return 1;
}

void
cset_colors(const UBYTE *src)
{
	SDL_Colour col[256];
	unsigned int count = ((const UWORD *)src)[0];
	int dst = 0;
	src += 2;

	for (; count > 0; count--) {
		int nskip = src[0];
		int ncopy = src[1];
		int c;

		if (ncopy == 0)
			ncopy = 256;

		src += 2;
		dst += nskip;

		for (c = dst; c < dst + ncopy; c++) {
			unsigned int r = src[3 * c + 0];
			unsigned int g = src[3 * c + 1];
			unsigned int b = src[3 * c + 2];

			col[c].r = (r << 2) | (r >> 4);
			col[c].g = (g << 2) | (g >> 4);
			col[c].b = (b << 2) | (b >> 4);
		}

		SDL_SetPalette(s_surface, SDL_LOGPAL | SDL_PHYSPAL, col, dst, ncopy);

		src += 3 * ncopy;
		dst += ncopy;
	}
}

void
jset_colors(int start, int length, UBYTE *cmap)
{
	SDL_Colour col[256];
	int c;

	for (c = 0; c < length; c++) {
		unsigned int r = cmap[3 * c + 0];
		unsigned int g = cmap[3 * c + 1];
		unsigned int b = cmap[3 * c + 2];

		col[c].r = (r << 2) | (r >> 4);
		col[c].g = (g << 2) | (g >> 4);
		col[c].b = (b << 2) | (b >> 4);
	}

	SDL_SetPalette(s_surface, SDL_LOGPAL | SDL_PHYSPAL, col, start, length);
}

void
flip_video(void)
{
	SDL_Flip(s_surface);
}

void
wait_vblank(void)
{
}

void
wait_novblank(void)
{
}

long
get80hz(void)
{
	unsigned int t = SDL_GetTicks() * 2;
	unsigned int ts = t - (t % 25);
	unsigned int te = ts + 25;

	SDL_Delay((te - t + 1) / 2);

	return te / 25;
}
