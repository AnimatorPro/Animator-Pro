#include "errcodes.h"
#include <stdio.h>
#include "makehdr.c"

prtecode(char *name,Errcode val)
{
	fprintf(ofile,"%c%-25s equ %d\n", tolower(*name), &name[1], val);
}
#define prtcode(ename)	prtecode(#ename,ename)

#define printit outf

main(int argc, char **argv)
{
	openit(argc,argv);
	prtcode(Success);
	printit("; /* general errors */\n");
	prtcode(Err_nogood);
	prtcode(Err_no_memory);
	prtcode(Err_bad_input);
	prtcode(Err_format);
	prtcode(Err_no_vram);
	prtcode(Err_no_stack);
	prtcode(Err_reported);
	prtcode(Err_unimpl);
	prtcode(Err_overflow);
	prtcode(Err_not_found);
	prtcode(Err_bad_magic);
	prtcode(Err_abort);
	prtcode(Err_timeout);
	prtcode(Err_wrong_res);
	prtcode(Err_too_big);
	prtcode(Err_version);
	prtcode(Err_bad_record);
	prtcode(Err_widget);


	printit("; /* system errors */\n");
	printit("; #ifdef SYSERR\n");
	prtcode(Err_stdio);
	printit("; #endif /* end SYSERR */\n");

	printit("; /* io and file errors */\n");
	printit("; #ifdef FERR\n");
	prtcode(Err_no_file);
	prtcode(Err_no_path);
	prtcode(Err_no_device);
	prtcode(Err_write);
	prtcode(Err_read);
	prtcode(Err_seek);
	prtcode(Err_eof);
	prtcode(Err_in_use);
	prtcode(Err_extant);
	prtcode(Err_create);
	prtcode(Err_truncated);
	prtcode(Err_corrupted);
	prtcode(Err_no_space);
	prtcode(Err_disabled);
	prtcode(Err_invalid_id);
	prtcode(Err_file_not_open);
	prtcode(Err_suffix);
	prtcode(Err_too_many_files);
	prtcode(Err_no_record);
	prtcode(Err_access);
	prtcode(Err_sys_mem_bad);
	prtcode(Err_bad_env);
	prtcode(Err_bad_address);
	prtcode(Err_disk_format);
	prtcode(Err_file_access);
	prtcode(Err_data);
	prtcode(Err_no_more_files);
	prtcode(Err_write_protected);
	prtcode(Err_disk_not_ready);
	prtcode(Err_not_dos_disk);
	prtcode(Err_disk_data);
	prtcode(Err_sector);
	prtcode(Err_no_paper);
	prtcode(Err_general_failure);
	prtcode(Err_critical);
	prtcode(Err_network);
	prtcode(Err_file_share);
	prtcode(Err_file_lock);
	prtcode(Err_disk_change);
	prtcode(Err_no_remote);
	prtcode(Err_network_busy);
	prtcode(Err_share_pause);
	prtcode(Err_redirect_pause);
	prtcode(Err_directory_entry);
	prtcode(Err_dir_too_long);
	prtcode(Err_dir_name_err);
	prtcode(Err_file_name_err);
	prtcode(Err_no_temp_devs);
	prtcode(Err_macrosync);
	printit("; fatal file errors\n");
	prtcode(Err_swap);
	printit("; #endif /* end FERR */\n");

	printit("; /* rex file and rex library errors */\n");
	printit("; #ifdef REXERR\n");
	prtcode(Err_file_not_rex);
	prtcode(Err_not_rexlib);
	prtcode(Err_rexlib_type);
	prtcode(Err_host_library);
	prtcode(Err_library_version);
	prtcode(Err_rexlib_usertype);
	printit("; #endif /* end REXERR */\n");

	printit("; /* video driver subsystem errors */\n");
	printit("; #ifdef VDERR\n");
	prtcode(Err_rast_type);
	prtcode(Err_no_display);
	prtcode(Err_clipped);
	prtcode(Err_no_8514);
	prtcode(Err_video_bios);
	prtcode(Err_no_such_mode);
	prtcode(Err_isopen);
	prtcode(Err_driver_protocol);
	prtcode(Err_pdepth_not_avail);
	prtcode(Err_aspect_not_disp);
	printit("; #endif /* end VDERR */\n");

	printit("; /* window errors */\n");
	printit("; #ifdef WERR\n");
	prtcode(Err_tomany_wins);
	printit("; #endif /* end WERR */\n");

	printit("; /* font subsystem errors */\n");
	printit("; #ifdef FONTERR\n");
	prtcode(Err_unknown_font_type);
	printit("; #endif /* end FONTERR */\n");

	printit("; /* poco subsystem errors */\n");
	printit("; #ifdef POCOERR\n");
	prtcode(Err_stack);
	prtcode(Err_bad_instruction);
	prtcode(Err_syntax);
	prtcode(Err_poco_internal);
	prtcode(Err_in_err_file);
	prtcode(Err_null_ref);
	prtcode(Err_no_main);
	prtcode(Err_zero_divide);
	prtcode(Err_float);
	prtcode(Err_invalid_FILE);
	prtcode(Err_index_small);
	prtcode(Err_index_big);
	prtcode(Err_poco_free);
	prtcode(Err_free_null);
	prtcode(Err_free_resources);
	prtcode(Err_zero_malloc);
	prtcode(Err_string);
	prtcode(Err_fread_buf);
	prtcode(Err_fwrite_buf);
	prtcode(Err_buf_too_small);
	prtcode(Err_parameter_range);
	prtcode(Err_early_exit);
	prtcode(Err_too_few_params);
	prtcode(Err_function_not_found);
	printit("; #endif /* end POCOERR */\n");

	printit("; #ifdef FLIERR\n");
	prtcode(Err_no_cel);
	prtcode(Err_too_many_frames);
	printit("; #endif /* end FLIERR */\n");

	prtcode(Err_integ);

	printit("\n\n");
	closeit();
}
