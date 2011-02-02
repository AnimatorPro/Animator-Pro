pushdir
@if "%1"=="" goto USAGE
cd %1\
if ERRORLEVEL 1 goto USAGE 
md %1\bin
if ERRORLEVEL 1 goto USAGE 
copy *.bat %1\bin
pkunzip -d bin %1\bin
if ERRORLEVEL 1 goto ERROR 
md %1\v
if ERRORLEVEL 1 goto ERROR 
pkunzip -d v %1\v
if ERRORLEVEL 1 goto ERROR 
md %1\play
if ERRORLEVEL 1 goto ERROR 
pkunzip -d play %1\play
if ERRORLEVEL 1 goto ERROR 
md %1\crop
if ERRORLEVEL 1 goto ERROR 
pkunzip -d crop %1\crop
if ERRORLEVEL 1 goto ERROR 
md %1\tc
if ERRORLEVEL 1 goto ERROR 
pkunzip -d tc %1\tc
if ERRORLEVEL 1 goto ERROR 
@echo Unzipped all ok.
goto OUT
:USAGE
@echo UNZIP.BAT
@echo This batch file unzips all the source and tool files into the
@echo right places.  It should be run from the directory containing all the
@echo Animator build zips.  
@echo Usage: unzipall device:
@echo Example: unzipall d:
@goto OUT
:ERROR
@echo Couldn't UNZIP fully!
@beep 1
:OUT
@popdir
