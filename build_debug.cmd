@echo off

call "D:\vs2022version\VC\Auxiliary\Build\vcvarsall.bat" x64

set CC=cl
set CXX=cl
cmake ^
    -S . ^
    -B ./build ^
    -G Ninja ^
    -D CMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build ./build --config Debug

cmake --install ./build --config Debug
pause