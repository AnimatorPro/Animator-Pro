/* sys_sdl.c */

#include <SDL/SDL.h>
#include "init.str"
#include "io_.h"
#include "io_sdl.h"
#include "sys.h"

/*--------------------------------------------------------------*/
/* Init                                                         */
/*--------------------------------------------------------------*/

int
init_system(void)
{
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	if (!set_vmode()) {
		puts(init_102);
		return 0;
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(SDL_ENABLE);
	SDL_ShowCursor(SDL_DISABLE);

	return 1;
}

void
cleanup(void)
{
	SDL_Quit();
}

/*--------------------------------------------------------------*/
/* Input                                                        */
/*--------------------------------------------------------------*/

static WORD mscale = 1;
static WORD umouse_x, umouse_y; /* unscaled mousexy */

extern char reuse;

static int
translate_sdl_keysym(SDL_keysym *key)
{
	if (key->sym == SDLK_PAGEUP) return PAGEUP;
	if (key->sym == SDLK_PAGEDOWN) return PAGEDN;
	if (key->sym == SDLK_END) return ENDKEY;
	if (key->sym == SDLK_HOME) return HOMEKEY;
	if (key->sym == SDLK_DELETE) return DELKEY;
	if (key->sym == SDLK_LEFT) return LARROW;
	if (key->sym == SDLK_RIGHT) return RARROW;
	if (key->sym == SDLK_UP) return UARROW;
	if (key->sym == SDLK_DOWN) return DARROW;

	if ((key->unicode & 0xFF80) == 0)
		return key->unicode;

	return 0;
}

void
c_input(void)
{
	SDL_Event ev;
	int got_event;

	if (reuse) {
		reuse = 0;
		return;
	}

	lastx = uzx;
	lasty = uzy;
	omouse_button = mouse_button;
	key_hit = 0;

	got_event = SDL_PollEvent(&ev);

	if (got_event) {
		int key = 0;
		int mb;

		switch (ev.type) {
			case SDL_KEYDOWN:
				key = translate_sdl_keysym(&ev.key.keysym);
				if (key != 0) {
					key_hit = 1;
					key_in = key;
				}
				break;

			case SDL_MOUSEMOTION:
				umouse_x = ev.motion.x;
				umouse_y = ev.motion.y;
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				umouse_x = ev.button.x;
				umouse_y = ev.button.y;

				mb = (ev.button.button == SDL_BUTTON_LEFT) ? 0x1
					: (ev.button.button == SDL_BUTTON_RIGHT) ? 0x2
					: 0;

				if (ev.type == SDL_MOUSEBUTTONDOWN) {
					mouse_button |= mb;
				}
				else if (ev.type == SDL_MOUSEBUTTONUP) {
					mouse_button &= ~mb;
				}
				break;

			default:
				break;
		}
	}

	uzx = (umouse_x/mscale);
	uzy = (umouse_y/mscale);
	mouse_moved = 0;

	if (mouse_on && !(uzx == lastx && uzy == lasty)) {
		mouse_moved = 1;
		ucursor();
		scursor();
		ccursor();
		flip_video();
	}

	if (mouse_button != omouse_button) {
		mouse_moved = 1;
	}
}
