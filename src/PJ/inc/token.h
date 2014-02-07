/*****************************************************************************
 *
 * token.h - defines and prototypes for token.c
 *
 * MAINTENANCE
 *	08/21/90	(Ian)
 *				Changed #define's to enum, added Token_t typedef, as part of
 *				setting up to use enum types more, since debuggers like 'em.
 *				Also, added #include for stdtypes.
 *	10/30/90	(Ian)
 *				Added 'plen' parameter to tokenize_word routine.  If this
 *				parm is non-NULL, the number of bytes placed into the word
 *				buffer will be placed into the SHORT that this parm points to.
 ****************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

#ifndef STDTYPES_H
  #include "stdtypes.h"
#endif

/* Watcom C's stdlib.h redefines atof, so watch out for that! */
#ifdef atof
#error "atof redefined"
#endif

/* values stored in tok_type will be an ascii char, or one of these... */

typedef enum token_t {
	TOK_LBRACE	= '{',
	TOK_RBRACE	= '}',
	TOK_LPAREN	= '(',
	TOK_RPAREN	= ')',
	TOK_COMMA	= ',',
	TOK_HIASC	= 257,
	TOK_EOF,
	TOK_INT,
	TOK_QUO,
	TOK_UNDEF,
	TOK_EQ,
	TOK_NE,
	TOK_LE,
	TOK_GE,
	TOK_LSHIFT,
	TOK_RSHIFT,
	TOK_LAND,
	TOK_LOR,
	TOK_SQUO,
	TOK_LINE_COMMENT,
	TOK_PLUS_PLUS,
	TOK_PLUS_EQUALS,
	TOK_MINUS_MINUS,
	TOK_MINUS_EQUALS,
	TOK_START_COMMENT,
	TOK_DIV_EQUALS,
	TOK_END_COMMENT,
	TOK_MUL_EQUALS,
	TOK_MOD_EQUALS,
	TOK_RSHIFT_EQUALS,
	TOK_LSHIFT_EQUALS,
	TOK_AND_EQUALS,
	TOK_OR_EQUALS,
	TOK_XOR_EQUALS,
	TOK_ARROW,
	TOK_LONG,
	TOK_DOUBLE,

	TOK_TOK_MAX 			/* this one MUST be last! */
	} Token_t;

/* prototypes... */

extern int atoi(const char *s);
extern long atol(const char *s);
extern double atof(const char *s);
extern long htol(const char *s);

extern char *tokenize_word(char *line, char *oword, char *qstring,
						SHORT *plen, SHORT *ttype, Boolean quote);

#endif /* TOKEN_H */
