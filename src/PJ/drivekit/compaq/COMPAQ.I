;  COMPAQ.I



;******************************************************************************
;									      *
;		   Copyright (C) 1991 by Autodesk, Inc. 		      *
;									      *
;	Permission to use, copy, modify, and distribute this software and     *
;	its documentation for the purpose of creating applications for	      *
;	AutoCAD, is hereby granted in accordance with the terms of the	      *
;	License Agreement accompanying this product.			      *
;									      *
;	Autodesk makes no warrantees, express or implied, as to the	      *
;	correctness of this code or any derivative works which incorporate    *
;	it.  Autodesk provides the code on an ''as-is'' basis and             *
;	explicitly disclaims any liability, express or implied, for	      *
;	errors, omissions, and other problems in the code, including	      *
;	consequential and incidental damages.				      *
;									      *
;	Use, duplication, or disclosure by the U.S.  Government is	      *
;	subject to restrictions set forth in FAR 52.227-19 (Commercial	      *
;	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)      *
;	(1) (ii) (Rights in Technical Data and Computer Software, as	      *
;	applicable.							      *
;									      *
;******************************************************************************

;   1/7/91  - jdb - put into ADI stream


_data		segment public dword use32 'DATA'
_data		ends

const		segment public dword use32 'DATA'
const		ends

_bss		segment public dword use32 'BSS'
_bss		ends

DGROUP		group	const,_data,_bss


_text	segment para public use32 'code'
_text	ends

CGROUP	group	_text


SC_INDEX equ 3c4h
ER_PAGE_SEL equ 0f9h
ER_BANK_SEL equ 0f6h
MISC_INPUT equ 3cch
MISC_OUTPUT equ 3c2h

XMAX equ 640
YMAX equ 400

PHAR_REAL_SEG equ 34h

MAXVESAMODES	equ 100 		; max number of video modes

infostruc	struc
vib_Signature	db  'VESA'
vib_Version	dw  0
vib_Ptr 	dd  0
vib_Cap 	db  4 dup (0)
vib_ModesPtr	dd  0
vib_TotalMem	dw  0
vib_filler	db  252 dup (?)
infostruc	ends

modestruc   struc
				    ; MANDATORY INFORMATION
ms_modeattr	dw  ?
ms_winAattr	db  ?
ms_winBattr	db  ?
ms_wingran	dw  ?		    ; window granularity (in KBytes)
ms_winsize	dw  ?		    ; window size (in Kbytes)
ms_winAseg	dw  ?		    ; window A start segment
ms_winBseg	dw  ?		    ; window B start segment
ms_winFuncPtr	dd  ?
ms_pitch	dw  ?		    ; bytes per scan line

				    ; EXTENDED INFORMATION
ms_xsize	dw  ?
ms_ysize	dw  ?
ms_xcharsize	db  ?
ms_ycharsize	db  ?
ms_numplanes	db  ?
ms_bitsperpixel db  ?
ms_numbanks	db  ?
ms_memmodel	db  ?
ms_banksize	db  ?
ms_numimages	db  ?
ms_reserved	db  1
ms_filler	db  242 dup (?)
modestruc   ends

args	equ	[ebp + 8]
