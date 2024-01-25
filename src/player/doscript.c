#include "player.h"
#include "gfx.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

static Rgb3 pure_white = {255,255,255};
static Rgb3 pure_black = {0,0,0};

static char	call_word[] 	= "call";
static char	choice_word[] 	= "choice";
static char	exittodos_word[] = "exittodos";
static char	exittoplayer_word[] = "exittoplayer";
static char	gosub_word[] 	= "gosub";
static char	link_word[] 	= "link";
static char break_word[] 	= "break";
static char endchoice_word[] = "endchoice";
static char endloop_word[] 	= "endloop";
static char endsub_word[] 	= "endsub";
static char escape_word[] 	= "escape";
static char freefli_word[] 	= "freeflic";
static char keychoice_word[] = "keychoice";
static char loadfli_word[] 	= "loadflic";
static char loop_word[] 	= "loop";
static char return_word[] 	= "return";
static char subroutine_word[] = "subroutine";

static Errcode change_to_pathdir(char *path)
/* changes to directory of drawer part of a path */
{
char drawer[PATH_SIZE];

	strcpy(drawer,path);
	*pj_get_path_name(drawer) = 0; /* truncate name from path */
	if(drawer[0] != 0) /* if a drawer */
		return(change_dir(drawer)); /* change directory to path */
	return(Success);
}
static void free_pnode(Cnode *pn)
{
	free_slist(pn->labels);
	pj_freez(&pn);
}
static Errcode pop_cnode()

/* "pop" node.  Go back to calling script file seek to where we left off. */
{
Errcode err;
Cnode *pn;

	free_key_choices();
	pcb.toklen = pcb.reuse_tok = 0;
	if((pn = pcb.cn) == NULL)
		return(Err_eof);

	pn = pcb.cn->parent;

	free_pnode(pcb.cn);
	pcb.cn = pn;

	if(pcb.scr_file)
		fclose(pcb.scr_file);
	pcb.scr_file = NULL;

	if(pn == NULL)
		return(Err_eof);

	if((err = change_to_pathdir(pn->scr_path)) < Success)
		return(err);

	if((pcb.scr_file = fopen(pn->scr_path,"r")) == NULL)
		return(Err_no_file);

	if(fseek(pcb.scr_file,pn->fpos,SEEK_SET))
		return(Err_seek);

	return(Success);
}
static void clear_token()
{
	pcb.toklen = pcb.reuse_tok = 0;
}
static Errcode push_cnode(char *in_path)
{
Errcode err;
Cnode *pn;
char scr_path[PATH_SIZE];


	clear_token();
	if(pcb.cn)
	{
		pcb.cn->fpos = ftell(pcb.scr_file);

		if(pcb.scr_file)
			fclose(pcb.scr_file);
		pcb.scr_file = NULL;
	}

	if((err = get_full_path(in_path, scr_path)) < Success)
		return(err);

	/* change to directory of script path */

	if((err = change_to_pathdir(scr_path)) < Success) 
		return(err);

	if((pn = pj_zalloc(sizeof(Cnode))) == NULL)
		return(Err_no_memory);

	/* open the script file */

	if((pcb.scr_file = fopen(scr_path,"r")) == NULL)
	{
		err = Err_no_file;
		goto error;
	}

	/* save script name for later reopen if calling out */

	strcpy(pn->scr_path,scr_path);

	/* add to list */

	pn->parent = pcb.cn;
	pcb.cn = pn; 

	/* set up atom stack */

	pn->anext = pn->astack;
	pn->amax = pn->astack + sizeof(pn->astack);

	return(Success);
error:
	free_pnode(pn);
	return(err);
}
void cleanup_pstack()

/* Cleans up all that might be alloc'd during a script play
 * frees all pnodes in current list and closes any open script files */
{
Cnode *pn;
Cnode *pnt;

	fclose(pcb.scr_file);
	pcb.scr_file = NULL;

	pn = pcb.cn;
	while(pn != NULL)
	{
		pnt = pn->parent;
		free_pnode(pn);
		pn = pnt;
	}
	pcb.cn = NULL;
	free_key_choices();
	free_ramflis();
	change_dir(vb.init_drawer);
}
static void rewind_script()
{
	rewind(pcb.scr_file);
	pcb.cn->line = 0;
	clear_token();
}
static Errcode jump_offset(LONG offset,SHORT line)
{
	if(fseek(pcb.scr_file,offset,SEEK_SET))
		return(Err_seek);
	pcb.cn->line = line;
	return(Success);
}
static Errcode load_root_script(char *path)
/* opens the lowest level script and scans it for syntax and ram fli loading */
{
Errcode err;

	if((err = push_cnode(path)) < Success)
		goto error;
	if((err = scan_script()) < Success)
		goto error;
	if((err = verify_ramflis()) < Success)
		goto error;
	rewind_script();
error:
	return(err);
}
static Errcode push_atom(int type, int size, Atom** patom)

