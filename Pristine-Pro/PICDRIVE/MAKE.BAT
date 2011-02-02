@echo off

rem *************************************************************************
rem * update inc\ and lib\ dirs, but don't make distribution files...
rem *************************************************************************

	call makekit no_distribution_file

rem *************************************************************************
rem * the following subdirs are parts of the pdrkit that gets distributed...
rem *************************************************************************

	if "%1" == "host" goto make_host

	cd flilo
	cd
	wmake
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd rif
	cd
	wmake
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd targa
	cd
	wmake
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

rem *************************************************************************
rem * the following subdirs are our own private things...
rem *************************************************************************

	cd anim
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd bmp
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd degas
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd gif
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd lbm
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd mac
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd mov
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd neo
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd pcx
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd pict2
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd rnd
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd seq
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd sld
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd tiff
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

	cd jpeg\lib
	cd
	make
	if ERRORLEVEL 1 goto error

	cd ..
	cd
	make
	if ERRORLEVEL 1 goto error
	copy *.pdr \paa\resource
	cd ..

:make_host

	cd host
	cd
	make
	if ERRORLEVEL 1 goto error
	cd ..

	goto out

:error

	echo Error: make failed !!!!
	beep 1

:out

