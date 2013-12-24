@echo off

	call setwc386

	if "%1"=="rex" goto make_rex

:make_host

	echo >\paa\inc\make.inc !include "\paa\inc\makewc.inc"
	pushdir
	cd \paa\inc
	tcmake ilib
	if ERRORLEVEL 1 goto out
	popdir
	cd host
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..

	if "%1"=="host" goto out

:make_rex

rem
rem **** rexlib side code uses include dir in rexlib\include made above
rem 	 One must have made the rexlib dir before these
rem
	echo >\paa\rexlib\inc\make.inc !include "\paa\rexlib\inc\makewc.inc"
	cd gif
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\flilo
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\pcx
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\mac
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\lbm
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\neo
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\degas
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\seq
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\rif
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\anim
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\sld
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\targa
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..\tiff
	tcmake
	if ERRORLEVEL 1 goto out
	cd ..

:out

@echo on
