@rem - Since High C doesn't ship with a make utility this little batch
@rem - file builds the examples for High C the old fashioned way.
@rem - If you do use a make utility check out makefile.hc,  which will
@rem - work with most makes, including Borland's and Watcom's.
hc386 -I.. -l..\flc386ps circflic.c
hc386 -I.. -l..\flc386ps copyflic.c
hc386 -I.. -l..\flc386ps playfram.c
hc386 -I.. -l..\flc386ps quickflc.c
386asm farcopy.asm -NOLIST -TWOCASE
hc386 -DUSING_PHARLAP -I.. -l..\flc386ps custom.c farcopy.obj
@rem - The -Hloc in the command line below is needed to keep High C from
@rem   padding the Fli_head struct.
hc386 -Hloc -I.. -l..\flc386ps ramflic.c
