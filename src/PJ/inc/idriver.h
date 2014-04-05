
#ifndef IDRIVER_H
#define IDRIVER_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#define IDR_MAX_OPTIONS 4  /* maximum number of options each with up to 
							* 9 modes that are saved in the config */ 

typedef struct idr_option {
	char	*choice_text;
	USHORT  enabled;  /* one bit on for each enabled mode 0 to 8 */
	UBYTE	mode;     /* set to the default mode */
} Idr_option;


#define IDR_VERSION 1

typedef struct idriver
	{
	struct {
		Errcode (*init)(struct idriver *idr);
		Errcode (*cleanup)(struct idriver *idr);
	} hdr;

	void *hardware;		/* driver specific data pointer */
	struct idr_library *lib;

	Idr_option *options; /* NULL if no options */

	USHORT num_options;  /* number of options in options array */

	UBYTE does_keys;  /* does this driver read the keyboard? a driver might
					   * actually be developed that takes an input stream 
					   * from ANY source */
	UBYTE unused;
	SHORT comm_port;   /* comm port to use for any serial io set by host 
						* 0 == com1 1 == com2 etc */

	USHORT key_code;   /* msdos key code as if from bios interrupt */

	USHORT button_count; /* we actually dont read any more than 2 for now */

	LONG buttons;	     /* is 32 buttons enough? for now we only look at 
						  * two buttons, a "pen" button and a "right" button
						  * these are defined in input.h as MBPEN and MBRIGHT */

	USHORT channel_count; /* x,y,z, theta?, etc? for now we only use 2 
						   * for X and Y for a mouse like pointing device 
						   * channel 0 = X channel 1 = Y */

	LONG *pos;		/* 1 unscaled position for each channel */
	LONG *min;		/* 1 for each channel */
	LONG *max;		/* 1 for each channel */
	LONG *clipmax;  /* 1 for each channel */
	LONG *aspect;   /* 1 for each channel or NULL. */
	UBYTE *flags;   /* a set of 8 flags for every channel */
	} Idriver;

/* channel flags */

#define RELATIVE 0x0001
#define PRESSURE 0x0002

typedef struct idr_library
	{
	Errcode (*detect)(struct idriver *);
	Errcode (*inquire)(struct idriver *);
	Errcode (*input)(struct idriver *);
	Errcode	(*setclip)(struct idriver *idr,short channel,long clipmax);
	} Idr_library;


Errcode load_idriver(Idriver **pidr, char *iname,UBYTE *modes,SHORT comm_port);
void close_idriver(Idriver **idr);
void idr_clip(Idriver *idr,SHORT first_channel, SHORT last_channel);

extern char key_idriver_name[];
extern char mouse_idriver_name[];
extern char summa_idriver_name[];

extern Errcode init_key_idriver(Idriver *idr);
extern Errcode init_mouse_idriver(Idriver *idr);

extern Errcode
config_idriver(char *name, UBYTE *modes, Boolean use_modes, SHORT port);

#endif /* IDRIVER_H */
