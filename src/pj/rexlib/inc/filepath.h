#ifndef FILEPATH_H
#define FILEPATH_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* Maximum directory name size ms-dos will accept */
#define DEV_NAME_LEN 2+1 	/* for ms dos includes null terminator */

#define MAX_DIR_LEN 66
#define MAX_FILE_NAME_LEN 8
#define MAX_SUFFIX_NAME_LEN 3
#define FILE_NAME_SIZE (8+1+3+1)

/* size of a string big enough to hold any path/file name */
#define PATH_SIZE 80

/* string big enough for a wildcard */
#define WILD_SIZE 16

#define DIR_DELIM	'\\'
#define DIR_DELIM_STR	"\\"
#define DEV_DELIM	':'
#define DEV_DELIM_STR	":"
#define SUFF_DELIM  '.'
#define SUFF_DELIM_STR  "."

 

/* Items in _a_a_loadpath: get_load_path() is the only call 
 * now in _a_a_loadpath **/

/* get_load_path() returns a poiner to a static buffer containing the path
 * the rexlib code was loaded from ie "d:\dir1\dir2\yourcode.drv" 
 * it may not be a full path but contains the path exactly as supplied by
 * the loading program to load_rexlib() Do not declare this library if
 * you do not need it */

char *pj_get_load_path(void);

/***** items in _a_a_syslib ******/

  /* REXLIB_CODE */


/* returns pointer to "dot" of suffix or terminating '\0' if no suffix */
char *pj_get_path_suffix(char *path); 

/* gets pointer to last name in a path. */
char *pj_get_path_name(char *path);


#ifdef BIG_COMMENT /*************************************/

sample usage: 
{
char path[PATH_SIZE];  /* contains file path including file name */

 /* get path into a local buffer */

 	strcpy(path, pj_get_load_path());

 /* replace suffix on path */

	strcpy(pj_get_path_suffix(path),".NEW"); 

 /* replace last name on path */

	strcpy(pj_get_path_name(path), "NEWNAME.XXX");

 /* truncate file name from path */

 	*pj_get_path_name(path) = 0;

 /* truncate suffix from from path */

 	*pj_get_path_suffix(path) = 0;

#endif /* BIG_COMMENT ************************************/




#endif /* FILEPATH_H */