/* note that this reports out of stack error */
{
Cnode *pn = pcb.cn;
Atom *at;

	if((pn->anext + size) >= pn->amax)
	{
		return(soft_script_error(Err_no_stack,"gosub_nest"));
	}
	at = (Atom *)(pn->anext);
	pn->anext += size;
	at->parent = pn->atom;
	at->type = type;
	pn->atom = at;
	if(patom != NULL)
		*patom = at;
	return(Success);
}
static void pop_atom()
/* pops current atom off stack, note this does not destroy the atom but re
 * adjusts stack pointers. */
{
	if(pcb.cn->atom != NULL)
	{
		pcb.cn->anext = (char *)(pcb.cn->atom);
		pcb.cn->atom = pcb.cn->atom->parent;
	}
}
static Boolean in_atom(int type)
{
	return(pcb.cn->atom != NULL && pcb.cn->atom->type == type);
}
static Errcode on_to_next(char *token)
{
Errcode err;

	/* go forward to next token of same case insensitive value */

	for(;;)
	{
		if((err = get_token()) < Success)
			return(err);
		if(txtcmp(pcb.token,token)==0)
			return(Success);
	}
}
static Errcode do_key_choice()
{
Errcode err;
int ix;
char *found;

	if(pcb.choice != 0)
	{
		if((found = strchr(pcb.choices,pcb.choice)) == NULL)
		{
			err = Err_corrupted;
			goto error;
		}
		ix = found - pcb.choices;

		if(fseek(pcb.scr_file,pcb.choice_osets[ix],SEEK_SET) < 0)
		{
			err = Err_seek;
			goto error;
		}
		clear_token();
		pcb.cn->line = pcb.choice_lines[ix];
		err = Success;
	}
	else /* a pause or loop spec ended and finished */
	{
		err = break_choice(NULL);
	}

error:
	free_key_choices();
	return(err);
}
static Errcode abort_to_choice()
/* will take script(s) back to a previous keychoice or exit the script if not
 * within a keychoice */
{
Errcode err;

	clear_token();
	for(;;)
	{
		while(pcb.cn->atom)
		{
			if(in_atom(CHOICE_ATOM))
				return(break_choice(NULL));
			pop_atom();
		}
		if((err = pop_cnode()) < Success) /* pop one callscript level */
			return(err);
	}
	return(Success);
}
static Errcode cant_show_fli(Errcode err)
{
	return(softerr(err,"!%s%d%s", "play_cant_display",
					   pcb.cn->scr_path, pcb.fliline+1,
					   pcb.fliname ));
}
Errcode play_script(Boolean by_menu)

/* plays script file in pcb.scr_root.  Reports errors if any */
{
Errcode err;
int mouse_was_on;

	mouse_was_on = hide_mouse();
	pcb.script_mode = TRUE;
	clear_token();

	close_curfli();
	if((err = load_root_script(pcb.scr_root)) < Success)
		goto error;

	for(;;)
	{
		pcb.choice = 0;
		if((err = get_scripted_anim()) < Success)
		{
			goto script_error;
		}
		pcb.abort_key = 0;
		if((err = play_scripted_anim()) < Success)
		{
			if(err != Err_abort) /* error will make main() go to dos */
				goto display_error;

			if((UBYTE)pcb.abort_key == ESCKEY)
			{
				if((err = abort_to_choice()) < Success)
					goto script_error;
				continue;
			}
			if(!by_menu) /* so main will not drop through to menu */
				err = Err_reported;
			goto error;
		}
		if(pcb.choices)
		{
			if((err = do_key_choice()) < Success)
				goto error;
		}
	}

	err = Success;
	goto error;

display_error:
	err = cant_show_fli(err);
	goto error;

script_error:
	if(err == Err_eof)
		err = Success;
error:
	if(mouse_was_on)
		show_mouse();
	err = softerr(err,"!%s", "play_cant_script", pcb.scr_root );
	close_curfli();
	cleanup_pstack();
	pcb.lock_key = 0;
	pcb.script_mode = FALSE;
	return(err);
}

static Errcode scan_secs1000(int *psec1000)
{
Errcode err;
float fsecs;

	if((err = scan_number('f',&fsecs)) >= Success)
	{
		if((*psec1000 = fsecs * 1000) >= 0)
			return(Success);
		err = Err_syntax;
	}
	return(err);
}
static Errcode get_secs1000(int *psec1000)

