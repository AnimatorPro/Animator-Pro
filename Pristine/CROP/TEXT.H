
struct textline 
        {
        struct textline *next;
        struct textline *last;
        char *line;
        };

struct textlist
        {
        struct textline *first;
        struct textline *stopper;
        struct textline *end;
        };

extern char *text_buf;

#define MAXTEXTSIZE 32000        /* max size for test buffer */
#define DTSIZE 16000        /* default size for text buffer */
extern long fstring_width();
