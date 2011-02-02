
/* fli header */


typedef struct fli_id 
    {
    LONG create_time;    /* time file created */
    LONG create_user;    /* user id of creator */
    LONG update_time;    /* time of last update */
    LONG update_user;    /* user id of last update */
    } Fli_id;

#define FLIHR_MAGIC     0xAF12      /* Hi res FLI magic */

struct flihr_head
    {
    LONG size;              /* size of entire fli file */
    USHORT type;            /* == FLIHR_MAGIC */
    USHORT frame_count;     /* # of frames in fli (ring frame not included) */
    USHORT width;           /* pixel width */
    USHORT height;          /* pixel height */
    USHORT bits_a_pixel;    /* 8 for this version at least... */
    SHORT flags;            /* set to zero/ignore for now */
    SHORT speed;            /* time between frames in 70th of a second */
    LONG next_head;         /* set to zero/ignore */
    Fli_id id;              /* who made fli and when. */
    UBYTE reserved1[42];    /* set to zero */
    LONG frame1_oset;       /* offset to first frame */
    LONG frame2_oset;       /* offset to second frame (for looping) */
    UBYTE reserved2[40];    /* set to zero */
    };

