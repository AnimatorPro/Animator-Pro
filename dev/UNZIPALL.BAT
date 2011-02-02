@echo off

	if "%1"=="" goto helpout
	if "%2"=="" goto error
	if "%1"=="%2" goto error

	%1
if not exist *.bzp goto nobuild
	%2
	if ERRORLEVEL 1 goto error
	md %2\build
	if ERRORLEVEL 1 goto error 
	cd %2\build
	if ERRORLEVEL 1 goto error 
	%1pkunzip %1*.bzp -d
	if ERRORLEVEL 1 goto error 

:nobuild
	%1
if not exist *.izp goto noimages
	%2
	if ERRORLEVEL 1 goto error
	md %2\images
	if ERRORLEVEL 1 goto error 
	cd %2\images
	if ERRORLEVEL 1 goto error 
	%1pkunzip %1*.izp -d
	if ERRORLEVEL 1 goto error 

:noimages
	%2
	if ERRORLEVEL 1 goto error 
	md %2\paa
	if ERRORLEVEL 1 goto error 
	cd %2\paa
	if ERRORLEVEL 1 goto error 
	%1pkunzip %1*.zip -d
	if ERRORLEVEL 1 goto error 
	%1
	goto out

:error
	echo Error unzipping "%1" to "%2"
:helpout
	echo Usage: unzipall src_drive dest_drive
	echo        unzips all archive files on src_drive 
	echo        and creates directorys \paa, \build and \images on dst_drive
	echo .
	echo Example: unzipall i: d:

:out
	@echo on
