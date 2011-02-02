@if "%1"=="" goto USAGE
call unzip %1
if ERRORLEVEL 1 goto ERROR 
call set_tc %1
cd %1\
%1
call make3
@echo All done.
@goto OUT
:USAGE
@echo This batch file unzips everything to a specified device and then
@echo compiles aa (in the v directory) convert (in crop) and aaplay (in play).
@echo Usage: doit device:
@echo Example: doit d:
@goto OUT
:ERROR
@echo Couldn't do it all!
:OUT
