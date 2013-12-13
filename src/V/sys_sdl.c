/* sys_sdl.c */

#include <assert.h>
#include <string.h>
#include <SDL/SDL.h>
#include "jimk.h"
#include "fs.h"
#include "io_.h"
#include "jfile.h"
#include "peekpok_.h"

extern Vscreen menu_vf;

static void init_config(void);

#include "memory.str"
outta_memory()
{
continu_line(memory_100 /* "Out of Memory" */);
}

/*--------------------------------------------------------------*/
/* Init                                                         */
/*--------------------------------------------------------------*/

extern char init_drawer[71];

int
init_sys(void)
{
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	if (!set_vmode()) {
		return 0;
	}

	SDL_WM_SetCaption("Animator", NULL);

	menu_vf.p = vf.p;
	uf.p = malloc(64016L);
	assert(uf.p);

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(SDL_ENABLE);
	SDL_ShowCursor(SDL_DISABLE);

	init_config();
	copy_structure(&default_vs, &vs, sizeof(vs));
	copy_cmap(init_cmap, sys_cmap);
	init_seq();
	make_current_drawer(vs.drawer, sizeof(vs.drawer));
	strcpy(init_drawer, vs.drawer);

	return 1;
}

void
uninit_sys(void)
{
	SDL_Quit();
}

/*--------------------------------------------------------------*/
/* Config                                                       */
/*--------------------------------------------------------------*/

char tflxname[] = "AATEMP.FLX";	/* main animation temp file */
char new_tflx_name[] = "AATEMP2.FLX";
									/* Animation temp file for to all ops */
char tmacro_name[] = "AATEMP.REC";	/* The current input macro */
char alt_name[] = "AATEMP.PIC";	/* Swap file for alt screen */
char screen_name[] = "AATEMP2.PIC";/* Swap file for current frame */
char cel_name[] = "AATEMP.CEL";	/* Swap file for cel */
char text_name[] = "AATEMP.TXT";	/* Swap file for text buffer */
char cclip_name[] = "AATEMP.CCL";	/* Current color clip */
char mask_name[] = "AATEMP.MSK";	/* Swap file for mask */
char poly_name[] = "AATEMP.PLY";	/* Latest vector shape */
char poly1_name[] = "AATEMP1.PLY";	/* Start shape of tween */
char poly2_name[] = "AATEMP2.PLY";	/* End shape of tween */
char ppoly_name[] = "AATEMPP.PLY";	/* Optics path */
char bscreen_name[] = "AATEMP3.PIC";	/* Back frame buffer. Frame c. 4 back */
char another_name[] = "AATEMP.XXX"; /* swap file for wierd misc uses */
char optics_name[] = "AATEMP.OPT";	/* Stack of optics moves. */
char default_name[] = "DEFAULT.FLX";	/* User defined startup-state */
char conf_name[] = "aa.cfg";			/* Small stuff unlikely to change*/

struct config vconfg;					/* Ram image of v.cfg */
struct config_ext vconfg_ext;
static struct config ivconfg = {VCFGMAGIC,2,0,"", 0, 0};

extern char goodconf;
extern char init_drawer[];

void
rewrite_config(void)
{
	FILE *f;
	char odrawer[80];

	strcpy(odrawer, vs.drawer);
	change_dir(init_drawer);
	if ((f = jcreate(conf_name)) != 0) {
		jwrite(f, &vconfg, sizeof(vconfg) );
		jwrite(f, &vconfg_ext, sizeof(vconfg_ext) );
		jclose(f);
	}
	change_dir(odrawer);
}

static void
save_settings(void)
{
	extern FILE *tflx;

	flush_tempflx();	/* make sure latest configuration stuff is written */
	jclose(tflx);
	change_dir(init_drawer);
	jcopyfile(tflxname, default_name);
	change_dir(vs.drawer);
	tflx = jopen(tflxname, 2);
}

void
new_config(void)
{
	save_settings();
}

static void
init_config(void)
{
	FILE *jcf;
	int dev = 0;

	if ((jcf = jopen(conf_name, 0)) != 0) {
		if (jread(jcf, &vconfg, sizeof(vconfg)) == sizeof(vconfg) ) {
			if (vconfg.cmagic == VCFGMAGIC) {
				/* deal with variable sized configuration extention */
				zero_structure(&vconfg_ext, sizeof(vconfg_ext));
				if (vconfg.extention_size > 0) {
					if (vconfg.extention_size > sizeof(vconfg_ext))
						vconfg.extention_size = sizeof(vconfg_ext);
					if (jread(jcf, &vconfg_ext, vconfg.extention_size))
						goodconf = 1;
				}
				else {
					goodconf = 1;
				}
				vconfg.extention_size = sizeof(vconfg_ext);
			}
		}
		jclose(jcf);
	}

	if (!goodconf) {
		copy_structure(&ivconfg, &vconfg, sizeof(vconfg));
		vconfg.scratch_drive = dev;
		zero_structure(&vconfg_ext, sizeof(vconfg_ext));
	}
}

/*--------------------------------------------------------------*/
/* Input                                                        */
/*--------------------------------------------------------------*/

static WORD mscale = 1;

extern char reuse;
extern char usemacro;
extern char zoomcursor;
extern WORD lmouse_x, lmouse_y;

extern void get_macro(void);
extern void next_histrs(void);

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

static void
c_input(int block)
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

	if (block && !usemacro) {
		got_event = SDL_WaitEvent(&ev);
	}
	else {
		got_event = SDL_PollEvent(&ev);
	}

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
	if (usemacro)
		get_macro();

	grid_x = uzx;
	grid_y = uzy;
	if (vs.zoom_mode) {
		grid_x = vs.zoomx + uzx/vs.zoomscale;
		grid_y = vs.zoomy + uzy/vs.zoomscale;
	}

	get_gridxy();
	next_histrs();
	mouse_moved = 0;
	if (!(uzx == lastx && uzy == lasty)) {
		if (!zoomcursor
				|| uzy/vs.zoomscale != lasty/vs.zoomscale
				|| uzx/vs.zoomscale != lastx/vs.zoomscale) {
			lmouse_x = lastx;
			lmouse_y = lasty;
			mouse_moved = 1;
			if (mouse_on) {
				ucursor();
				scursor();
				ccursor();
			}
			flip_video();
		}
	}

	if (mouse_button != omouse_button) {
		lmouse_x = uzx;
		lmouse_y = uzy;
		mouse_moved = 1;
	}
	put_macro(clickonly);
}

void
c_poll_input(void)
{
	c_input(0);
}

void
c_wait_input(void)
{
	c_input(1);
}
