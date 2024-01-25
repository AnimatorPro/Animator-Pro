/*    (c) Copyright 1987 by Everex Systems Inc.                             *
 *                                                                          *
 *              ***IMPORTANT NOTE****                                       *
 *                                                                          *
 *        This code is supplied to you for assisting in the development     *
 *  of software for Everex hardware products only.                          *
 *     You may distribute binary code that results from this source, but you*
 *  may not destribute this source to your end users.                       *
 *     You may not re-distribute this source to other software developers   *
 *  without express permission from Everex Systems Inc.                     *
 *                                                                          *
 */
/* FILE : img.h
    This is the header file for the image load and save functions*/


struct colormap
{
    WORD    maporg;         /*index of first map entry in LUT*/
    WORD    maplen;         /*num of elements to be loaded*/
    BYTE    mapbits;        /*number of bits in each element*/
};    

struct imgspec
{
    WORD    xorg;           /*x position of lower left corner of image*/
    WORD    yorg;           /*y position of lower left corner of image*/
    WORD    width;          /*width of image in pixels*/
    WORD    height;         /*height of image in pixels*/
    BYTE    pixsiz;         /*num of bits per pixel*/
    BYTE    imgdesc;        /*contains bit fields of descriptor info*/
};    

struct imgfile
{
    BYTE    idlength;       /*length of id field*/
    BYTE    maptype;        /*color map type*/
    BYTE    imgtype;        /*image storage type*/
    struct  colormap  cms;   /*color map specs.*/
    struct  imgspec imgdata;    /*image info*/
};

struct packet
{
    BYTE count;
    WORD data[128];
};

                /*masks for image descriptor field*/

#define ATTRIB  0x0f        /*mask for attribute bits*/
#define SCRORG  0x20        /*mask for screen origin*/
                            /*0 = upper left, 1 = lower left*/
#define INLEAVE  0xc0       /*data storage interleaving bits*/
                            /*00 = non-interleaved*/
                            /*01 = even-odd interleave*/
                            /*10 = 4 way interleave*/



