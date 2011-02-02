#ifndef ERRCODES_H
#define ERRCODES_H

/* errcode values used by our routines */

/* no error */
#define Success ((int)0)

/* general errors */

#define Err_nogood      -1 /* something generally went wrong */
#define Err_no_memory  	-2 /* no memory available */
#define Err_bad_input	-3 /* bad input data */
#define Err_format 		-4 /* general bad data format */
#define Err_no_vram   	-5 /* out of video display memory */
#define Err_no_stack   	-6 /* out of stack space */
#define Err_reported   	-7 /* still an error but was reported below */
#define Err_unimpl  	-8 /* feature unimplemented */
#define Err_overflow	-9 /* data overflow for size of result */
#define Err_not_found   -10 /* object not found */
#define Err_bad_magic   -11 /* bad magic id number on data or record */
#define Err_abort		-12 /* user abort request */
#define Err_timeout		-13 /* the call waiting timed out */
#define Err_wrong_res	-14 /* Can't deal with object of this dimension */
#define Err_too_big		-15 /* object too big to handle */
#define Err_version		-16 /* correct file type but new version */
#define Err_bad_record	-17 /* record magic number is bad */
#define Err_uninit		-18 /* subsystem not initialized */
#define Err_wrong_type  -19 /* object found but not requested type */
#define Err_spline_points -20 /* too many points in spline */
#define Err_user_lock	-21   /* file locked to unauthorized users */
#define Err_rgb_convert -22  /* only CONVERT program can load rgb image files */
#define Err_pic_unknown -23  /* picture file format unknown */

/* system errors */
#define SYSERR -50
#ifdef SYSERR
#define Err_stdio		(SYSERR -0) /* error occurred in stdio routine */
/* end SYSERR */ 
#endif


#define REXERR -75
#ifdef REXERR
#define Err_file_not_rex (REXERR-0) /* file isn't a REX file */
#define Err_not_rexlib	(REXERR-1) /* File is rex but not a pj rex library */
#define Err_rexlib_type (REXERR-2) /* File is rex library but wrong type */ 
#define Err_host_library (REXERR-3) /* missing host provided library */
#define Err_library_version	(REXERR-4) /* wrong host library type or vers */
#define Err_rexlib_usertype	(REXERR-5) /* USER TYPE id_string doesn't match */
/* end REXERR */ 
#endif

/* io and file errors */

#define FERR -100
#ifdef FERR
#define Err_no_file		(FERR -0) /* file not found */
#define Err_no_path		(FERR -1) /* path not found */
#define Err_no_device	(FERR -2) /* device not found */
#define Err_write   	(FERR -3) /* write error */
#define Err_read   		(FERR -4) /* read error */
#define Err_seek    	(FERR -5) /* seek error */
#define Err_eof     	(FERR -6) /* end of file */
#define Err_in_use 		(FERR -7) /* file in use */ 
#define Err_extant 		(FERR -8) /* file exists */
#define Err_create		(FERR -9) /* file creation error */
#define Err_truncated   (FERR -10) /* file data truncated */
#define Err_corrupted   (FERR -11) /* file data corrupted or invalid */
#define Err_no_space    (FERR -12) /* out of space writing to device */
#define Err_disabled    (FERR -13) /* the device,window,file etc. disabled */
#define Err_invalid_id  (FERR -14) /* invalid id value */
#define Err_file_not_open  (FERR -15) /* file is not open */
#define Err_suffix		(FERR-16) /* unrecognized file suffix */
#define Err_too_many_files (FERR-17) /* can't open file cause too many open */
#define Err_access		(FERR-18)	/* Don't have permission to use this file */
#define Err_sys_mem_bad (FERR-19)	/* MS-DOS memory block corrupted */
#define Err_bad_env		(FERR-20)	/* MS-DOS environment block bad */
#define Err_bad_address (FERR-21)  /* "Invalid memory-block address", */
#define Err_disk_format (FERR-22)	/* Invalid format */
#define Err_file_access (FERR-23)	/* Bad file access code (internal) */
#define Err_data		(FERR-24)	/* Invalid data */
#define Err_no_more_files (FERR-25)  /* No more files */
#define Err_write_protected (FERR-26)  /* Disk write protected. */
#define Err_disk_not_ready (FERR-27) /* No disk in drive */
#define Err_not_dos_disk (FERR-28)   /* Not an MS-DOS disk */
#define Err_disk_data 	(FERR-29)		/* Disk data error */
#define Err_sector 		(FERR-30)		/* Disk sector not found */
#define Err_no_paper 	(FERR-31)		/* Printer out of paper */
#define Err_general_failure (FERR-32) /* MS-DOS general failure */
#define Err_critical 	(FERR-33)		/* critical error */
#define Err_network 	(FERR-34)		/* General network failure */
#define Err_file_share 	(FERR-35)	/* file sharing error */
#define Err_file_lock 	(FERR-36)		/* file lock error */
#define Err_disk_change (FERR-37)	/* invalid disk  change */
#define Err_no_remote 	(FERR-38)		/* remote computer not listening */
#define Err_network_busy (FERR-39)  /* Network busy */
#define Err_share_pause (FERR-40)	/* Sharing temporarily paused */
#define Err_redirect_pause (FERR-41) /* File/printer redirection paused */
#define Err_directory_entry (FERR-42) /* Can't create directory entry */
#define Err_dir_too_long	(FERR-43) /* Directory name too big for ms-dos */
#define Err_dir_name_err	(FERR-44) /* Directory name malformed */
#define Err_file_name_err	(FERR-45) /* FIle name malformed (extra .?) */
#define Err_no_temp_devs (FERR-46) /* Temp files path doen't have any devs. */ 
#define Err_macrosync	 (FERR-47) /* Input macro out of sync */
#define Err_no_record	(FERR-48)    /* Data record not found */
#define Err_end_of_record	(FERR-49)    /* End of data record */
/* end FERR */ 
#endif

