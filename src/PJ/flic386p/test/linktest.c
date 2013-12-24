/*****************************************************************************
 * LINKTEST.C - Link everything in the lib, to find unresolved references.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * first a bunch of bogus protos...
 *--------------------------------------------------------------------------*/

void pj_clock_init(void);
void pj_clock_cleanup(void);
void pj_clock_1000(void);
void pj_clock_jiffies(void);
void pj_clock_jiffies2ms(void);
void pj_clock_ms2jiffies(void);
void pj_malloc(void);
void pj_zalloc(void);
void pj_free(void);
void pj_freez(void);
void pj_doserr_install_handler(void);
void pj_doserr_remove_handler(void);
void pj_error_get_message(void);
void pj_error_internal(void);
void pj_video_add(void);
void pj_video_add_all(void);
void pj_video_detect(void);
void pj_video_close(void);
void pj_video_open(void);
void pj_video_find_open(void);
void pj_video_find(void);
void pj_video_mode_info(void);
void pj_video_get_current(void);
void pj_raster_make_custom(void);
void pj_raster_free_custom(void);
void pj_raster_make_compress_only(void);
void pj_raster_free_compress_only(void);
void pj_raster_make_ram(void);
void pj_raster_free_ram(void);
void pj_raster_bind_ram(void);
void pj_raster_unbind_ram(void);
void pj_raster_center_virtual(void);
void pj_raster_clip_virtual(void);
void pj_raster_make_centered(void);
void pj_raster_free_centered(void);
void pj_raster_make_offset(void);
void pj_raster_free_offset(void);
void pj_raster_copy(void);
void pj_raster_clear(void);
void pj_col_load(void);
void pj_col_save(void);
void pj_cmap_update(void);
void pj_playoptions_init(void);
void pj_flic_play(void);
void pj_flic_play_once(void);
void pj_flic_play_timed(void);
void pj_flic_play_until(void);
void pj_flic_play_next(void);
void pj_flic_play_frames(void);
void pj_flic_close(void);
void pj_flic_open(void);
void pj_flic_open_info(void);
void pj_flic_file_info(void);
void pj_flic_info(void);
void pj_flic_rewind(void);
void pj_flic_set_speed(void);
void pj_animinfo_init(void);
void pj_flic_complete_filename(void);
void pj_flic_create(void);
void pj_flic_write_first(void);
void pj_flic_write_next(void);
void pj_flic_write_finish(void);

/*----------------------------------------------------------------------------
 * now a bunch of bogus calls...
 *--------------------------------------------------------------------------*/

void main(int argc)
{

	if (argc >= 0)	/* never let the calls execute, but we have to fool watcom*/
		return; 	/* into not optimizing them away by keying off of argc.*/

pj_clock_init();
pj_clock_cleanup();
pj_clock_1000();
pj_clock_jiffies();
pj_clock_jiffies2ms();
pj_clock_ms2jiffies();
pj_malloc();
pj_zalloc();
pj_free();
pj_freez();
pj_doserr_install_handler();
pj_doserr_remove_handler();
pj_error_get_message();
pj_error_internal();
pj_video_add();
pj_video_add_all();
pj_video_detect();
pj_video_close();
pj_video_open();
pj_video_find_open();
pj_video_find();
pj_video_mode_info();
pj_video_get_current();
pj_raster_make_custom();
pj_raster_free_custom();
pj_raster_make_compress_only();
pj_raster_free_compress_only();
pj_raster_make_ram();
pj_raster_free_ram();
pj_raster_bind_ram();
pj_raster_unbind_ram();
pj_raster_center_virtual();
pj_raster_clip_virtual();
pj_raster_make_centered();
pj_raster_free_centered();
pj_raster_make_offset();
pj_raster_free_offset();
pj_raster_copy();
pj_raster_clear();
pj_col_load();
pj_col_save();
pj_cmap_update();
pj_playoptions_init();
pj_flic_play();
pj_flic_play_once();
pj_flic_play_timed();
pj_flic_play_until();
pj_flic_play_next();
pj_flic_play_frames();
pj_flic_close();
pj_flic_open();
pj_flic_open_info();
pj_flic_file_info();
pj_flic_info();
pj_flic_rewind();
pj_flic_set_speed();
pj_animinfo_init();
pj_flic_complete_filename();
pj_flic_create();
pj_flic_write_first();
pj_flic_write_next();
pj_flic_write_finish();

}
