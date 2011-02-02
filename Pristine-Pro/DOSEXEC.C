/*****************************************************************************
 * DOSEXEC.C - Fire up a DOS shell or command from within the current program.
 *
 *	 Errcode text_mode_exec(char *cmdline)
 *	   This function saves the graphics state, goes to text mode, executes
 *	   the DOS command specified (or runs an interactive shell if no command
 *	   is passed), then restores the graphics state.  It does no error
 *	   reporting (unless it bombs the whole application because the graphics
 *	   state can't be restored).  This one is used by the Poco system() call.
 *
 *	 Errcode shell_out_to_dos(void)
 *	   This function runs the interactive shell, or if the shell can't be
 *	   started it reports the problem via softerr().  This one is used by
 *	   the pulldown menu DOS SHELL selection.
 ****************************************************************************/

#include "jimk.h"
#include "errcodes.h"

#include <stdlib.h> 	/* environ stuff comes from here */
#include <process.h>	/* spawn() family functions come from here */
#include <errno.h>		/* so we can handle errors from spawn() */

#define ENV_PAD 1024	/* the amount of env space to add when shelling out */

static Errcode dos_exec_command(char *cmdline)
/*****************************************************************************
 * fire up a DOS shell.  video must be in text mode before calling this.
 ****************************************************************************/
{
	Errcode status;
	int 	envsize;
	char	**envptr;
	char	*comspec;
	char	*cmdflag;
	char	envsize_string[16];
	static char newprompt[128];
	static char oldprompt[128] = "";

	/*------------------------------------------------------------------------
	 * find the user's preferred command shell.
	 *----------------------------------------------------------------------*/

	if (NULL == (comspec = getenv("COMSPEC"))) {
		comspec = "COMMAND.COM";
	}

	/*------------------------------------------------------------------------
	 * set the DOS prompt to the old prompt with "[Ani]" in front of it
	 *	(to remind them they're shelled out from us).  we only have to set up
	 *	the prompt strings once, since they live in static buffers.  (and they
	 *	MUST live in static buffers, that's the nature of envstring stuff.)
	 *----------------------------------------------------------------------*/

	if (oldprompt[0] == '\0') {
		char *prompt;
		if (NULL == (prompt = getenv("PROMPT"))) {
			prompt = "$p$g";
		}
		sprintf(oldprompt, "PROMPT=%s", prompt);
		sprintf(newprompt, "PROMPT=[Ani] %s", prompt);
	}

	putenv(newprompt);

	/*------------------------------------------------------------------------
	 * count up the size of the current environment data (which we'll pass
	 * to the new shell), and add a padding factor of ENV_PAD to the
	 * current size (so that SET commands from the shell don't fail).
	 *----------------------------------------------------------------------*/

	envsize = 0;
	for (envptr = environ; *envptr != NULL; ++envptr) {
		envsize += strlen(*envptr);
	}
	sprintf(envsize_string, "/e:%d", envsize + ENV_PAD);

	/*------------------------------------------------------------------------
	 * if we were passed a command line, set the '/c' flag for the call to
	 * dos, else set the flag and the command line image to empty strings,
	 * which will make the command processor run in interactive mode.
	 *----------------------------------------------------------------------*/

	if (cmdline == NULL || cmdline[0] == '\0') {
		cmdflag = "";
		cmdline = "";
	} else {
		cmdflag = "/c";
	}

	/*------------------------------------------------------------------------
	 * spawn the shell, passing our environment (with a modified prompt)
	 * and the padded environment size.
	 *----------------------------------------------------------------------*/

	status = spawnlpe(P_WAIT, comspec,
		comspec, envsize_string, cmdflag, cmdline, NULL, environ);

	/*------------------------------------------------------------------------
	 * restore the old prompt, and return the status.
	 *	running command.com doesn't really give us a useful status.
	 *	basically, it ran and that's Success, or it didn't, because we
	 *	couldn't find it or there wasn't enough memory to load it.  this is
	 *	a limitation of command.com, which isn't bright enough to return
	 *	the status of the last command it ran.
	 *----------------------------------------------------------------------*/

	putenv(oldprompt);

	if (status == -1) {
		switch (errno) {
		  case ENOMEM:
			return Err_no_memory;
		  case ENOENT:
			return Err_no_file;
		  default:
			return Err_nogood;
		}
	} else {
		return Success;
	}
}