/* gets thousandths of a sec from a decimal seconds in the script. Does not 
 * allow negative values */
{
Errcode err;
float fsecs;

	if((err = get_number('f',&fsecs)) >= Success)
	{
		if((*psec1000 = fsecs * 1000) < 0)
			return(Err_bad_input);
	}
	return(err);
}
static Errcode get_rgb(Rgb3 *pcolor)
{
Errcode err;
char colbuf[(4+32+1)*3];
char **colargs = (char **)colbuf;
UBYTE *rgbval = &pcolor->r;
float colval;
int i;

	if((err = get_func_args(3,3,colargs,32)) < Success)
		return(err);
	for(i = 2;i >= 0;--i)
	{
		if( (sscanf(*colargs++,"%f%*c",&colval) != 1)
			|| (colval < 0 || colval > 255))
		{
			return(Err_bad_input);
		}
		*rgbval++ = (UBYTE)((float)(colval /* * 25.5 */));
	}
	return(Success);
}
static Errcode get_fades()
{
Errcode err;
int *ptime;
Rgb3 *pcolor;
int num_cuts;
int whats_done;

	num_cuts = 0;
	pcb.in_from = pcb.out_to; /* default fade in is previous fade out */
	pcb.out_to = pure_black;  /* default fade out is black */

	while(pcb.fadein < 0 || pcb.fadeout < 0)
	{
		if((err = get_token()) < Success)
		{
			/* if simply end of file and we have a fadein or out it's ok */

			if(err == Err_eof && (pcb.fadein >= 0 || pcb.fadeout >= 0))
				break;
			goto error;
		}

		if(txtcmp(pcb.token,"fadein") == 0)
		{
			pcolor = &pcb.in_from;
			ptime = &pcb.fadein;
			if(num_cuts)
				pcb.fadeout = 0;
		}
		else if(txtcmp(pcb.token,"fadeout") == 0)
		{
			pcolor = &pcb.out_to;
			ptime = &pcb.fadeout;
			if(num_cuts)
				pcb.fadein = 0;
		}
		else if(txtcmp(pcb.token,"cut") == 0)
		{
			if(++num_cuts == 2)
				pcb.fadeout = pcb.fadeout = 0 ;
			else if(pcb.fadein >= 0)
				pcb.fadeout = 0;
			else if(pcb.fadeout >= 0)
				pcb.fadein = 0;
			continue;
		}
		else if(pcb.fadein < 0 && pcb.fadeout < 0) /* neither set: error */
			goto bad_data;
		else
		{
			reuse_token();
			break; 
		}

		if(*ptime >= 0)
			goto bad_data;

		*ptime = 800; /* 0.8 secs default fade time */
		whats_done = 0;

		for(;;)
		{
			if((err = get_token()) < Success)
				goto continue_big_loop;

			if(!(whats_done & 1))
			{
				whats_done |= 1;

				if(txtcmp(pcb.token,"color") == 0)
				{
					if((err = get_rgb(pcolor)) < Success)
						goto error;
					continue;
				}
				else if(txtcmp(pcb.token,"black") == 0)
				{
					continue;
				}
				else if(txtcmp(pcb.token,"white") == 0)
				{
					*pcolor = pure_white;
					continue;
				}
				else
					whats_done &= ~1;
			}
			else if(whats_done & 2)
				break;

			if(scan_secs1000(ptime) < Success)
				break;
			whats_done |= 2;
		}
		reuse_token();

continue_big_loop:
		continue;
	}
	return(Success);

bad_data:
	err = Err_syntax;
error:
	return(err);
}
static Errcode cant_open_script(Errcode err)
{
	return(soft_script_error(err,"!%s", "cant_open_scr", pcb.token));
}
static Errcode link_script(Scan_data *sd)
{
Errcode err;
Cnode opn;

	if((err = get_required_token()) < Success)
		return(err);

	if(sd) /* if scanning check to se if file exists */
	{
		if(!pj_exists(pcb.token))
			return(cant_open_script(Err_no_file));
		return(Success);
	}

	opn = *pcb.cn; /* save this for error report */
	cleanup_pstack(); /* bye bye previous script */
	if((err = load_root_script(pcb.token)) < Success)
	{
		pcb.cn = &opn;
		err = cant_open_script(err);
		pcb.cn = NULL;
	}
	strcpy(pcb.scr_root,pcb.token); /* new one takes over ! */
	return(err);
}
static Errcode load_called_script()
{
Errcode err;

	if((err = push_cnode(pcb.token)) < Success)
		return(cant_open_script(err));
	return(Success);
}
static Errcode call_script(Scan_data *sd)
{
Errcode err;

	if((err = get_required_token()) < Success)
		return(err);
	if((err = load_called_script()) >= Success)
		err = scan_script_labels();
	return(err);
}
static Errcode get_func_args(int minargs, int maxargs, 
							 char **args, int argsize)

