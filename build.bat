@echo off
setlocal enabledelayedexpansion

echo Building Where's My Bus...

set FILES=
for %%f in (backend\*.cpp) do (
    set FILES=!FILES! %%f
)

g++ -std=c++17 -Wall -Wextra -Ibackend !FILES! -o wheresmybus.exe
if errorlevel 1 (
    echo.
    echo BUILD FAILED. Is g++ installed and in your PATH?
    pause
    exit /b 1
)

echo.
echo Build successful: wheresmybus.exe
pause
