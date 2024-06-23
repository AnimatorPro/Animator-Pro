@echo off

mkdir _build
cmake -B _build -S . -G "Visual Studio 17 2022"
start _build/animator-pro.sln