/* Argsize includes size of terminal null. Each argument is a single token 
 * returns number of args retrieved or Errcode, does not clear args area */
{
Errcode err;
char *pbuf;
int argcount;
char last_tok;

	if((err = get_token()) < Success)
		goto error;

	if(pcb.token[0] != '(')
		goto syntax_error;

	pbuf = (char *)(&args[maxargs]);

	last_tok = ',';
	argcount = 0;

	for(;;)
	{
		if((err = get_token()) < Success)
			goto error;

		if(last_tok == ',')
		{
			if(pcb.token[0] == ',')
				goto syntax_error;
			if(pcb.token[0] == ')')
			{
				if(argcount == 0 && minargs == 0) /* ok, no args */
					break;
				goto syntax_error;
			}
			if(argcount >= maxargs)
				return(Err_too_many_params);

			args[argcount] = pbuf;
			pbuf += sprintf(pbuf,"%.*s", argsize, pcb.token );
			++pbuf;
			++argcount;
		}
		else /* last token not a comma */
		{
			if(pcb.token[0] == ')') /* done */
			{
				if(argcount < minargs)
					return(Err_parameter_range);
				break;
			}
			if(pcb.token[0] != ',')
				goto syntax_error;
		}
		last_tok = pcb.token[0];
	}
	return(argcount);
syntax_error:
	return(Err_syntax);
error:
	return(err);
}
static void free_key_choices()
{
	pcb.choice = 0;
	pj_freez(&pcb.choices);
}
static int get_choice_key()
{
UBYTE havekey;

	if(pcb.toklen == 1)
	{
		havekey = toupper(pcb.token[0]);
		if(! ((havekey >= '0' && havekey <= '9') 
	   			|| (havekey >= 'A' && havekey <= 'Z'))) 
		{
			goto bad_choice;
		}
	}
	else if(!txtcmp(pcb.token,escape_word))
		havekey = ESCKEY;
	else
		goto bad_choice;

	return(havekey);
bad_choice:
	return(soft_script_error(Err_bad_input,"!%s", "bad_choice", pcb.token ));
}
static Errcode load_key_choices()
{
Errcode err;
Choice_atom *ca;
char keys[100];
LONG osets[100];
LONG lines[100];
int havekey;
char *key = keys;
LONG *oset = osets;
LONG *line = lines;
char *wantkey;
int level;
int len; 

	free_key_choices();

	if((err = push_atom(CHOICE_ATOM,sizeof(Choice_atom),
						 (Atom **)&ca)) < Success)
	{
		return(err);
	}

	wantkey = NULL;
	level = 0;
	keys[0] = 0;

	for(;;)
	{
		if((err = get_token()) < Success)
			return(err);

		if(wantkey)
		{
			if((havekey = get_choice_key()) < 0)
				return((Errcode)havekey);

			if(strchr(keys,havekey) != NULL)
			{
				return(soft_script_error(Err_syntax,"!%s", "dubl_choice",
									pcb.token ));
			}
			*wantkey = havekey;
			if((*oset++ = ftell(pcb.scr_file)) < 0)
				return(Err_seek);
			*line++ = pcb.cn->line; 
			wantkey = NULL;
		}

		if(!txtcmp(pcb.token,choice_word))
		{
			if(level > 0)
				continue;
			wantkey = key++;
		}
		if(!txtcmp(pcb.token,keychoice_word))
		{
			++level;
			continue;
		}
		else if(!txtcmp(pcb.token,endchoice_word))
		{
			if(level > 0)
				--level;
			else
				break;
		}
	}
	*key = 0;

	if((len = strlen(keys)) == 0)
		return(unexpected());

	ca->end_oset = ftell(pcb.scr_file);
	ca->end_line = pcb.cn->line;

	if((pcb.choices = pj_malloc( 
					(len*(sizeof(char)+(sizeof(LONG)*2))) + 1 )) == NULL)
	{
		return(Err_no_memory);
	}
	pcb.choice_osets = (LONG *)(pcb.choices + len + 1);
	pcb.choice_lines = pcb.choice_osets + len;

	strcpy(pcb.choices,keys);
	len *= sizeof(LONG);
	copy_mem(lines,pcb.choice_lines,len);
	copy_mem(osets,pcb.choice_osets,len);

	if(pcb.pause < 0) 	/* if no pause set then set it to infinite */
		pcb.pause = 0;

	return(Success);
}
static Errcode break_choice(Scan_data *sd)
{
Errcode err;
Choice_atom *ca;

	if(!in_atom(CHOICE_ATOM))
		return(unexpected());
	ca = (Choice_atom *)(pcb.cn->atom);
	err = jump_offset(ca->end_oset,ca->end_line);
	pop_atom();
	return(err);
}
static Errcode push_loop(Scan_data *sd)
{
Errcode err;
char arg[32+4+1];
Loop_atom *la;

	if((err = push_atom(LOOP_ATOM,sizeof(Loop_atom),(Atom **)&la)) < Success)
		return(err);

	if((err = get_func_args(1,1,(char **)arg,32)) < Success)
		goto error;
	if((txtcmp(&arg[4],"forever")) == 0)
		la->count = 0;
	else if( (sscanf(&arg[4],"%hu%*.c",&la->count) != 1)
	 		 || la->count <= 0)
	{
		err = Err_bad_input;
		goto error;
	}

	if(sd) /* if scanning simply do loop once */
	{
		la->count = 0;
	}
	else
	{
		--la->count;
		la->top_oset = ftell(pcb.scr_file);
		la->code_line = pcb.cn->line;
	}

	return(Success);
error:
	return(soft_script_error(err,"loop_count"));
}
static Errcode do_endloop(Scan_data *sd)
{
Loop_atom *la;

	if(!in_atom(LOOP_ATOM))
		return(unexpected());

	la = (Loop_atom *)pcb.cn->atom;

	if(la->count == 0) /* last time, pop up one level */
		pop_atom();
	else
	{
		if(la->count > 0)
			--la->count;
		return(jump_offset(la->top_oset,la->code_line));
	}
	return(Success);
}
static Errcode put_token(char *token)
{
	pcb.toklen = sprintf(pcb.token,token);
	reuse_token();
	return(Success);
}
static Errcode put_exitscript()
{
	return(put_token("exitscript"));
}
static Errcode break_loop(Scan_data *sd)
{
Errcode err;
Atom *at;

	while((at = pcb.cn->atom) != NULL)
	{	
		if(at->type == GOSUB_ATOM)
			break;
		else if(at->type == LOOP_ATOM)
		{
			if((err = on_to_next("endloop")) < Success)
				return(unexpected());
			pop_atom();
			return(Success);
		}
		pop_atom();
	}
	return(unexpected());
}
static Errcode get_load_type()
{
Errcode err;
char *suffix;

	if((err = get_token()) < Success)
		return(err);
	if(	*(suffix = pj_get_path_suffix(pcb.token)) != 0 
			 && txtcmp(suffix,".pdr") != 0)
	{
		return(Err_suffix);
	}
	if((suffix - pcb.token > 8)
		|| get_drawer_len(pcb.token))
	{
		return(Err_file_name_err);
	}
	*suffix = 0;
	if(txtcmp(pcb.token,"flc"))
		sprintf(pcb.loadpdr,"%s.pdr", pcb.token);

	return(Success);
}
static Errcode get_required_fullpath(char *fp)

