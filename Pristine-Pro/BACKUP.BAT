@echo off
@verify on

	if "%1"=="" goto showhelp
	set DRV=%1
	set F=FALSE
	set OBJ=FALSE
	set BLD=FALSE
	set NOSRC=FALSE

:looparg
	shift
	if "%1"=="-f" goto setf
	if "%1"=="-obj" goto setobj
	if "%1"=="-build" goto setbld
	if "%1"=="-nosource" goto setnosrc
	if "%1"=="" goto breakarg
	goto showhelp

	:setobj
		set OBJ=TRUE
		goto looparg
	:setf
		set F=TRUE
		goto looparg
	:setbld
		set BLD=TRUE
		goto looparg
	:setnosrc
		set NOSRC=TRUE
		goto looparg
:breakarg

	if "%DRV%"=="a:" goto fdrive
	if "%DRV%"=="b:" goto fdrive
	if "%DRV%"=="i:" goto idrive
	goto makedir

:showhelp
	echo USAGE: backup drive: [-build] [-obj]] [-f] [-nosource]
	echo	 The -f option formats drive if it's i:
	echo	 The -obj option zips objects too.
	echo     The -build option backs up the build environment and artwork.
	echo     The -nosource option makes it NOT back up source.
	echo EXAMPLE: backup d:
	echo     This will create a backup directory full of zips.  
	echo     The directory name will be aaYYMMDD.HHX
	echo     where YY is the year, MM the month, DD the hour,  and
	echo     X a letter related to the minute somehow.
	echo     (Also creates a .bat file cdlast to get to that directory.)
	goto quitit

:fdrive
	echo Can't possibly do this to a floppy, forget it.
	goto quitit

:idrive
	if "%f%" == "FALSE" goto makedir
	set f=

:formatit
	echo Insert backup disk in drive i:. All data will be erased!
	choice "Continue ? [y/n] " y
	if not ERRORLEVEL 1 goto quitit
	echo >foo xxxxxxxx
	rep "xxxxxxxx" "y\ry\ry\ry\ry\ry" foo -b -s
	c:\rcd\rcd format <foo i:
	if ERRORLEVEL 3 goto quitit

:makedir
	datedir %DRV%\aa
	showdate >%DRV%backup.txt -p "ALL AA files as of " -d -t -s "."

:dosource
	if "%NOSRC%"=="TRUE" goto dobuild
	echo Backing up source.
	textmul driv1src.inc suffsrc.inc >driv1.lst
	pkzip -es -P %DRV%driv1 @driv1.lst
	if ERRORLEVEL 1 goto error
	textmul driv2src.inc suffsrc.inc >driv2.lst
	pkzip -es -P %DRV%driv2 @driv2.lst
	if ERRORLEVEL 1 goto error
	textmul driv3src.inc suffsrc.inc >driv3.lst
	pkzip -es -P %DRV%driv3 @driv3.lst
	if ERRORLEVEL 1 goto error
	textmul rootsrc.inc suffsrc.inc >root.lst
	pkzip -es -P %DRV%root @root.lst
	if ERRORLEVEL 1 goto error
	textmul incsrc.inc suffsrc.inc >inc.lst
	pkzip -es -P %DRV%inc @inc.lst
	if ERRORLEVEL 1 goto error
	textmul misc1src.inc suffsrc.inc >misc1.lst
	pkzip -es -P %DRV%misc1 @misc1.lst
	if ERRORLEVEL 1 goto error
	textmul misc2src.inc suffsrc.inc >misc2.lst
	pkzip -es -P %DRV%misc2 @misc2.lst
	if ERRORLEVEL 1 goto error
	textmul flicsrc.inc suffsrc.inc >flicsrc.lst
	pkzip -es -P %DRV%flic386p @flicsrc.lst
	if ERRORLEVEL 1 goto error
	textmul pocosrc.inc suffsrc.inc >poco.lst
	pkzip -es -P %DRV%poco @poco.lst
	if ERRORLEVEL 1 goto error
	textmul poesrc.inc suffsrc.inc >poe.lst
	pkzip -es -P %DRV%poe @poe.lst
	if ERRORLEVEL 1 goto error
	textmul flic386p.inc suffsrc.inc >flic386p.lst
	pkzip -es -P %DRV%flic386p @flic386p.lst
	if ERRORLEVEL 1 goto error
	pkzip -es -P %DRV%resource resource\*.* resource\font\*.*
	if ERRORLEVEL 1 goto error
		rem - build group doesn't want exe's or it'll screw up error
		rem - detection when build doesn't work.
	if "%BLD%"=="TRUE" goto dobuild
	pkzip -es -P %DRV%exe pj.exp convert.exp aniplay.exp
	if ERRORLEVEL 1 goto error

:dobuild
	if "%BLD%"=="FALSE" goto doobj
	echo Backing up build and image directories.
	pkzip %DRV%build.bzp -es -p -r \build\*.*
	if ERRORLEVEL 1 goto error
	pkzip %DRV%images.izp -es -p -r \images\*.*
	if ERRORLEVEL 1 goto error

:doobj
	if "%OBJ%"=="FALSE" goto finishup
	echo Backing up object and lib files.
	textmul rootsrc.inc suffobj.inc >root.lst
	textmul incsrc.inc suffobj.inc >inc.lst
	textmul misc1src.inc suffobj.inc >misc1.lst
	textmul misc2src.inc suffobj.inc >misc2.lst
	textmul pocosrc.inc suffobj.inc >poco.lst
	textmul driv1src.inc suffobj.inc >driv1.lst
	textmul driv2src.inc suffobj.inc >driv2.lst
	copy root.lst+inc.lst+misc1.lst+misc2.lst+poco.lst+driv1.lst+driv2.lst obj.lst
	pkzip -es -P %DRV%obj @obj.lst
	if ERRORLEVEL 1 goto error
	textmul rootsrc.inc sufflib.inc >root.lst
	textmul incsrc.inc sufflib.inc >inc.lst
	textmul misc1src.inc sufflib.inc >misc1.lst
	textmul misc2src.inc sufflib.inc >misc2.lst
	textmul pocosrc.inc sufflib.inc >poco.lst
	textmul driv1src.inc sufflib.inc >driv1.lst
	textmul driv2src.inc sufflib.inc >driv2.lst
	copy root.lst+inc.lst+misc1.lst+misc2.lst+poco.lst+driv1.lst+driv2.lst lib.lst
	pkzip -es -P %DRV%lib @lib.lst
	if ERRORLEVEL 1 goto error

:finishup
	rem ** make sure we've got the unzipper when we need it
	copy c:\bin\pkunzip.exe %DRV%
	copy unzipall.bat %DRV%

rem tag the directory with a file containing the time and date of this arc
	showdate >%DRV%backdate.txt -p "Archive made " -d -t -s "."

:cdlast

rem for convienience build a file called cdlast.bat which is "cd the-dir"
	cd >foo %DRV%
	rep "%DRV%" "cd %%1" foo -b -s -c
	copy foo %DRV%\cdlast.bat

	echo Backup successfully zipped to %DRV%
	cd %DRV%
	goto quitit

:error
	echo BACKUP ABORTED!!!!
	beep 2
:quitit
	cd \paa
	set f=
	set obj=
	set bld=
	set drv=
	set nosrc=
	@echo on
