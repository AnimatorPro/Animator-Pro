#define REXLIB_INTERNALS
#define REXLIB_CODE
#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "jfile.h"
#include "rexlib.h"
#include "rexlib.h"
#include "filepath.h"

extern char *clone_string(char *s);

void pj_rexlib_free(Rexlib **prl)
{
Rexlib *rl;
void *entry;

    if((rl = *prl) == NULL)
        return;
    if(rl->cleanup != NULL)
        ((Rexlib_cleanup)(rl->cleanup))(rl);
	entry = rl->host_data;
	pj_freez(&rl->first_hostlib);
    pj_rex_free(&entry);
	*prl = NULL;
}
Errcode pj_rexlib_init(Rexlib *rl,void *data)
{
extern USHORT program_id;
extern USHORT program_version;

	if(rl->init == NULL)
		return(Success);
	return(((Rexlib_init)(rl->init))(rl,program_id,program_version,data));
}
static Errcode load_hostlibs(char *name, Rexlib *rex_header, 
							 Hostlib *first_hl,Libhead **hostlibs)
{
USHORT type;
Libhead *hosthead;
Hostlib *prev_hl, *hl;
Hostlib *hostpath = NULL;


    if(hostlibs)
		hosthead = *hostlibs;
	else
		hosthead = NULL;

    while(first_hl)
    {
		hl = first_hl;

        if(hosthead == NULL)
		{
		/* no more host librarys see if what rexlib still wants is optional 
		 * if they are optional set them to null otherwise Bomb */

			if(hl->type == AA_LOADPATH)
			{
				if((hl->version & ~(HLIB_OPTIONAL)) != AA_LOADPATH_VERSION)
					return(Err_library_version);
				hostpath = hl;
			}
			else if(!(hl->version & HLIB_OPTIONAL))
				return(Err_host_library);
			first_hl = hl->next;
			hl->next = NULL;
			continue;
		}
		type = hosthead->type;

		if(type == AA_NOT_AVAILABLE)
			goto get_next_hosthead;

		if(type == AA_LOADPATH)
			return(Err_bad_input);

		/* search list from rex code and see if we have what it wants */

		/* this will make prev_hl->next point to first_hl */
		prev_hl = TOSTRUCT(Hostlib,next,&first_hl);

		for(;;)
		{
			if(type == hl->type)
			{
				if(hosthead->version < (hl->version & ~(HLIB_OPTIONAL)))
					return(Err_library_version);

				/* if we have a match load hostlib header pointer in next of
				 * field hl after detatching it from list */

				prev_hl->next = hl->next;
				hl->next = hosthead;
				break;
			}
			prev_hl = hl;
			if((hl = (Hostlib *)(hl->next)) == NULL)
				break;
		}
get_next_hosthead:
        hosthead = *hostlibs++; /* get next libhead input by host */
    }
	/* if rexlib has requested it we clone the path rexfile loaded from
	 * and put it in the "first_hostlib" field for later freeing and 
	 * in the _a_a_loadpath.next for rexlib access */

	if(hostpath != NULL)
	{
		if(NULL == (hostpath->next = clone_string(name)))
			return(Err_no_memory);
		rex_header->first_hostlib = (Hostlib *)(hostpath->next);
	}
	return(Success);
}
Errcode pj_rexlib_load(char *name, /* path to find rex library file */
                    USHORT type, /* Rexlib code type seeking to find */
					Rexlib **prl,	 /* Rexlib structure to load */
          			void **hostlibs,  /* null terminated list of 
							   	   	   * of Libheads for REX code 
									   * NULL == no list */
					char *id_string) /* id_string checked against
									  * id_string in library if type 
									  * is REX_USERTYPE this value
									  * must be non NULL */

/* load a rex code library into ram 
 * handshake, load data, check for errors and pre-initialize 
 * host libraries for rex code and return pointer to header. */
{
Errcode err;
Rexlib *rl;
void *entry;
Hostlib *firstlib; /* pointer to first Hostlib header in code */
extern Rexlib *pj_rexlib_verify(void *entry);

	*prl = NULL;

    /* Verify it's a pj rex library, and retrieve Rexlib header pointer */

    if((err = pj_rex_load(name,&entry,prl)) < Success)
        return(err);

	rl = *prl;
	rl->host_data = (void *)entry;

	/* we re-use the first_hostlib pointer for the optional loadpath string 
	 * and maybe other things in the future retrieve and 
	 * set to NULL for free */

	firstlib = rl->first_hostlib;
	rl->first_hostlib = NULL;

	/* at this point all pointers are set and 
	 * it is safe to call free_rexlib() to clean up */

    if(rl->type != type) /* is rex library the right type ?? */
    {
        err = Err_rexlib_type;
        goto error;
    }

	if(id_string != NULL)
	{
		if(rl->id_string == NULL 
			|| 0 != strcmp(rl->id_string,id_string))
		{
			goto id_string_error;
		}
	}
	else if(type == REX_USERTYPE && rl->id_string != NULL)
		goto id_string_error;

    /* load libraries in rex code from input args verify library types
     * and versions and whether all are satisfied */

	if((err = load_hostlibs(name,rl,firstlib,hostlibs)) < Success)
		goto error;
	goto done;

id_string_error:
	err = Err_rexlib_usertype;
error:
    pj_rexlib_free(prl);
done:
    return(err);
}
