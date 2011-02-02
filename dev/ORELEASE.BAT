	@echo off
	if "%1"=="" goto help
	if "%2"=="" goto help
	if not "%3"=="" goto help
	rem drive is in %2,  release # in %1
	set d=%2\pj%1\

:make_dir_tree
	echo %d%
	if ERRORLEVEL 1 goto error 
	md %2\pj%1
	if ERRORLEVEL 1 goto error 
	md %d%tutorial
	if ERRORLEVEL 1 goto error 
	md %d%vesa
	if ERRORLEVEL 1 goto error 
	md %d%resource
	if ERRORLEVEL 1 goto error 
	md %d%resource\font
	if ERRORLEVEL 1 goto error 
	copy doc\pj.doc %d%readme.doc

:do_binding
  	bind386 \build\pharlap\exe\run386b pj.exp -exe %d%ani.exe
	if ERRORLEVEL 1 goto error 
	cfig386 %d%ani.exe -minreal 4096
	if ERRORLEVEL 1 goto error 
  	bind386 \build\pharlap\exe\run386b convert.exp -exe %d%aniconv.exe
	if ERRORLEVEL 1 goto error 
	cfig386 %d%aniconv.exe -minreal 4096
	if ERRORLEVEL 1 goto error 
  	bind386 \build\pharlap\exe\run386b aniplay.exp -exe %d%aniplay.exe
	if ERRORLEVEL 1 goto error 
	cfig386 %d%aniplay.exe -minreal 4096
	if ERRORLEVEL 1 goto error 


:do_resource
	copy resource\aa.mu %d%resource
	copy resource\*.fnt %d%resource
	copy resource\*.cur %d%resource
	copy resource\mcga.drv %d%resource\aadisp.drv
	copy resource\vesa.drv %d%resource
	copy resource\8514a.drv %d%resource
	copy resource\compaq.drv %d%resource
	copy resource\*.ink %d%resource
	copy resource\*.idr %d%resource
	copy resource\*.pdr %d%resource
	copy resource\cone.poc %d%resource
	copy resource\rays.poc %d%resource
	copy resource\starfiel.poc %d%resource
	copy resource\turtbush.poc %d%resource
	copy resource\numpic.poc %d%resource
	copy resource\pdracces.poe %d%resource
	copy resource\pocogblv.poe %d%resource
	copy resource\ctype.h %d%resource
	copy resource\errcodes.h %d%resource
	copy resource\limits.h %d%resource
	copy resource\optics.h %d%resource
	copy resource\stddef.h %d%resource

:do_utils
	xcopy \images\flipaper %d%flipaper\ /s
	if ERRORLEVEL 1 goto error 
	xcopy \images\flitape %d%flitape\ /s
	if ERRORLEVEL 1 goto error 
	xcopy \images\pixelpop %d%pixelpop\ /s
	if ERRORLEVEL 1 goto error 
	xcopy \images\vesa %d%vesa\ /s
	if ERRORLEVEL 1 goto error 

:do_newdx
	newdx %d%ani.exe
	if ERRORLEVEL 1 goto error 
	newdx %d%aniconv.exe
	if ERRORLEVEL 1 goto error 
	newdx %d%aniplay.exe
	if ERRORLEVEL 1 goto error 
	newdx %d%flipaper\flipaper.exe
	if ERRORLEVEL 1 goto error 
	newdx %d%flitape\flitape.exe
	if ERRORLEVEL 1 goto error 

:do_fonts
	xcopy \images\font %d%resource\font\
	if ERRORLEVEL 1 goto error 
	xcopy resource\font %d%resource\font\
	if ERRORLEVEL 1 goto error 

:do_art
	xcopy \images\tutorial %d%tutorial\ /s
	xcopy \images\art %d% /s

	goto out

:help
	echo usage:
	echo " 	release version drive:"
	echo example:
	echo "	release 101 i:"
	goto out

:error
	echo release aborted with errors
	goto out

:out
	set d=
	echo on
