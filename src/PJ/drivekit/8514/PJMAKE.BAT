@echo Edit 'pjmake.bat' to suit your system.
@echo off
rem
rem
rem   This batch file should be edited so that it:
rem
rem   1.  saves the path
rem   2.  creates a new path pointing to dos and development programs
rem   3.  sets development environment variables
rem   4.  creates the proper 'make.inc'
rem   5.  does the 'make'
rem   6.  restores the path
rem
rem   Below are examples WATCOM and HIGH C, both using the Opus MAKE.EXE.
rem
rem   NOTE:  if you rename this file to 'make.bat', be sure to specify the
rem 	 full path name of 'make.exe' if you want to use the make utility;
rem 	 otherwise, the system may recurse on 'make.bat'.
rem
set savepath=%path%
rem
rem
rem  *****	SELECT WHETHER YOU WANT WATCOM OR HIGH C  *****
rem


set ipath=..\inc;
nmake.exe -f hcmake
