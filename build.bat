@echo off
echo Compiling...

g++ -std=c++17 -Wall -O2 BigInt.cpp Timer.cpp Testing.cpp exceptions.cpp main.cpp -o my_program.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
) else (
    echo Compilation succeeded.
    echo Running program.
    my_program.exe
)
pause