#include <SDL.h>
#include <iostream>

/*
 * SDL2_PALETTE_TEST.CC
 *
 * SDL 1 had easier palettized drawing, as it was a 90's-era API.  Palettes are a bit
 * more involved with SDL2.  This test is for creating palettized drawing.
 */

int main()
{
	const int scale = 5;
	const int w		= 320;
	const int h		= 200;
	const int win_w = w * scale;
	const int win_h = h * scale;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window =
	  SDL_CreateWindow("SDL_Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_w, win_h, 0);
	SDL_Surface* screen = SDL_GetWindowSurface(window);

	// don't kill my beautiful pixels
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	SDL_Surface* buffer	 = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);

	SDL_Color colors[2] = { { 0, 0, 255, 255 }, { 255, 0, 0, 255 } };
	SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event) > 0) {
			switch (event.type) {
				case SDL_QUIT:
					return EXIT_SUCCESS;
			}
		}
		uint8_t* offscreen = (uint8_t*)surface->pixels;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w / 2; j++) {
				offscreen[j * 2 + 0] = 0;
				offscreen[j * 2 + 1] = 1;
			}
			offscreen += surface->pitch;
		}

		SDL_Rect source_rect = { 0, 0, w, h };
		SDL_Rect target_rect = { 0, 0, win_w, win_h };

		/* SDL_BlitScaled doesn't work from 8 bit to screen,
		 * so I'm copying to a second buffer first and then
		 * doing my stretched blit. */
		SDL_BlitSurface(surface, NULL, buffer, NULL);
		SDL_BlitScaled(buffer, &source_rect, screen, &target_rect);

		SDL_UpdateWindowSurface(window);
		SDL_Delay(100);
	}
}
