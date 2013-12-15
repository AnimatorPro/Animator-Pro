/*---------------------------------------------------------------------*\

    DEVICE.C

 Device specific module for Animator/386 TIGA Driver.

 Copyright (C) 1989, 1990  Panacea Inc.

 Panacea Inc.
 50 Nashua Road, Suite 305
 Londonderry, New Hampshire, 03053-3444
 (603) 437-5022

 NOTICE:
   This code may be freely distributed and modified as long as the
   Panacea copyright notice above is maintained intact.
------------------------------------------------------------------------

Revision History:

Date     Who   What
======== ===   =======================================================
12/14/90 JBR   Initial Version
01/03/91 JBR   Converted over to PJ.48

\*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------

   INCLUDES

-----------------------------------------------------------------------*/
#include <stdio.h>
#include "tiga.h"
#include "errcodes.h"
#include "tigahc.h"
#include "extend.h"
#include "typedefs.h"


/*-----------------------------------------------------------------------

   DEFINES

-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------

   STRUCTURES

-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------

   FUNCTION PROTOTYPE DECLARATIONS

-----------------------------------------------------------------------*/
struct rastlib *tiga_get_rlib(Vdevice *dev, int mode, tigaRast *r);
Errcode        tiga_close_graphics(Vdevice *driver);
Errcode        tiga_detect(Vdevice *dev);
Errcode        tiga_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm);
char           *tiga_mode_text(Vdevice *driver, USHORT mode);
Errcode        tiga_open_graphics(Vdevice *dev, tigaRast *r,
                                  LONG width, LONG height, USHORT mode);
Errcode        InitVInfo();

/*-----------------------------------------------------------------------

   EXTERNALS

-----------------------------------------------------------------------*/
extern   void  PrintText(char *string);

/*-----------------------------------------------------------------------

   STATIC VARIABLES

-----------------------------------------------------------------------*/
static Vmode_info tiga_info =
   {
   sizeof(Vmode_info),
   0, /* mode ix */
   "Panacea TIGA Driver",
   8, /* bits */
   1, /* planes */
   {1024,1024,1024,1},  /* fixed width */
   {768,768,768,1},  /* fixed height */
   TRUE, TRUE,    /* read and write */
   TRUE,       /* Has viewable screen suitable for menus etc */
   1,             /* fields_per_frame */
   1,          /* display pages */
   1,          /* store pages */
   1024*768,    /* display bytes */
   1024*768,    /* storage bytes */
   FALSE,         /* Palette set's only allowed during vblank */
   0,          /* Swap screen during vsync. (We can't swap screens even)*/
   60,            /* Vsync rate */
   };

static char *mtext =
 "Panacea TIGA Driver.\
  \nCopyright (c) 1989,1990 Panacea Inc.\
  \nAll Rights Reserved.";

static USHORT  tigaOrigMode;              /* Tells us what to go back to...*/
static UBYTE   modeTable[20];
static UBYTE   numValidModes = 0;
static UBYTE   haveVInfo = FALSE;

/*-----------------------------------------------------------------------

   UNINITIALIZED GLOBAL VARIABLES

-----------------------------------------------------------------------*/

CONFIG      tigaConfig;                   /* Current display mode info.    */
MODEINFO    tigaModeInfo;
ULONG       dispPitch;                    /* Display pitch for cur. mode   */
ULONG       dispAddr;                     /* TI Address of display         */
ULONG       buffAddr;                     /* Address of transfer buffer    */
ULONG       buffSize;                     /* Size of buffer                */
SHORT       moduleNum;                    /* Used by the RLM function calls*/

/*-----------------------------------------------------------------------

   INITIALIZED GLOBAL VARIABLES

-----------------------------------------------------------------------*/

struct vdevice_lib tiga_device_library =
   {
   tiga_detect,         /* detect - Is our hardware attatched? */
   tiga_get_modes,      /* get modes */
   tiga_mode_text,      /* get extra info */
   NOFUNC,              /* set max height for width.  */
   tiga_open_graphics,  /* open_graphics */
   tiga_close_graphics, /* close_graphics */
   NOFUNC,              /* open_cel */
   NOFUNC,              /* show_rast */
   };

Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION};
Hostlib _a_a_syslib = { &_a_a_stdiolib, AA_SYSLIB, AA_SYSLIB_VERSION };

Vdevice rexlib_header =
   {
      { REX_VDRIVER, 0, InitVInfo, NOFUNC, &_a_a_syslib},
      0,                              /* first_rtype */
      0, /* num_rtypes */
      0, /* mode_count */
      sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
      &tiga_device_library,         /* lib */
      NULL,          /* grclib */
      NUM_LIB_CALLS,    /* rast_lib_count */
   };

UBYTE    tigaIsOpen = FALSE;              /* Local TIGA Flag...            */
UBYTE    tigaDisplay = FALSE;             /* Indicates TIGA display stat.  */

/************************************************************************

   ACTUAL CODE

************************************************************************/

