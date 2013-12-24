@echo off
	call setwc386
	if "%1"=="link" goto link
	if "%1"=="convert" goto justmake
	cd \paa\inc
	tcmake ilib
	if ERRORLEVEL 1 goto error
rem *************
	cd \paa\pjhigh
	tcmake
	if ERRORLEVEL 1 goto error
rem *************
	cd \paa\softmenu
	tcmake
	if ERRORLEVEL 1 goto error
rem *************
	cd \paa\convert
:justmake
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
	if ERRORLEVEL 1 goto error
:link
	wlink @convert.lnk option map
	if ERRORLEVEL 1 goto error
	goto out
:error
	beep 3
:out
@echo on
