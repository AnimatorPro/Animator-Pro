@echo off
rem Hacked copy of MAKEKIT.BAT as we don't have the necessary utilities.

rem pushdir
rem call make
rem if ERRORLEVEL 1 goto out
rem popdir

rem *** rexlib include files ******************

copy \paa\rexlib\inc\compiler.h inc
copy \paa\rexlib\inc\pjinline.h inc
copy \paa\rexlib\inc\raster.i inc
copy \paa\rexlib\inc\raster.h inc
copy \paa\rexlib\inc\rastlib.i inc
copy \paa\rexlib\inc\rastlib.h inc
copy \paa\rexlib\inc\stdtypes.h inc
copy \paa\rexlib\inc\stdmacro.i inc
copy \paa\rexlib\inc\syslib.h inc
copy \paa\rexlib\inc\rexlib.h inc
copy \paa\rexlib\inc\filemode.h inc
copy \paa\rexlib\inc\filepath.h inc
copy \paa\rexlib\inc\jfile.h inc
copy \paa\rexlib\inc\stdio.h inc
copy \paa\rexlib\inc\stddef.h inc

rem **** include file from pj includes with cmacro processing ****

echo.
echo SKIPPED: cmacro \paa\rexlib\inc\vdevice.h vdevice.mac -o inc\vdevice.h
echo SKIPPED: cmacro \paa\rexlib\inc\vdevinfo.h vdevice.mac -o inc\vdevinfo.h

rem **** include file from pj includes with cmacro processing ****

echo SKIPPED: cmacro \paa\inc\errcodes.h errcodes.mac -o inc\errcodes.h -b
echo SKIPPED: cmacro \paa\inc\errcodes.i errcodes.mac -o inc\errcodes.i -b
echo.

rem *** get rexlib libraries

copy \paa\rexlib\aasyslib.lib lib\
copy \paa\rexlib\stdiolib.lib lib\
copy \paa\rexlib\rexentry.obj lib

rem *** get rexlib source for loader interface

copy \paa\rexlib\rexhost\rexentry.asm lib\loader\
copy \paa\rexlib\rexhost\rexentry.i lib\loader

rem *** get aasyslib source


:out

@echo on
