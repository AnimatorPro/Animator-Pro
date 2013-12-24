@echo off
	call setwc386
	echo >\paa\inc\make.inc !include "\paa\inc\makewc.inc"

	pushdir
	cd \paa\inc
	tcmake ilib
	if ERRORLEVEL 1 goto out
	popdir

	if "%1"=="rex" goto make_rex

:make_host

	cd rexhost
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..

	if "%1"=="host" goto nobeep

:make_rex

	cd inc
	tcmake
	if ERRORLEVEL 1 goto out

	echo >make.inc !include "\paa\rexlib\inc\makewc.inc"

	cd ..\syslib
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\stdiolib
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\gfxlib
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\mathlib
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..

:out
	beep 1
:nobeep

@echo on
