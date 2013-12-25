@echo off

	set -r=
	set -foo=
	set -all=
	set -opath=%path%
	set -d=

rem ***** check which version we want to make, default to test version ****

	if "%1"=="release" goto usa_release
	if "%1"=="english" goto euro_release

:test_version
	echo Making test version.....
	\build\bin\cmpfiles >nul inc\reltest.h inc\release.h
	if not ERRORLEVEL 1 goto make_test

	echo .
	choice "Warning! configuring for TEST version. Continue?" y
	if not ERRORLEVEL 1 goto out
	copy >nul inc\reltest.h inc\release.h
	touch >nul inc\release.h
	goto make_test

:usa_release
	echo Making USA release version.....
	set -foo=inc\relusa.h
	goto setup_release

:euro_release
	echo Making European english release version.....
	set -foo=inc\releuro.h

:setup_release

	shift
	if "%1"=="" goto showhelp
	set -r=%1
	shift
	set -d=%1
	shift
	if not "%1"=="" goto showhelp

	cmpfiles >nul %-foo% inc\release.h
	if not ERRORLEVEL 1 goto make_release

	echo .
	choice "Warning! using %-foo% Continue?" y
	if not ERRORLEVEL 1 goto out
	copy >nul %-foo% inc\release.h
	touch >nul inc\release.h

:make_release

	set -foo=
	if exist status.obj del status.obj
	if exist player\playpull.obj del player\playpull.obj
	if exist convert\convpull.obj del convert\convpull.obj
	goto do_all

:make_test
	call setwc386
	if "%1"=="tags" goto tags
	if "%1"=="link" goto link
	if "%1"=="all" goto do_all
	if "%1"=="root" goto root
	if "%1"=="rl" goto rootl;
	goto default_make

:do_all
	call setwc386
	set -all=all
	shift

:default_make
	echo >inc\make.inc !include "\paa\inc\makewc.inc"
	echo >make.inc !include "\paa\inc\makewc.inc"
	cd inc
	tcmake ilib
	cd ..
	wmake %1 %2 %3 %4 %5 %6 %7 %8 %9
	if ERRORLEVEL 1 goto error
	if not "%1"=="" goto out
	cd rastlib
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd util
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd fileio
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd fli
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd font
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd gfx
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd input
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd menus
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd pjhigh
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd softmenu
	tcmake objlib
	if ERRORLEVEL 1 goto error
	cd ..
	cd wndo
	tcmake libs
	if ERRORLEVEL 1 goto error
	cd ..
	cd poco
	call make watcom objlib
	if ERRORLEVEL 1 goto error
	cd ..
	cd rexlib
	tcmake host
	if ERRORLEVEL 1 goto error
	cd ..
	cd picdrive
	call make host
	if ERRORLEVEL 1 goto error
	cd ..

if not "%-all%"=="all" goto link_release

rem The makes below here are dependent on having successfully made what is
rem above here the include directory and the object modules etc.

	cd rexlib
	tcmake rex
	if ERRORLEVEL 1 goto error
rem	cd ..\inks
rem	tcmake
rem	if ERRORLEVEL 1 goto error
	cd ..\drivekit
	call makekit_
	if ERRORLEVEL 1 goto error
	call make
	if ERRORLEVEL 1 goto error
	cd ..\picdrive
	call make rex
	if ERRORLEVEL 1 goto error
	cd ..\input\loadable
	call make
	if ERRORLEVEL 1 goto error
	cd ..\..
rem	cd flic386p\libsrc
rem	call makeall
rem	cd ..\..
rem	cd poco\poekit
rem	call make
rem	cd ..\..

rem Here we do the player and convert program.
	cd player
	call make
	if ERRORLEVEL 1 goto error
	cd ..
	cd convert
	call make
	if ERRORLEVEL 1 goto error
	cd ..

:link_release

	wlinkp @v.lnk
	if ERRORLEVEL 1 goto error

rem make up a release directory if we want a release

:call_release
	if "%-r%"=="" goto out
	call release %-r% %-d%
	if ERRORLEVEL 1 goto error
	goto out

:root
	tcmake %2 %3 %4 %5 %6 %7 %8 %9
	goto out

:rootl
	tcmake %2 %3 %4 %5 %6 %7 %8 %9
	if ERRORLEVEL 1 goto error
	goto l

:link
	wlinkp @v.lnk >foo
	type foo
	if ERRORLEVEL 1 goto error
	goto out

:tags
	jtags \paa\*.c
	jtags \paa\fileio\*.c \paa\fileio\*.asm -a
	jtags \paa\gfx\*.c \paa\gfx\*.asm -a
	jtags \paa\inc\*.h -a
	jtags \paa\input\*.c -a
	jtags \paa\menus\*.c -a
	jtags \paa\picdrive\host\*.c -a
	jtags \paa\pjhigh\*.c -a
	jtags \paa\poco\pocoface.c \paa\poco\safefile.c \paa\poco\pocolib.c -a
	jtags \paa\rastlib\*.c \paa\rastlib\*.asm -a
	jtags \paa\rexlib\rexhost\*.c -a
	jtags \paa\softmenu\*.c -a
	jtags \paa\util\*.c \paa\util\*.asm -a
	jtags \paa\wndo\*.c \paa\wndo\*.asm -a
	jtags \paa\fli\*.c \paa\fli\*.asm -a
	jtags \paa\font\*.c -a
	copy tags \tgs
	goto out

:showhelp
	echo Usage: "make release version_number drive:
	echo	version_number is a number between 0 and 999
	echo	it will create a directory
	echo	drive:\pjVNUM with final exe files, resource etc.
	echo	example:
	echo		make release 101 i:
	goto out

:error
	echo Error: make failed !!!!
	beep 1

:out
	set -foo=
	set -r=
	set -all=
	beep 1
	path=%-opath%
	set -opath=
	@echo on