/* rex file and rex library errors */

/* video driver subsystem errors */

#define VDERR -200
#ifdef VDERR
#define Err_rast_type	(VDERR-0) /* invalid raster type for operation */
#define Err_no_display	(VDERR-1) /* display hardware not found, invalid */	
#define Err_clipped		(VDERR-2) /* Blit clipped out entirely */
#define Err_no_8514		(VDERR-3) /* 8514/a adapter not found */
/* ------ gap ------- */
#define Err_no_such_mode (VDERR-7) /* Asking for a mode that isn't there */
#define Err_isopen		(VDERR-8) /* driver is already open can't open again */
#define Err_driver_protocol (VDERR-9) /* driver behaving irrationally */
#define Err_pdepth_not_avail (VDERR-10) /* pixel depth not available */
#define Err_aspect_not_disp (VDERR-11) /* aspect ratio not displayable */
/* end VDERR */ 
#endif

#ifndef REXLIB_CODE 

/* window errors */
#define WERR -220
#ifdef WERR
#define Err_tomany_wins	(WERR -0) /* to many windows requested */
/* end WERR */ 
#endif

/* font subsystem errors */
#define FONTERR -300
#ifdef FONTERR
#define Err_unknown_font_type (FONTERR-0) /* most likely it's a .fnt suffix,
										   * but not Animator 1.0 compatible*/
/* end FONTERR */ 
#endif

/* ADI errors */
#define ADIERR -350
#ifdef ADIERR
#define Err_adi_driver_bad_status (ADIERR-0)
/* end ADIERR */ 
#endif

/* poco subsystem errors */
#define POCOERR -400
#ifdef POCOERR
#define Err_stack (POCOERR-2)
#define Err_bad_instruction	(POCOERR-3)
#define Err_syntax (POCOERR-4)
#define Err_poco_internal (POCOERR-5)
#define Err_in_err_file	(POCOERR-7)		/* look in errfile for error */
#define Err_null_ref (POCOERR-8)
#define Err_no_main (POCOERR-10)
#define Err_zero_divide (POCOERR-11)
#define Err_float (POCOERR-12)
#define Err_invalid_FILE (POCOERR-13)
#define Err_index_small (POCOERR-14)
#define Err_index_big (POCOERR-15)
#define Err_poco_free (POCOERR-16)
#define Err_free_null (POCOERR-17)
#define Err_free_resources (POCOERR-18)
#define Err_zero_malloc (POCOERR-19)
#define Err_string	(POCOERR-20)
#define Err_fread_buf (POCOERR-21)
#define Err_fwrite_buf (POCOERR-22)
#define Err_buf_too_small (POCOERR-23)
#define Err_parameter_range (POCOERR-24)
#define Err_early_exit (POCOERR-25)
#define Err_too_few_params (POCOERR-26)
#define Err_function_not_found (POCOERR-27)
#define Err_too_many_params (POCOERR-28)
/* end POCOERR */ 
#endif

#define FLIERR -500
#ifdef FLIERR
#define Err_no_cel	(FLIERR-0) 	/* no cel present */
#define Err_too_many_frames (FLIERR-1)
#define Err_no_chunk	(FLIERR-2)	/* data chunk not found */
/* end FLIERR */ 
#endif

/* Softmenu related errors */
#define SMUERR -600
#ifdef SMUERR
#define Err_unmatched_quote (SMUERR-0)
#define Err_unmatched_squote (SMUERR-1)
#define Err_unmatched_brace (SMUERR-2)
#define Err_line_too_long (SMUERR-3)
#define Err_mu_syntax (SMUERR-4)
#define Err_no_such_class (SMUERR-5)
#define Err_not_enough_fields (SMUERR-6)
#define Err_too_many_fields (SMUERR-7)
#define Err_expecting_string (SMUERR-8)
#define Err_expecting_lbrace (SMUERR-9)
#define Err_expecting_rbrace (SMUERR-10)
#define Err_expecting_id (SMUERR-11)
#define Err_expecting_number (SMUERR-12)
#define Err_invalid_char (SMUERR-13)
#define Err_expecting_name (SMUERR-14)
#define Err_duplicate_name (SMUERR-15) /* duplicate symbol in menu file */


#define Err_too_many_args (SMUERR-13)  /* too many replaceable arguments 
										* in a format string */
#define Err_n_args_invalid (SMUERR-14)  /* formats can't include %n types */
#define Err_bad_argnum  (SMUERR-15)  /* ![??] argument number decriptor is 
									  * invalid */
#define Err_format_invalid (SMUERR-16) /* a formattter '%%' type argument is
										* invalid */
/* end SMUERR */ 
#endif


#define FATALERR -900
#define Err_integ   (FATALERR -0) /* data integrity error */
#define Err_swap	(FATALERR -1) /* can't restore context of file we swapped */

int get_errtext(int err, char *buf);
	/* buf must be ERRTEXT_SIZE */
#define ERRTEXT_SIZE 128

/* REXLIB_CODE */ 
#endif

#endif /* ERRCODES_H */
