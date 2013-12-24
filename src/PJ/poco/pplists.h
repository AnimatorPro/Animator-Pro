
/*****************************************************************************
 *
 * 08/18/90 - this file is apparently not used in poco. (Ian)
 *
 ****************************************************************************/

#ifndef PPLISTS_H
#define PPLISTS_H

/**** this is where I put the massive global ifdef switches ****/
#define PARANOID

#ifndef TRUE
#define TRUE 1
#endif 

#ifndef FALSE
#define FALSE 0
#endif

#ifndef WORD
#define WORD int
#endif 

#ifndef lprintf
#define lprintf printf
#endif 

typedef WORD bool;		/*boolean (logical only) type*/

extern WORD debug;
extern int null_funct();

extern char *malloc();
extern int *clone_structure();
extern int *clone_zero();

extern char *clone_string();
extern char *cat_strings();
extern char *str_num();
extern char *lsprintf();

struct name_list
    {
    struct name_list *next;
    char *name;
    };
typedef struct name_list Names;

extern Names *add_name_to_list();
extern Names *remove_name_from_list();
extern Names *reverse_list();
extern Names *in_name_list();
extern Names *in_nlist();
extern Names *reverse_list();
extern Names *cat_lists();
extern Names *in_list();
extern Names *sn_list();
extern Names *remove_from_list();
extern void free_name_list();

typedef int * POINTER;
struct item_list
    {
    struct item_list *next;
    POINTER item;
	char *why;
    };
typedef struct item_list Item_list;

extern Item_list *add_item();
extern Item_list *or_in_item();
extern Item_list *in_item_list();

#define Alloc_a(type) (type *)malloc(sizeof(type) )
#define Alloc_z(type) (type *)clone_zero(sizeof(type) )
#define Clone_a(pt, type) (type *)clone_structure(pt, sizeof(type) )
#define Free_a(pt)	free(pt)
#define Copy_a(s, d) pj_copy_structure( s, d, sizeof(*(s)))
#define Array_els(array) (sizeof(array)/sizeof(array[0]) )

#endif /* PPLISTS_H */
