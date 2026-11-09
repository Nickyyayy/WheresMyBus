@echo off
title Where's My Bus - Network Web Server
cls
echo Starting local web server...
python server.py
if errorlevel 1 (
    echo.
    echo Python was not found in PATH. Trying default Python 3.13 installation...
    "%LOCALAPPDATA%\Programs\Python\Python313\python.exe" server.py
)
pause
