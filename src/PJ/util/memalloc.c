#ifdef BIG_COMMENT /*************************************/

memalloc.c contains low level alloc and free used by pj libraries
and utilities.  There are only 5 external symbols in this file 

	pj_mem_used - memory currently allocated by pj_malloc.
	pj_max_mem_used - max above has been since startup.
	pj_mem_last_fail - value of last failure used for error reporting.

	pj_malloc(); - low level allocator.
	pj_free() - low level memory free.

#endif /* BIG_COMMENT ***********************************/

#include "memory.h"
#include "ptrmacro.h"

long pj_mem_used;
long pj_max_mem_used;
long pj_mem_last_fail;


/*******

#define ALLOCLIST  This to maintain a list of memory allocated (not freed)
#define COOKIES    This to have start and end "Cookie" checking
#define C_MEMORY   To have root allocs be by malloc()
#define TRD_MEMORY To have temp drive compatible memory management

*******/


#ifdef TESTING
	#define COOKIES
	#define ALLOCLIST
#endif

#ifdef ALLOCLIST
	#include "linklist.h"

	/* for debugging a list of memory allocated */
	static Dlheader alloclist = DLHEADER_INIT(alloclist);

#endif

/* note that size must immediately precede memory allocated by 
 * low level allocator and must be a long !!!! */

#ifdef TRD_MEMORY
	#include "tfile.h"
	extern void *trd_flush_alloc(long size);
	extern long trd_freemem(void *pt);
	#define SYS_ALLOC(sz) trd_flush_alloc(sz)
	#define SYS_FREE(pt) trd_freemem(pt)
	#define SYS_SEESIZE(pt)  ((((long *)pt)[-1])-sizeof(long))
#endif /* TRD_MEMORY */

#ifdef CLIB_MEMORY
	extern void *c_askmem(long sz);
	extern long c_freemem(void *pt);
	#define SYS_ALLOC(sz) c_askmem(sz)
	#define SYS_FREE(pt) c_freemem(pt)
	#define SYS_SEESIZE(pt)  ((((long *)pt)[-1])-sizeof(long))
#endif /* CLIB_MEMORY */

typedef struct mem_chunk {

#ifdef ALLOCLIST
	Dlnode anode;
#endif /* ALLOCLIST */

#ifdef COOKIES
	ULONG start_cookie;
#endif /* COOKIES */

	char mem[1];

} Memchunk;

#ifdef COOKIES

/* magic numbers tagged at beginning and end of allocated memory blocks */
#define START_COOKIE (0x41f38327)
#define END_COOKIE (0x15998327)

void bad_cookie(Memchunk *chunk,ULONG cookie,char *txt)
{
    old_video();   
	printf("Bad %s cookie %#08x chunk %#08x size %ld Gronk!"
		, txt, cookie, chunk, SYS_SEESIZE(chunk));
	exit(0);	/* okok... */
}
#endif /* COOKIES */



void pj_free(void *p)
{
register Memchunk *chunk;

	chunk = TOSTRUCT(Memchunk,mem,p);

#ifdef COOKIES
	{
	ULONG *endcookie;

		if(chunk->start_cookie != START_COOKIE)
			bad_cookie(chunk,chunk->start_cookie,"START");

		endcookie = OPTR(chunk,SYS_SEESIZE(chunk)-sizeof(ULONG));

		if(*endcookie != END_COOKIE)
			bad_cookie(chunk,*endcookie,"END");

		*endcookie = 0;
		chunk->start_cookie = 0;
	}
#endif /* COOKIES */

#ifdef ALLOCLIST
	safe_rem_node(&chunk->anode);
#endif /* ALLOCLIST */

	pj_mem_used -= SYS_FREE(chunk);	
	return;
}

#ifdef ALLOCLIST
void check_mem_list(Memchunk *chunk)
{
#ifdef DEBUG

ULONG *list;
LONG size;

/* size, pointer. - 0 size terminates */

static ULONG checklist[64] = {
	0, 0xe0258,
	0, 0xe3518,
	0, NULL
};

	size = SYS_SEESIZE(chunk);

	for(list = checklist;*list != 0;++list)
	{
		if(size == *list++
		   && chunk == *((Memchunk **)list))
		{
			if(yes_no_box("allocing %lx sz %d fail?", chunk, size))
			{
				chunk = NULL;
				*((LONG *)chunk) = 0;
				exit(0);
			}
		}
	}
#endif /* DEBUG */

}
#endif /* ALLOCLIST */

