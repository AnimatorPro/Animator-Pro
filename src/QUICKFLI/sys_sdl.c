/* sys_sdl.c */

#include <SDL/SDL.h>
#include "io_.h"
#include "io_sdl.h"
#include "sys.h"

static Uint8 *s_key;

int
init_system(void)
{
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	if (!set_vmode()) {
		puts("Couldn't get a 320x200 256 color VGA screen");
		return 0;
	}

	s_key = SDL_GetKeyState(NULL);

	return 1;
}

void
cleanup(void)
{
	SDL_Quit();
}

unsigned int
strobe_keys(void)
{
	SDL_Event event;

	SDL_PollEvent(&event);

	if (s_key[SDLK_ESCAPE]) return ESC;
	if (s_key[SDLK_SPACE]) return SPACE;

	return 0;
}
