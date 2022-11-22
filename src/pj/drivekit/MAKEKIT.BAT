@echo off

goto end_comment

**************************************************************************
We process some files through a utility that does macro preprocessing on
files and some are just copied

USAGE: cmacro source_file(s) symbol_file
  options: -b (no backup files)
		   -o output_file (only if source_file isn't a wildcard)
		   -a (append to output file if output file specified and valid)
		   -w (include leading white space with substitution args
			   in source file when expanding macros)

**************************************************************************

:end_comment

pushdir
cd \paa\inc
call make
if ERRORLEVEL 1 goto out
popdir

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

cmacro \paa\rexlib\inc\vdevice.h vdevice.mac -o inc\vdevice.h
cmacro \paa\rexlib\inc\vdevinfo.h vdevice.mac -o inc\vdevinfo.h

rem **** include file from pj includes with cmacro processing ****

cmacro \paa\inc\errcodes.h errcodes.mac -o inc\errcodes.h -b
cmacro \paa\inc\errcodes.i errcodes.mac -o inc\errcodes.i -b


rem *** get rexlib libraries

xcopy \paa\rexlib\aasyslib.lib lib\
xcopy \paa\rexlib\stdiolib.lib lib\
copy \paa\rexlib\rexentry.obj lib

rem *** get rexlib source for loader interface

xcopy \paa\rexlib\rexhost\rexentry.asm lib\loader\
copy \paa\rexlib\rexhost\rexentry.i lib\loader

rem *** get aasyslib source


:out

@echo on
