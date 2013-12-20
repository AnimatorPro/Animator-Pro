@echo off
cd ..\..\rexlib
call tcmake rex
cd ..\drivekit\lib

cd loader
tcmake
cd ..

copy ..\..\rexlib\aasyslib.lib ..\..\rexlib\stdiolib.lib loader\rexentry.obj .