/* gets path from pcb.token */
{
Errcode err;

	/* ignore current directorys on removable devices */

	if(	   pcb.toklen > 2
		&& pcb.token[1] == ':' 
		&& pcb.token[2] != '\\'
		&& (pj_pathdev_is_fixed(pcb.token) == 0) )
	{
		sprintf(fp,"%c:\\%.*s",pcb.token[0],sizeof(pcb.fliname)-3,
								&pcb.token[2]);

		return(Success);
	}

	if((err = get_full_path(pcb.token,fp)) < Success)
		err = soft_script_error(err,"!%s", "cant_open",pcb.token);
	return(err);
}
static Errcode get_required_flipath()
{
Errcode err;

	if((err = get_required_token()) < Success)
		return(err);

	return(get_required_fullpath(pcb.fliname));
}

static Errcode get_play_spec(Boolean scanning)
{
Errcode err;
ULONG doneflags;
ULONG checkflags;
char argstr[32];

	pcb.loadpdr[0] = 0;
	pcb.fliline = pcb.cn->line; /* save line number because possible
								 * error report will be 
								 * when file is opened after control
								 * arguments are parsed */

	pcb.flioset = ftell(pcb.scr_file);

	if((err = get_required_fullpath(pcb.fliname)) < Success)
		return(err);

	doneflags = 0;

	/* set all items un-loaded or to default values */

	pcb.pause = -1;  /* fli pause */
	pcb.sspeed = -1; /* fli speed */
	pcb.loops = 1;   /* fli loops not script loops */
	pcb.fadein = -1;  /* fli fade in time */
	pcb.fadeout = -1; /* fli fade out time */


	for(;;)
	{
		/* if this fails we terminate, next read will also return eof */
		if(get_token() < Success)
			break;

		if(pcb.token[0] != '-')
		{
			if(txtcmp(pcb.token,keychoice_word) == 0)
			{
				if(scanning)
					err = scan_keychoice();
				else
					err = load_key_choices();
				if(err < Success)
					goto error;
			}
			else
				reuse_token();

			break;
		}

		if(strlen(pcb.token) > sizeof(argstr))
			goto invalid_option;

		strcpy(argstr,pcb.token);

		if(!txtcmp(&pcb.token[1],"s"))
		{
			checkflags = 0x0001;
			err = getuint(&pcb.sspeed);
			pcb.sspeed = jiffies_to_millisec(pcb.sspeed);
		}
		else if(!txtcmp(&pcb.token[1],"l"))
		{
			checkflags = 0x0002;
			err = getuint(&pcb.loops);
		}
		else if(!txtcmp(&pcb.token[1],"p"))
		{
			checkflags = 0x0004;
			err = get_secs1000(&pcb.pause);
		}
		else if(!txtcmp(&pcb.token[1],"t"))
		{
			checkflags = 0x0008;
			if((err = get_fades()) < Success)
				goto error;
		}
		else if(!txtcmp(&pcb.token[1],"type"))
		{
			checkflags = 0x0010;
			err = get_load_type();
		}
		else
		{
			goto invalid_option;
		}

		if(err >= Success)
		{
			if(doneflags & checkflags)
			{
				err = Err_syntax;
				goto error;
			}
			else
			{
				doneflags |= checkflags;
				continue;
			}
		}
		goto bad_argument;
	}
	return(Success);

invalid_option:
	return(soft_script_error(err,"!%s", "bad_option", pcb.token ));
bad_argument:
	return(soft_script_error(err,"!%s%s", "bad_arg", argstr, pcb.token));
error: 
	return(soft_script_error(err,NULL));
}
static Errcode unexpected()
{
	return(soft_script_error(Err_syntax,"!%s", "unexpected", pcb.token));
}
static Errcode unexpected_eof()
{
	return(soft_script_error(Err_eof,"eof_err"));
}
static Errcode get_required_token()
{
	if(get_token() < Success)
		return(unexpected_eof());
	return(Success);
}
static Errcode check_substart(Scan_data *sd)
{
	if(pcb.cn->atom)
		return(unexpected());
	return(Err_eof);
}
static Errcode do_subret(Scan_data *sd)
{
Errcode err;
Gosub_atom *ga;

	if(!in_atom(GOSUB_ATOM))
		return(unexpected());
	ga = (Gosub_atom *)(pcb.cn->atom);
	err = jump_offset(ga->ret_oset,ga->ret_line);
	pop_atom();
	return(err);
}
static Errcode do_return(Scan_data *sd)
{
	while(pcb.cn->atom && pcb.cn->atom->type != GOSUB_ATOM)
		pop_atom();

	if(pcb.cn->atom == NULL)
		return(put_exitscript());
	return(do_subret(sd));
}
static Errcode no_subroutine(char *subname,int line)
{
Errcode err;
int oline;

	oline = pcb.cn->line;
	pcb.cn->line = line;
	err = soft_script_error(Err_not_found,"!%s","sub_not_found", subname);
	pcb.cn->line = oline;
	return(err);
}
static Errcode call_gosub(Scan_data *sd)
{
Errcode err;
Gosub_atom *ga;
Label *lab;

	if((err = push_atom(GOSUB_ATOM,sizeof(Gosub_atom),(Atom **)&ga)) < Success)
		return(err);

	if((err = get_token()) < Success)
		return(err);

	if((lab = (Label *)text_in_list(pcb.token,(Names *)(pcb.cn->labels))) 
											== NULL)
	{
		return(no_subroutine(pcb.token,pcb.cn->line));
	}
	ga->ret_oset = ftell(pcb.scr_file);
	ga->ret_line = pcb.cn->line;
	return(jump_offset(lab->oset,lab->line));
}
static Errcode ret_success(Scan_data *sd)
{
	return(Success);
}
static Errcode ret_eof(Scan_data *sd)
{
	return(Err_eof);
}
static Errcode ret_abort(Scan_data *sd)
{
	return(Err_abort);
}
static Errcode scan_label(Boolean call)

