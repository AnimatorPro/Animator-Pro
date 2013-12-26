@echo off

	if "%1"=="highc" goto highc
	if "%1"=="watcom" shift

:watcom
	copy inc\makewc.inc inc\make.inc
	goto make_dirs
:highc
	shift
	copy inc\makehc.inc inc\make.inc

:make_dirs

rem making of drvcomn, vesa, and svga must be done in that order!

	cd drvcomn
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\vesa
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\svga
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\vram
	tcmake
	if ERRORLEVEL 1 goto out
rem	cd ..\8514
rem	tcmake
rem	if ERRORLEVEL 1 goto out
	cd ..\mcga
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\null
	tcmake
	if ERRORLEVEL 1 goto out
rem	cd ..\hgsc
rem	tcmake
rem	if ERRORLEVEL 1 goto out
rem	cd ..\compaq
rem	tcmake
	cd ..

:out

@echo
