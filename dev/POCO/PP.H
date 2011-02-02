
/*****************************************************************************
 *
 * 08/18/90 - this file is apparently not used in poco. (Ian)
 *
 ****************************************************************************/

#ifndef PP_H
#define PP_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif /* STDTYPES_H */

#ifndef PPLISTS_H
#include "pplists.h"
#endif /* PPLISTS_H */

struct text_symbol
{
struct text_symbol *next;
char *name;
Boolean is_macro;
char *value;
struct name_list *parameters;
};
typedef struct text_symbol Text_symbol;

struct file_place
{
FILE *file;
struct file_place *pred;
char *name;
int line_count;
};
typedef struct file_place File_place;


struct conditional
{
struct conditional *next;
Boolean state;/*true or false*/
};
typedef struct conditional Conditional;

/*****global functions from tokeni.c ******/
extern void err_info();
extern void ppf_err();
extern void outta_memory();
extern void unexpected_eof();
extern void po_expecting_got();
extern void zero_divide();
extern Boolean po_eat_token();
extern Boolean skip_through();
extern char *po_skip_space();
extern char *po_chop_to();
extern char *chop_cword();
extern void po_init_pp();
extern void po_free_pp();
extern char *next_line();
extern char *next_ctoken();
extern po_get_csource_line();
extern int hash_function();
extern Text_symbol *in_hash_list();
extern void add_to_hash();

/***** global data from tokeni.c *******/
extern Boolean pp_fatal;
extern Boolean pp_eof;
extern struct file_place *file_stack;
extern struct text_symbol *define_list[];
extern struct conditional *ifdef_stack;
extern char line_b1[];/*this one gets unexpanded line*/
extern char line_b2[];


#define TOKEN_MAX 2048
#define LINE_MAX 2048
#define HASH_SIZE 256

#define LBRACE '{'
#define RBRACE '}'
#define LPAREN '('
#define RPAREN ')'
extern Names *includes;	/*where to get include files from */
extern Names *cl_defines;


#ifndef iscsymf
#define iscsymf(c) (isalpha(c) || (c) == '_')
#endif /* iscsymf */

#ifndef iscsym
#define iscsym(c) (isalnum(c) || (c) == '_')
#endif /* iscsym */
#endif /* PP_H */