/* see if already declared, if not put in list, if so then we cant do it 
 * twice */
{
Errcode err;
Label *lab;

	if((err = get_token()) < Success)
		return(err);
	if(!isalnum(pcb.token[0]))
		return(unexpected());
	if((lab = (Label *)text_in_list(pcb.token,(Names *)(pcb.cn->labels)))
						!= NULL)
	{
		if(call)
			return(Success);

		if(lab->oset != -1)
		{
			return(soft_script_error(Err_syntax,"!%s", "sub_declared",
							  pcb.token));
		}
	}
	else
	{
		if((lab = pj_malloc(sizeof(Label)+pcb.toklen)) == NULL)
			return(Err_no_memory);

		/* put in list */
		lab->next = pcb.cn->labels;
		pcb.cn->labels = lab;

		/* load name */
		lab->name = lab->nbuf;
		strcpy(lab->nbuf,pcb.token);
	}

	lab->line = pcb.cn->line;
	if(call)
		lab->oset = -1;
	else
		lab->oset = ftell(pcb.scr_file);
	return(Success);
}
static Errcode scan_gosub(Scan_data *sd)
{
	return(scan_label(TRUE));
}
static Errcode scan_substart(Scan_data *sd)
{
Errcode err;

	if((pcb.cn->flags & SCAN_IN_SUB) || pcb.cn->atom != NULL)
		return(unexpected());
	if((err = push_atom(GOSUB_ATOM,sizeof(Atom),NULL)) < Success)
		return(err);
	pcb.cn->flags |= SCAN_IN_SUB;
	return(scan_label(FALSE));
}
static Errcode scan_endsub(Scan_data *sd)
{
	if( !(pcb.cn->flags & SCAN_IN_SUB) || !in_atom(GOSUB_ATOM))
		return(unexpected());
	pcb.cn->flags &= ~SCAN_IN_SUB;
	pop_atom();
	if(get_token() >= Success)
	{
		if(txtcmp(pcb.token,subroutine_word)!=0)
			return(unexpected());
		reuse_token();
	}
	return(Success);
}
static Errcode verify_labels()
{
Label *lab;

	lab = pcb.cn->labels;
	while(lab != NULL)
	{
		if(lab->oset == -1)
			return(no_subroutine(lab->name,lab->line));
		lab = lab->next;
	}
	return(Success);
}
static Errcode scan_keychoice()
{
Errcode err;
Choice_atom *ca;

	if((err = push_atom(CHOICE_ATOM,sizeof(Choice_atom),
						 (Atom **)&ca)) < Success)
	{
		return(err);
	}
	if((err = get_required_token()) < Success)
		return(err);
	if(txtcmp(pcb.token,choice_word) != 0)
		return(unexpected());
	reuse_token();
	return(Success);
}
static Errcode scan_endchoice(Scan_data *sd)
{
	if(!in_atom(CHOICE_ATOM))
		return(unexpected());
	pop_atom();
	return(Success);
}
static Errcode scan_choice(Scan_data *sd)
{
Errcode err;
int choicekey;

/* Choice_atom *ca; */

	if(!in_atom(CHOICE_ATOM))
		return(unexpected());
	if((err = get_token()) < Success)
		return(err);
	if((choicekey = get_choice_key()) < 0)
		return(choicekey);
	return(Success);
}
static Errcode scan_script_labels()
/* function called to load labels into list for a script node, assumes script
 * has already been scanned for syntax, used when opening and loading sub
 * scripts by call_script() */
{
Errcode err;

	while(get_token() >= Success)
	{
		if(txtcmp(pcb.token,subroutine_word) == 0
			&& (err = scan_label(FALSE)) < Success)
		{
			return(soft_script_error(err,NULL));
		}
	}
	rewind_script();
	return(Success);
}
static Errcode scan_call(void *dat)
{
Errcode err;
char path[PATH_SIZE];
Names *sc;

	if((err = get_required_token()) < Success)
		return(err);

	if((err = get_required_fullpath(path)) < Success)
		return(err);

	if(text_in_list(path,pcb.scan_calls) != NULL)
		return(Success);

/* add new name to list of scan calls */

	err = strlen(path) + (sizeof(Names)+1);
	if((sc = pj_malloc(err)) == NULL)
		return(Err_no_memory);

	strcpy((sc->name = (char *)(&sc[1])),path);
	sc->next = pcb.scan_calls;
	pcb.scan_calls = sc;

	return(load_called_script());
}
static Errcode do_loadfli()
{
Errcode err;
char device[DEV_NAME_LEN];
Ramfli *rf;
Flifile flif;
Boolean isfixed;
void *cbuf;

	if((err = get_required_flipath()) < Success)
		return(err);

	/* make absolutely sure we can get whatever compression buffer is needed */

	pj_freez(&pcb.cbuf);
	if((cbuf = pj_malloc(pcb.max_cbuf_size)) == NULL)
		return(Err_no_memory);

	isfixed = pj_pathdev_is_fixed(pcb.fliname);

	if(!isfixed)
	{
		get_path_device(pcb.fliname,device);
		soft_continu_box("!%s%s", "play_ask_fd", device, pcb.fliname );
	}

	if((rf = find_ramfli(pcb.fliname)) == NULL)
	{
		err = soft_script_error(Err_nogood,"load_unplayed");
		goto error; /* if we got to here err >= Success */
	}

	if(isfixed && rf->frames != NULL) /* already loaded */
		goto error; /* if we got to here err >= Success */

	if((err = pj_fli_open(pcb.fliname,&flif,JREADONLY)) < Success)
		goto fli_error;

	if((err = load_ramfli(rf, &flif)) < Success)
		goto fli_error;

	err = Success;
fli_error:
	pj_fli_close(&flif);
error:
	pj_free(cbuf);
	if(err < Success)
		return(cant_load_fli(err));
	return(err);
}
static Errcode scan_loadfli()
{
Errcode err;

	if((err = get_required_flipath()) < Success)
		return(err);

	return(add_specified_fli(RF_LOAD_ASKED));
}
static Errcode scan_freefli()
{
Errcode err;

	if((err = get_required_flipath()) < Success)
		return(err);

	return(add_specified_fli(RF_FREE_ASKED));
}
static Errcode do_freefli()
{
Errcode err;
Ramfli *rf;

	if((err = get_required_flipath()) < Success)
		return(err);

	if((rf = find_ramfli(pcb.fliname)) == NULL)
	{
		return(soft_script_error(Err_nogood,"not_loaded"));
	}

	free_ramfli_data(rf);
	return(Success);
}

