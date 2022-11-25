@echo off

	call makekit no_distribution_file

rem *************************************************************************
rem * the following subdirs are parts of the poekit that gets distributed...
rem *  Note that poepatch MUST be made first, others depend on it.
rem *************************************************************************

	cd poepatch
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd colorutl
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd lookup
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd otdemo
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd pdrdemo
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd pstamp
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

rem *************************************************************************
rem * the following subdirs are our own private things...
rem *************************************************************************

	cd pdracc
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd eco
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

	cd vscreen
	wmake
	if ERRORLEVEL 1 goto error
	cd ..

rem cd cycle
rem wmake
rem if ERRORLEVEL 1 goto error
rem cd ..

rem *************************************************************************
rem * copy bits and pieces to the resource and root dirs, as needed...
rem *************************************************************************

	copy pstamp\pstamp.poe		\paa\resource
	copy pstamp\flisumry.poc	\paa\resource

	copy pdracc\pdracces.poe	\paa\resource

	copy lookup\lookup.poe		\paa\resource
	copy lookup\lookup.poc		\paa\resource\lookup.po

	goto out

:error

	echo Error: make failed !!!!
	beep 1

:out

