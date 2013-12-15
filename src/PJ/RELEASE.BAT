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
	wstrip pj.exp
  	bind386 \build\bin\run386b pj.exp -exe %d%ani.exe
	if ERRORLEVEL 1 goto error 
	wstrip convert.exp
  	bind386 \build\bin\run386b convert.exp -exe %d%aniconv.exe
	if ERRORLEVEL 1 goto error 
	cfig386 %d%aniconv.exe -minreal 4096
	if ERRORLEVEL 1 goto error 
	wstrip aniplay.exp
  	bind386 \build\bin\run386b aniplay.exp -exe %d%aniplay.exe
	if ERRORLEVEL 1 goto error 
	cfig386 %d%aniplay.exe -minreal 4096
	if ERRORLEVEL 1 goto error 


:do_resource
	del resource\*.bak
	del resource\aaperm.glv
	copy resource\*.* %d%resource
	copy %d%resource\mcga.drv %d%resource\aadisp.drv
	del %d%resource\mcga.drv
	copy %d%resource\supervga.drv %d%vesa
	del %d%resource\supervga.drv
	del %d%resource\vram4.drv

:do_utils
	xcopy \images\flipaper %d%flipaper\ /s
	if ERRORLEVEL 1 goto error 
	xcopy \images\flitape %d%flitape\ /s
	if ERRORLEVEL 1 goto error 
	xcopy \images\pixelpop %d%pixelpop\ /s
	if ERRORLEVEL 1 goto error 
	xcopy \images\vesa %d%vesa\ /s
	if ERRORLEVEL 1 goto error 
	copy \build\bin\cfig386.exe %d%cfig386.exe

:do_newdx
	newdx %d%flipaper\flipaper.exe
	if ERRORLEVEL 1 goto error 
	newdx %d%flitape\flitape.exe
	if ERRORLEVEL 1 goto error 

:do_fonts
	xcopy \images\font %d%resource\font\
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