#define WTAB cmd_list

static Wordfunc cmd_list[] = {

	/*  index   runfunc			scanfunc		command word */

	INIT_WF(0,	push_loop,		push_loop,		loop_word),
	INIT_WF(1,  do_endloop, 	do_endloop,		endloop_word),
	INIT_WF(2,  call_script,	scan_call,		call_word),
	INIT_WF(3,  break_choice,	scan_choice,	choice_word),
	INIT_WF(4,  break_choice,   scan_endchoice,	endchoice_word),
	INIT_WF(5,  ret_eof,		ret_success,	exittodos_word),
	INIT_WF(6,  ret_abort,		ret_success,	exittoplayer_word),
	INIT_WF(7,  link_script,	link_script,	link_word),
	INIT_WF(8,  call_gosub,		scan_gosub,		gosub_word),
	INIT_WF(9,	check_substart, scan_substart,	subroutine_word),
	INIT_WF(10,	do_subret, 		scan_endsub,	endsub_word),
	INIT_WF(11,	do_return, 		ret_success,	return_word),
	INIT_WF(12,	break_loop, 	ret_success,	break_word),
	INIT_WF(13,	unexpected, 	unexpected,		keychoice_word),
	INIT_WF(14,	do_loadfli, 	scan_loadfli,	loadfli_word),
	INIT_WF(15,	do_freefli, 	scan_freefli,	freefli_word),
};
#undef WTAB

#define command_list  ((Names *)(&cmd_list[Array_els(cmd_list)-1]))