/*-----------------------------------------------------------------------

   PrintXXXXX()

   These functions are used to display information on an IBM compatible
   text display while debugging this driver. This is possible since
   most TIGA boards use something akin to a VGA pass-through. The
   function these routines call (PrintText), is located in OUTPUT.ASM.

-----------------------------------------------------------------------*/
void  PrintHex(USHORT num)
{
   char  string[8];

   sprintf(string, "%0.4X", num);
   PrintText(string);

   return;
}

void  PrintDec(USHORT num)
{
   char  string[8];

   sprintf(string, "%d", num);
   PrintText(string);

   return;
}

void  PrintLHex(ULONG num)
{
   char  string[10];

   sprintf(string, "%0.8X", num);
   PrintText(string);

   return;
}

void  PrintLDec(ULONG num)
{
   char  string[16];

   sprintf(string, "%ld", num);
   PrintText(string);

   return;
}

void  PrintVar(char *string, USHORT num)
{
   PrintText(string);
   PrintText(" =  ");
   PrintDec(num);
   PrintText(" (0x");
   PrintHex(num);
   PrintText(")\r\n");
   return;
}

void  PrintLVar(char *string, ULONG num)
{
   PrintText(string);
   PrintText(" =  ");
   PrintLDec(num);
   PrintText(" (0x");
   PrintLHex(num);
   PrintText(")\r\n");
   return;
}

void  PrintHexDump(UBYTE *dataArea, USHORT numBytes)
{
   char     string[6];
   USHORT   i, j;
   UBYTE    val;

   while (numBytes)
      {
      PrintLHex((ULONG)dataArea);
      PrintText(" -->  ");
      j = 16;
      while (j--)
         {
         if (!(numBytes--))
            break;
         val = *dataArea++;
         sprintf(string, "%0.2X", val);
         PrintText(string);
         PrintText(" ");
         if (j == 8)
            PrintText(" ");
         }
      PrintText("\r\n");
      }
   PrintText(")\r\n");
   return;
}

/*-----------------------------------------------------------------------

   TI Debugging routines, used since SDB can't work with protected mode.

-----------------------------------------------------------------------*/
void SetTIDeb()
{
   ULONG    temp = 0L;
                                          /* Read status                   */
   host2gsp(&temp, 0xFFFFEFE0L, 4, 0);
   return;
}


ULONG GetTIDeb(USHORT index)
{
   ULONG    retVal = 0L;
                                          /* Read status                   */
   gsp2host(0xFFFFF000L + index * 0x20, &retVal, 4, 0);
   return(retVal);
}


/*-----------------------------------------------------------------------

   tiga_detect
   
   This function checks if TIGA is installed and available for use.
   It returns an error code (Err_no_display) if not, and a success
   code if it is there.

-----------------------------------------------------------------------*/
Errcode tiga_detect(Vdevice *dev)
{
#ifdef DEBUG
   printf("tiga_detect()\n");
#endif

   if (tigaIsOpen)
      return(Success);                    /* If we opened, then it's there */

   if (tiga_set(CD_OPEN) < 0L)            /* Try to open TIGA CD           */
      return(Err_no_display);             /* Failed.                       */
   else
      {
      tigaIsOpen = TRUE;                  /* TIGA is now open for business */
      return(Success);                    /* Success!                      */
      }

}

/*-----------------------------------------------------------------------

   tiga_get_modes
   
   Sets a pointer to the requested mode structure. If the requested
   mode does not exist, then an error is returned.

-----------------------------------------------------------------------*/
static Errcode tiga_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
   USHORT   actualMode, x, y;
   Errcode  err;

#ifdef DEBUG
   printf("tiga_get_modes(mode = %d)\n", mode);
#endif

   if (mode >= numValidModes)
      return(Err_no_such_mode);           /* Mode number is too high       */

   actualMode = modeTable[mode];

   if ((err = tiga_detect(driver)) != Success)
      return(err);

   get_modeinfo(actualMode, &tigaModeInfo);

   x = tigaModeInfo.disp_hres;
   y = tigaModeInfo.disp_vres;

   tiga_info.mode_ix = mode;
   tiga_info.width.min = x;
   tiga_info.width.max = x;
   tiga_info.width.actual = x;
   tiga_info.height.min = y;
   tiga_info.height.max = y;
   tiga_info.height.actual = y;
   tiga_info.display_bytes = x * y;
   tiga_info.store_bytes = x * y;

   memcpy(pvm, &tiga_info, sizeof(Vmode_info));

   return(Success);
}

/*-----------------------------------------------------------------------

   tiga_mode_text

   Returns a pointer to the text string that explains the selected mode.
   If the mode does not exist, a NULL is returned.

-----------------------------------------------------------------------*/
static char *tiga_mode_text(Vdevice *driver, USHORT mode)
{
#ifdef DEBUG
   printf("tiga_get_text(mode = %d)\n", mode);
#endif

   if (mode >= numValidModes)
      return(NULL);

   return(mtext);
}

