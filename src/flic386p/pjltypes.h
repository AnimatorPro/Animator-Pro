/*****************************************************************************
 * PJLTYPES_H - Datatypes specific to the fliclib routines.
 *
 *	Items in here are specific to the interface between the pj internals and
 *	the fliclib routines visible to the clients of the fliclib.
 *
 *	This file contains the datatypes most frequently used with FlicLib.
 *	Other datatypes which are less frequently used can be found in:
 *
 *		PJCUSTOM.H - For working with custom raster types.
 *		PJFLI.H    - For directly accessing the data in a flic file.
 *
 *
 *	Note:  The PJSTYPES.H or STDTYPES.H file must be included before this
 *		   this file.  Don't code an include for it below, or the glue code
 *		   will break!
 ****************************************************************************/

#ifndef PJLTYPES_H
#define PJLTYPES_H

/*----------------------------------------------------------------------------
 * The PjRgb and PjCmap types...
 *	These types are used to store the color palette associated with a raster.
 *--------------------------------------------------------------------------*/

typedef struct pj_rgb {
	unsigned char r,g,b;
	} PjRgb;

typedef struct pj_cmap {
	long	num_colors; 	/* this will always by 256 in the fliclib */
	PjRgb	ctab[256];
	} PjCmap;

/*----------------------------------------------------------------------------
 * The FlicRaster type...
 *	This type describes a raster used for input or output of flic frames.
 *	Often, the raster will describe the physical video screen, but it can
 *	also describe a RAM-based raster, or a virtual raster which is a clipping
 *	window onto a physical raster.	For the most part, the client code
 *	will be dealing with pointers to these, and need not be concerned about
 *	the contents of the structure.
 *--------------------------------------------------------------------------*/

typedef struct flic_raster {
	short			type;			/* standard raster header fields... 	*/
	short			pdepth;
	struct rastlib	*lib;
	short			aspect_dx;
	short			aspect_dy;
	short			reserved[2];
	unsigned short	width;
	unsigned short	height;
	short			x;
	short			y;
	union { 						/* Raster-type-specific data... 		*/
		void			*custom_data[4];	/* Custom raster internal data. */
		unsigned long	bytemap_data[4];	/* Ram raster internal data.	*/
		unsigned long	video_data[4];		/* Video raster internal data.	*/
		}			hw;
	PjCmap			*cmap;			/* all fliclib rasters have a color map */
	} FlicRaster;

/*----------------------------------------------------------------------------
 * The Flic type...
 *
 *	This datatype is used as the primary 'handle' on a flic during creation
 *	and low-level playback.  The fields are used as follows:
 *
 *	  userdata - Anything the client application wants. Not touched internally.
 *	  flifile  - A standard Animator Flifile structure.  This structure is
 *				 documented, but it is not mapped out in detail here, because
 *				 detail-level access should not be needed often.  In most
 *				 cases, the client application can treat this as a pointer
 *				 to a blackbox structure.  Full mapping is in PJFLI.H.
 *				 The flilib routines will allocate a Flifile and attach it
 *				 to the Flic structure via this field.
 *	  libctl   - This is a pointer to a blackbox structure that must not be
 *				 touched by the client application.  The flilib code uses
 *				 the contents of the blackbox internally to manage creation
 *				 and playback of flic files via the higher-level routines.
 *--------------------------------------------------------------------------*/

typedef struct flic_struct {
	void					*userdata;
	struct flifile			*flifile;
	struct flilib_control	*libctl;
	} Flic;

/*----------------------------------------------------------------------------
 * The FlicPlayOptions type...
 *	When using the high-level playback functions, this (optional) structure can
 *	be filled with data to override the defaults supplied by the system and the
 *	information in the flic file.  The fields are used as follows:
 *
 *		speed			 - The playback speed, in milliseconds-per-frame.  If
 *						   negative, the speed from the flic file is used.
 *
 *		x,y 			 - The playback offsets.  If these values are both
 *						   zero, the playback is centered on the output
 *						   raster, and clipped if the size of the flic
 *						   exceeds the size of the output raster.  If these
 *						   values are non-zero, they are used as the x/y
 *						   coordinates of the upper left corner of the flic
 *						   on the output raster.  If any portion of the
 *						   playback would be offscreen, clipped output
 *						   routines are used, and performance suffers
 *						   somewhat.  Negative values are allowed.
 *
 *		keyhit_stops_playback -
 *						   If TRUE, hitting any key on the keyboard will stop
 *						   the playback, in addition to any termination
 *						   condition imposed by the event detection routine.
 *						   When TRUE, the user's event detector is called
 *						   before the keyboard is checked, and is still
 *						   guaranteed to be called at least once per frame.
 *
 *		*playback_raster - This raster will be used for flic playback.	If
 *						   this pointer is NULL, a video screen raster will
 *						   be opened automatically, and will be closed upon
 *						   completion of the playback.
 *--------------------------------------------------------------------------*/

typedef struct flic_play_options {
	long		speed;
	long		x;
	long		y;
	Boolean 	keyhit_stops_playback;
	FlicRaster	*playback_raster;
	long		reserved[4];			/* for future expansion */
	} FlicPlayOptions;

/*----------------------------------------------------------------------------
 * The AnimInfo type...
 *	This structure holds all the interesting facts about a flic.  It is
 *	somewhat easier to access than the Flifile structure, but contains many
 *	of the same values.
 *--------------------------------------------------------------------------*/

#define DEFAULT_AINFO_SPEED 71

typedef struct animinfo {
	unsigned short	width, height;
	short			x, y;
	long			num_frames;
	short			depth;
	unsigned short	millisec_per_frame;
	unsigned short	aspect_dx;
	unsigned short	aspect_dy;
	unsigned char	reserved[44];
	} AnimInfo;

/*----------------------------------------------------------------------------
 * UserEventData and UserEventFunc types...
 *	These datatypes are used to implement an event-detection routine used
 *	with pj_flic_play_until(), which will play a flic continuously until
 *	the client-specified event detection function returns FALSE.
 *--------------------------------------------------------------------------*/

typedef struct user_event_data {
	Flic	*flic;
	void	*userdata;
	long	 cur_loop;
	long	 cur_frame;
	long	 num_frames;
	} UserEventData;

typedef Boolean (UserEventFunc)(UserEventData *pdata);

/*----------------------------------------------------------------------------
 * The LocalVdevice type...
 *	From the client's point of view, this is just a strongly-typed handle
 *	to a blackbox structure which holds info about video devices.
 *--------------------------------------------------------------------------*/

typedef struct local_vdevice LocalVdevice;

#endif /* PJLTYPES_H */
