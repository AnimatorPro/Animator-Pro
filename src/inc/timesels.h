#ifndef TIMESELS_H
#define TIMESELS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct button;

typedef struct minitime_data {
	void (*first_frame)(void *data);
	void (*prev_frame)(void *data);
	void (*feel_ix)(void *data);
	void (*next_frame)(void *data);
	void (*play_it)(void *data);
	void (*last_frame)(void *data);
	void (*opt_all)(void *data);
	void (*opt_tsl_first)(void *data); /* optme for time slider tofirst */
	SHORT (*get_frameix)(void *data);
	SHORT (*get_framecount)(void *data);
	void (*clear_overlays)(void *data); /* clear any overlays on fli area */
	void (*draw_overlays)(void *data); /* restore any overlays on fli area */
	void (*seek_frame)(SHORT ix, void *data);
	SHORT olay_stack;					 /* so recursion only clears once */
	void *data;
} Minitime_data;

extern Minitime_data flxtime_data;

#ifdef MUPARTS_INTERNALS

#define IXSEL_ID 0xAAAA

void mtd_ix_dofeel(struct button *b, void (*feeler)(void *));
void mt_feel_prev(Button *b);
void mt_feel_next(Button *b);
void mt_feel_first(Button *b);
void mt_feel_play(Button *b);
void mt_feel_last(Button *b);

#endif /* MUPARTS_INTERNALS */

void update_time_sel(struct button *b);
void mini_playit(Minitime_data *mtd);
void mini_prev_frame(Minitime_data *mtd);
void mini_next_frame(Minitime_data *mtd);
void mini_first_frame(Minitime_data *mtd);
void mini_last_frame(Minitime_data *mtd);
void mini_seek_frame(Minitime_data *mtd, SHORT ix);
void mini_clear_overlays(Minitime_data *mtd);
void mini_draw_overlays(Minitime_data *mtd);

#endif