static Errcode unexpected_end()
{
char *word;

	if(!pcb.cn->atom)
		goto eof_err;

	switch(pcb.cn->atom->type)
	{
		case CHOICE_ATOM:
			word = keychoice_word;
			break;
		case LOOP_ATOM:
			word = loop_word;
			break;
		case GOSUB_ATOM:
			word = subroutine_word;
			break;
		default:
			goto eof_err;
	}
	return(soft_script_error(Err_eof,"!%s","unterminated", word));
eof_err:
	return(unexpected_eof());
}
static Errcode cant_load_fli(Errcode err)
{
	return(soft_script_error(err,"!%s", "cant_load_flic", pcb.fliname ));
}
static Errcode add_specified_fli(USHORT why)
{
Errcode err;
Ramfli *rf;
Flifile flif;
LONG speed;
char device[DEV_NAME_LEN];
Boolean on_floppy;

	if(why == RF_PLAY_ASKED && pcb.loadpdr[0])
		return(Success);

	if((rf = find_ramfli(pcb.fliname)) != NULL)
		goto got_it;

	if((err = pj_pathdev_is_fixed(pcb.fliname)) < 0)
		return(err);

	on_floppy = (err == 0);

	if(on_floppy)
	{
		clear_struct(&flif); /* for close below */
	}
	else if((err = pj_fli_open(pcb.fliname,&flif,JREADONLY)) < Success)
	{
		if(why == RF_LOAD_ASKED)
			return(cant_load_fli(err));
		if(err == Err_no_file)
		{
			if(why == RF_PLAY_ASKED)
				return(cant_show_fli(err));
			return(soft_script_error(err,NULL));
		}
		return(Success); /* don't worry about pics until later */
	}

	if((add_ramfli(pcb.fliname, &rf)) < Success)
		goto fli_error;
	rf->minspeed = 10000000; /* real big */

	if(on_floppy)
	{
		rf->speed = 10000000; /* real big */
		rf->flags |= RF_ON_FLOPPY;
		rf->cbuf_size = 0; /* we will resolve this later */
	}
	else
	{
		if(flif.hdr.frame_count == 1)
			rf->flags |= RF_ONE_FRAME;
		rf->speed = flif.hdr.speed;
		rf->cbuf_size = pj_fli_cbuf_size(flif.hdr.width,
										 flif.hdr.height, COLORS );
	}

	pj_fli_close(&flif);

got_it:

	rf->flags |= why;

	if(why == RF_PLAY_ASKED)
	{
		if(pcb.fadein < 100)
			rf->flags |= RF_LOAD_FIRST;
		if(pcb.loops != 1)
			rf->flags |= RF_LOAD_RING;

		if(pcb.sspeed < 0)
			speed = rf->speed;
		else
			speed = pcb.sspeed;
		if(speed < rf->minspeed)
			rf->minspeed = speed;
	}

	return(Success);
fli_error:
	pj_fli_close(&flif);
	return(err);
}
static Errcode verify_ramflis()

/* scans through ram fli list, scans for maximum cbuf size and deletes
 * ramflis not loaded */
{
Ramfli *rf, *next;
Ramfli *newlist = NULL;

	next = pcb.ramflis;
	pcb.max_cbuf_size = 0x00ffff; /* at least 64k */

	while(next)
	{
		rf = next;
		next = next->next;

		if( (rf->flags & (RF_PLAY_ASKED|RF_ON_FLOPPY))
				== (RF_PLAY_ASKED)) 
		{
			if(pcb.max_cbuf_size < rf->cbuf_size)
				pcb.max_cbuf_size = rf->cbuf_size;
		}

		if(   ((rf->flags & (RF_LOAD_ASKED|RF_PLAY_ASKED))
									!= (RF_LOAD_ASKED|RF_PLAY_ASKED)) 
			  || ((rf->flags & (RF_LOAD_FIRST|RF_ON_FLOPPY|RF_ONE_FRAME)) 
			  						== (RF_ONE_FRAME)) 
		  	)
		{
			if((rf->flags & (RF_FREE_ASKED|RF_LOAD_ASKED)) == RF_FREE_ASKED)
				soft_continu_box("!%s", "play_free", rf->name );
			free_ramfli(rf);
			continue;
		}
		/* add to list of ones to be loaded */
		rf->next = newlist;
		newlist = rf;
	}

	pcb.ramflis = newlist;
	return(Success);
}
static Errcode scan_script()

/* scans script and it's sub scripts to check syntax and setup ram loading of 
 * flis */
{
Errcode err;
Wordfunc *wf;
Scan_data sd;

	pcb.max_cbuf_size = 0;
	pcb.cn->flags &= ~SCAN_IN_SUB;
	for(;;)
	{
		if((err = get_token()) < Success)
		{
			if(err != Err_eof)
				goto error;

			if(pcb.cn->atom != NULL 
				|| (pcb.cn->flags & SCAN_IN_SUB))
			{
				err = unexpected_end();
				goto error;
			}
			if((err = verify_labels()) < Success)
				goto error;
			if(pcb.cn->parent == NULL)
				break;
			if((err = pop_cnode()) < Success)
				goto error;
			continue;
		}

		if((wf = (Wordfunc *)text_in_list(pcb.token,command_list)) != NULL)
		{
			if((err = (*(wf->scanfunc))(&sd)) < Success)
				goto error;
		}
		else if(txtcmp(pcb.token,"exitscript") == 0)
		{	
			continue;
		}
		else 
		{
			if((err = get_play_spec(TRUE)) < Success)
				goto error;
			if((err = add_specified_fli(RF_PLAY_ASKED)) < Success)
				goto error;
		}
	}

	err = Success;
	goto done;
error:
	free_ramflis();
done:
	free_slist(pcb.scan_calls);
	pcb.scan_calls = NULL;
	if(err == Err_eof)
		err = unexpected_eof();
	return(soft_script_error(err,NULL));
}

static Errcode get_scripted_anim()

/* parses script until the next animation is to be played,  It loads all that
 * is needed from the script to play the next animation file */
{
Errcode err;
Wordfunc *wf;

	for(;;)
	{
		if((err = get_token()) < Success)
		{
			if(err == Err_eof)
				goto exit_script;
			break;
		}
		if((wf = (Wordfunc *)text_in_list(pcb.token,command_list)) != NULL)
		{
			if((err = (*(wf->func))(NULL)) < Success)
				break;
		}
		else if(txtcmp(pcb.token,"exitscript") == 0)
		{
			goto exit_script;
		}
		else
		{
			return(get_play_spec(FALSE));
		}

		continue;

exit_script:
		if((err = pop_cnode()) < Success)
			break;
		continue; /* back to calling script */
	}

	return(err);
}