#ifdef TESTING

void mem_info(void *p, char *text)
{
Memchunk *chunk;

	chunk = TOSTRUCT(Memchunk,mem,p);
	printf("%s ", text );
#ifdef COOKIES
	{
	ULONG *endcookie, startc;

		startc = chunk->start_cookie;
		endcookie = OPTR(chunk,SYS_SEESIZE(chunk)-sizeof(ULONG));

		printf("s%d e%d ", startc != START_COOKIE, *endcookie != END_COOKIE);
	}
#endif /* COOKIES */

	printf("@ %08x sz %d\n", p, SYS_SEESIZE(chunk));
}
#endif /* TESTING */

#ifdef SLUFFED
void print_alloclist()
{
#ifdef ALLOCLIST

Memchunk *chunk;
Dlnode *next;
int i;

	i = 20;
	for(chunk = (Memchunk *)(alloclist.head);
		(next = ((Dlnode *)chunk)->next) != NULL;
		chunk = (Memchunk *)next)
	{
		chunk = TOSTRUCT(Memchunk,anode,chunk);
		printf("un-freed memory at %lx sz %d\n", chunk, SYS_SEESIZE(chunk));
		if(--i < 0)
		{
			printf(".........\n");
			break;
		}
	}
#endif /* ALLOCLIST */
}
#endif /* SLUFFED */

#ifdef ALLOCLIST 
#ifdef COOKIES

void check_a_cookie()
/* really check 20 at a time called by poll input */
{
Memchunk *chunk;
int i;

	for(i = 20;i > 0;--i)
	{
		if((chunk = (Memchunk *)get_head(&alloclist)) != NULL)
		{
			add_tail(&alloclist,(Dlnode *)chunk);
			chunk = TOSTRUCT(Memchunk,anode,chunk);
			if(chunk->start_cookie != START_COOKIE)
			{
				bad_cookie(chunk,chunk->start_cookie,"START");
			}
			if(END_COOKIE != 
					*((ULONG *)OPTR(chunk,SYS_SEESIZE(chunk)-sizeof(ULONG))))
			{
				bad_cookie(chunk,
					*((ULONG *)OPTR(chunk,SYS_SEESIZE(chunk)-sizeof(ULONG))),
					"END");
			}
		}
	}
}

void verify_cookies(int (*reportit)(char *fmt,...), char *file,int line)
{
Memchunk *chunk;
Dlnode *next;

	for(chunk = (Memchunk *)(alloclist.head);
		(next = ((Dlnode *)chunk)->next) != NULL;
		chunk = (Memchunk *)next)
	{
		chunk = TOSTRUCT(Memchunk,anode,chunk);
		if(chunk->start_cookie != START_COOKIE)
		{
			(*reportit)("bad start cookie in %08X %s %d", 
						 chunk->mem, file, line);
			break;
		}
		if(END_COOKIE != 
				*((ULONG *)OPTR(chunk,SYS_SEESIZE(chunk)-sizeof(ULONG))))
		{
			(*reportit)("bad end cookie in %08X %s %d", 
						 chunk->mem, file, line);
			break;
		}
	}
}
#endif /* COOKIES */
#endif /* ALLOCLIST */

void *pj_malloc(unsigned size)
{
register Memchunk *chunk;
long lastsize;

	if (size <= 0)
		return(NULL);
	lastsize = size;

#ifdef COOKIES
	size += OFFSET(Memchunk,mem) + sizeof(ULONG);	
#else
	size += OFFSET(Memchunk,mem);
#endif /* COOKIES */

	if((chunk = SYS_ALLOC(size)) == NULL)
	{
		pj_mem_last_fail = lastsize;
		return(NULL);
	}

	if ((pj_mem_used += size) > pj_max_mem_used)
		pj_max_mem_used = pj_mem_used;

#ifdef COOKIES
	chunk->start_cookie = START_COOKIE;
	*((ULONG *)OPTR(chunk,SYS_SEESIZE(chunk)-sizeof(ULONG))) = END_COOKIE;
#endif /* COOKIES */

#ifdef ALLOCLIST

#ifdef DEBUG
	check_mem_list(chunk);
#endif /* DEBUG */

	add_tail(&alloclist,&chunk->anode);
#endif /* ALLOCLIST */

	return(&(chunk->mem));
}
