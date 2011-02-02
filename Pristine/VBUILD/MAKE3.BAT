pushdir
cd v
if ERRORLEVEL 1 goto ERROR 
make
if ERRORLEVEL 1 goto ERROR 
cd ..\crop
if ERRORLEVEL 1 goto ERROR 
make
if ERRORLEVEL 1 goto ERROR 
cd ..\play
if ERRORLEVEL 1 goto ERROR 
make
if ERRORLEVEL 1 goto ERROR 
goto OUT
:ERROR
@echo Couldn't make all!
@beep 1
:OUT
popdir
