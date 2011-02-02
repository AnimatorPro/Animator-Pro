#ifdef BIG_COMMENT
 
First bug:
 
    This is a little isolated bug I found in the watcom compiler version 7.0
    for 386 machine I'm running the wcc386p version.  I couldn't get the 
    function parse_args() to compile without a cast to a (void *) for one
    of a (*Do_aparse)(...) call's arguments.
 
    It doesn't correctly assign the type of the variable 'ap' in the typedef
    for the Do_aparse function pointer.
 
Second bug:
 
    This has to do with compiled character constants in strings as escapes
    using hex digits of the form "\xFFmore text" The items in the little main
    below give impropper values.  I would assume with an 8 bit char the 
    constant should terminate after 2 recognized hex digits.
 
    This code is compiled with the following line:
 
        wcc386p test.c -j -3s -oxt -fpi -w2 
 
#endif /* Big comment */
 

 
/* from stdtypes.h */
 
typedef int Errcode;    /* >= 0 if ok, < 0 otherwise see errcodes.h */
#define Success 0
#define NULL ((void *)0)
 
/* from linklist.h */
 
typedef struct names {
    struct names *next;
    char *name;
} Names;
 
Names *text_in_list(char *name, Names *list);
 
#ifndef ARGPARSE_H
#define ARGPARSE_H
 
/* Note: this is NOT a typedef, but it saves typeing. */
#define Argparse_list struct argparse_list 
 
typedef int (*Do_aparse)(Argparse_list *ap, /* what's calling this */
                    int argc,  /* count left including current argument */
                    char **argv, /* positioned at current argument */
                    int position); /* position in original: argv[position] */
 
struct argparse_list {
    void *next;
    char *text;
    Do_aparse doit;
};
 
#define APLAST -1
#define ARGP(lst,idx,txt,doit) \
    { idx==APLAST?NULL:(void *)(&(lst[idx])+1),txt,doit }
 
Errcode parse_args(Argparse_list *switches, 
                   Do_aparse do_others,
                   int argc, char **argv);
 
#endif /* ARGPARSE_H */
 

 
Errcode parse_args(Argparse_list *switches, Do_aparse do_others,
                   int argc, char **argv)
{
Do_aparse doit;
Argparse_list *apl;
int position;
Errcode ret;
 
    ++argv;
    position = 1;
    argc -= 1;
    while(argc > 0)
    {
        if((apl = (Argparse_list *)text_in_list(*argv,
                                   (Names *)switches)) != NULL)
        {
            doit = apl->doit;
        }
        else
            doit = do_others;
 
        if(doit != NULL)
        {
#ifdef WORKAROUND
            /* note the (void*) this is needed because of some compiler bug */
            if((ret = (*doit)((void *)apl, argc, argv, position)) < 0)
                return(ret);
#else
            if((ret = (*doit)(apl, argc, argv, position)) < 0)
                return(ret);
#endif
            ++ret;
        }
        else
            ret = 1;
 
        argv += ret;
        position += ret;
        argc -= ret;
    }
    return(Success);
}
 

main()
{
    printf("val %02x in \"\\x1b\"\n", *("\x1b"));
    printf("val %02x in \"\\x1ba\"\n", *("\x1bA"));
    printf("val %02x in \"\\x01ba\"\n", *("\x01bA"));
 
    printf("val %02x in \"\\x1bb\"\n", *("\x1bb"));
    printf("val %02x in \"\\x1bc\"\n", *("\x1bc"));
    printf("val %02x in \"\\x1bd\"\n", *("\x1bd"));
    printf("val %02x in \"\\x1be\"\n", *("\x1be"));
    printf("val %02x in \"\\x1bf\"\n", *("\x1bf"));
 
    printf("val %02x in \"\\x01bb\"\n", *("\x01bb"));
    printf("val %02x in \"\\x01bc\"\n", *("\x01bc"));
    printf("val %02x in \"\\x01bd\"\n", *("\x01bd"));
    printf("val %02x in \"\\x01be\"\n", *("\x01be"));
    printf("val %02x in \"\\x01bf\"\n", *("\x01bf"));
}
#ifdef OUTPUT_OF_ABOVE
 
val 1b in "\x1b"
val ba in "\x1ba"
val ba in "\x01ba"
val bb in "\x1bb"
val bc in "\x1bc"
val bd in "\x1bd"
val be in "\x1be"
val bf in "\x1bf"
val bb in "\x01bb"
val bc in "\x01bc"
val bd in "\x01bd"
val be in "\x01be"
val bf in "\x01bf"
 
#endif