Errcode text_mode_exec(char *command_line)
/*****************************************************************************
 * revert to text mode and run a dos command.  saves/restores graphics screen.
 *
 *	 return value is Success, Err_no_file, or Err_no_memory.  the latter two
 *	 happen if there's a problem firing up command.com; if command.com runs
 *	 the return value is success regardless of whether command.com did what
 *	 you expected it to or not (this is a command.com limitation).
 *
 *	 if the video mode can't be restored properly, this informs the user of
 *	 the problem, attempts to save the user's work, and exits the program.
 ****************************************************************************/
{
	Errcode 	err;
	Errcode 	cmdstatus;
	Screen_mode oldmode;
	char		pathbuf[128];
	char		errtextbuf[ERRTEXT_SIZE];
	Rcel_save opic;


	/*------------------------------------------------------------------------
	 * save the graphics screen.
	 *----------------------------------------------------------------------*/

	flush_tempflx();	/* clean up act in case we can't reopen video */

	oldmode = vconfg.smode;

	if (Success > (err = temp_save_rcel(&opic, vb.cel_a))) {
		return softerr(err, "dos_command");
	}

	pj_close_raster(vb.cel_a);
	pj_close_vdriver(&vb.vd);	/* driver should revert to text mode, but	*/
	restore_ivmode();			/* restore_ivode() makes extra sure of it.	*/

	/*------------------------------------------------------------------------
	 * execute the dos command, save the status for the caller.
	 *----------------------------------------------------------------------*/

	cmdstatus = dos_exec_command(command_line);

	/*------------------------------------------------------------------------
	 * fix up things the dos command might have screwed up on us.
	 *----------------------------------------------------------------------*/

	pj_set_gs();		/* in case dos command ran in protected mode */
	pj_clock_init();	/* in case the clock chip was reprogrammed.  */

	/*------------------------------------------------------------------------
	 * restore the graphics screen.
	 *----------------------------------------------------------------------*/

	make_resource_name(oldmode.drv_name, pathbuf);
	if (Success > (err = pj_open_ddriver(&vb.vd, pathbuf))) {
		goto FATAL_ERROR;
	}

	if(Success > (err = pj_vd_open_screen(vb.vd, vb.cel_a,
						oldmode.width, oldmode.height, oldmode.mode))) {
		pj_close_vdriver(&vb.vd);
		goto FATAL_ERROR;
	}

	report_temp_restore_rcel(&opic, vb.cel_a);

	return cmdstatus;

FATAL_ERROR:

	/*------------------------------------------------------------------------
	 * if we can't reopen the video graphics mode, whine about it,
	 * try to save the user's work-in-progress, then exit the program.
	 *----------------------------------------------------------------------*/

	disable_textboxes();		/* force error reporting to text mode */
	restore_ivmode();			/* make extra sure we're in text mode */

	softerr(err, "!%s%d%d%d", "driver_open",
				   pathbuf, oldmode.width, oldmode.height, oldmode.mode);
	printf("\n");

	flush_tsettings(TRUE);				/* update temp settings file */
	close_tflx();						/* not sure what this does */
	delete_file_list(work_temp_files);	/* punt files we don't wanna save */
	trd_ram_to_files(); 				/* save everything we can */

	exit(1);							/* so sorry, gotta go. */

}

Errcode shell_out_to_dos(void)
/*****************************************************************************
 * function to process pulldown menu selection to shell out to dos.
 ****************************************************************************/
{
	Errcode err;

	if (Success != (err = text_mode_exec(NULL))) {
		softerr(err, "dos_command");
	}
	return err;
}



#ifdef OLDWAY

 NOTE: this code refers to functions static to MAIN.C...if we were to
	   revert to this method (Bad Idea I think) it would have to be
	   moved back into MAIN.C.

Errcode text_mode_exec(Errcode *command_status_return, char *command_line)
/*****************************************************************************
 * revert to text mode, shell out to dos, upon return restore graphics mode.
 *	This can be called from Poco or from pulldown menu covering func below.
 *	It returns Success, or a negative error code if the video/user state
 *	couldn't be restored.  The status from the dos command is returned
 *	separately, via the (optional) command_status_return parameter.
 *	If command_line is NULL or an empty string, an interactive command shell
 *	is started, otherwise it runs the command and returns without going
 *	interactive with the user.
 ****************************************************************************/
{
	Errcode 	err;
	Rectangle	oldsize;
	Screen_mode oldmode;
//	Cmap		ocolors;

	/*------------------------------------------------------------------------
	 * save user's work-in-progress and info about current video mode.
	 *----------------------------------------------------------------------*/

//	ocolors.num_colors = COLORS;			/* these should come back from */
//	pj_cmap_copy(vb.pencel->cmap,&ocolors); /* the tflx when we reload it. */

	oldmode = vconfg.smode;
	copy_rectfields(vb.pencel,&oldsize);
	scrub_cur_frame();
	flush_tflx();

	/*------------------------------------------------------------------------
	 * revert to (presumably text) video mode we had at program startup.
	 *----------------------------------------------------------------------*/

	push_close_toscreen();
	free_undof();				/* re alloc'd by set_flisize */
	close_init_after();
	cleanup_screen();

	/*------------------------------------------------------------------------
	 * execute the dos command, save the status for the caller.
	 *----------------------------------------------------------------------*/

	err = dos_exec_command(command_line);
	if (command_status_return != NULL) {
		*command_status_return = err;
	}

	/*------------------------------------------------------------------------
	 * fix up things the dos command might have screwed up on us.
	 *----------------------------------------------------------------------*/

	pj_set_gs();		/* in case dos command ran in protected mode */
	pj_clock_init();	/* in case the clock chip was reprogrammed.  */

	/*------------------------------------------------------------------------
	 * reopen the old graphics video mode, reload user's work-in-progress.
	 *----------------------------------------------------------------------*/

	if ((err = init_screen(&oldmode,&oldmode,init_after_screen,NULL)) < Success) {
		boxf("Fatal error in restoring screen, err = %d", err);
		goto FATAL_ERROR;
	}

	if((err = set_flisize(&oldsize)) < Success) {
		boxf("Fatal error in setting flisize, err = %d", err);
		goto FATAL_ERROR;
	}

	if((err = open_tempflx(TRUE)) < Success) {
		softerr(err,"tflx_screen");
		goto FATAL_ERROR;
	}

	return Success;

FATAL_ERROR:

	return err; // for now
}

#endif /* OLDWAY */