/*-----------------------------------------------------------------------

   tiga_open_graphics
   

-----------------------------------------------------------------------*/
Errcode tiga_open_graphics(Vdevice *dev, tigaRast *r, 
                     LONG width, LONG height, USHORT mode)
{
   USHORT   actualMode, x, y;
   Errcode  err;

#ifdef DEBUG
   printf("tiga_open_graphics(width=%d, height=%d, mode=%d)\n",
           width, height, mode);
#endif

   if (mode >= numValidModes)
      return(Err_no_such_mode);           /* See if valid mode number      */

   if ((err = tiga_detect(dev)) == Success)
      {                                   /* Is the board present?         */
      r->lib = tiga_get_rlib(dev, mode, r);
      r->type = mode + dev->first_rtype;
      actualMode = modeTable[mode];
      set_config(actualMode, TRUE);
      get_modeinfo(actualMode, &tigaModeInfo);
      x = tigaModeInfo.disp_hres;
      y = tigaModeInfo.disp_vres;
      if (width != x || height != y)
         return(Err_wrong_res);
      r->width = width;
      r->height = height;
      r->pdepth = 8;
      r->aspect_dx = 3;
      r->aspect_dy = 2;
      r->hw.nm.dev = dev;
      r->hw.nm.puts = r->hw.nm.gets = r->hw.nm.colors = 0;

      /*
         Now we actually set TIGA into the proper display mode (if it
         hasn't already been done).
      */
      if (!tigaIsOpen)
         {
         if (tiga_set(CD_OPEN) < 0L)      /* Try to open TIGA CD           */
            return(Err_no_display);       /* Failed.                       */
         tigaIsOpen = TRUE;
         }
      if (!tigaDisplay)                   /* Already in TIGA mode?         */
         {

#ifdef DEBUG
         printf("Setting TIGA Video Mode\n");
#endif

         tigaOrigMode = get_videomode();
         if (!set_videomode(TIGA, INIT | CLR_SCREEN))
            return(Err_driver_protocol);  /* If error opening...           */

#ifdef DEBUG
         printf("TIGA Video Mode is set\n");
#endif

         tigaDisplay = TRUE;
         }

      if (install_primitives() < 0)       /* We need the primitives...     */
         return(Err_driver_protocol);     /* Couldn't install them!!!      */

#ifdef DEBUG
      printf("Primitives loaded\n");
#endif

      get_config(&tigaConfig);            /* Get current config info.      */
      dispPitch = tigaConfig.mode.disp_pitch;  /* Get current display pitch*/
      dispAddr = (ULONG)peek_breg(4);     /* Determine address in TI mem   */

      buffSize = 65535L;                  /* Start with 64K-1              */
      while (buffSize > 0L)               /* Try to allocate memory...     */
         {
         buffAddr = gsp_malloc(buffSize);
         if (buffAddr == NULL)
            buffSize -= 1024;
         else
            break;
         }
      if (!buffSize)                      /* If no board memory!!!         */
         return(Err_driver_protocol);
      else
         {
         moduleNum = install_rlm("AAEXT.RLM");
         if (moduleNum < 0)
            return(Err_driver_protocol);

         SetTIBuffAddr(buffAddr, buffSize);

         return(Success);
         }
      }
   return(err);
}

/*-----------------------------------------------------------------------

   tiga_close_graphics
   

-----------------------------------------------------------------------*/
static Errcode tiga_close_graphics(Vdevice *driver)
{
#ifdef DEBUG
   printf("tiga_close_graphics()\n");
#endif

   if (tigaDisplay)
      {
      gsp_free(buffAddr);
      buffSize = 0L;
      set_videomode(tigaOrigMode, INIT);  /* Switch to normal text screen  */
      }
   if (tigaIsOpen)
      tiga_set(CD_CLOSE);                 /* Close TIGA CD.                */
   tigaDisplay = tigaIsOpen = FALSE;      /* Change state.                 */
   return(Success);
}

/*-----------------------------------------------------------------------

   InitVInfo()
   
   Looks through the TIGA config info to find all 8bpp modes.

-----------------------------------------------------------------------*/
static Errcode InitVInfo()
{
   USHORT   i;
                                          /* Let's check if TIGA is there  */
   if (tiga_detect(&rexlib_header) == Success)
      {                                   /* It is, so lets get config info*/
      get_config(&tigaConfig);            /* Get current config info.      */
         
      for (i = 0; i < tigaConfig.num_modes; i++)
         {
         if (!(get_modeinfo(i, &tigaModeInfo)))
            return(Err_no_display);
         if (tigaModeInfo.disp_psize == 8)
            {
            modeTable[numValidModes] = i;
            numValidModes++;
            }
         }
      rexlib_header.num_rtypes = numValidModes;
      rexlib_header.mode_count = numValidModes;
      haveVInfo = TRUE;
      return(Success);
      }
   else
      {
      rexlib_header.num_rtypes = 0;
      rexlib_header.mode_count = 0;
      haveVInfo = TRUE;
      return(Err_no_display);
      }
}

