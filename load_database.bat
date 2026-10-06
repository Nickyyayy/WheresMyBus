@echo off
setlocal

echo ========================================================
echo Where's My Bus - Database Setup Utility
echo ========================================================
echo Import the schema, stops, routes, ETAs,
echo and buses into a local MySQL database 'where_my_bus'.
echo.

set /p MYSQLUSER="Enter MySQL username (e.g. root): "
if "%MYSQLUSER%"=="" set MYSQLUSER=root

echo.
echo Step 1/2: Loading schema, stops, routes, and ETAs (sql\a.sql)...
mysql -u %MYSQLUSER% -p < sql\a.sql
if errorlevel 1 (
    echo.
    echo ERROR: Failed to load sql\a.sql.
    echo Please make sure:
    echo  1. MySQL Server is running.
    echo  2. The mysql command is in your PATH (or use MySQL Workbench).
    echo  3. The username and password you entered are correct.
    echo Alternatively, you can open and run sql\a.sql and sql\buses.sql
    echo directly in MySQL Workbench.
    pause
    exit /b 1
)

echo.
echo Step 2/2: Loading buses data (sql\buses.sql)...
mysql -u %MYSQLUSER% -p < sql\buses.sql
if errorlevel 1 (
    echo.
    echo ERROR: Failed to load sql\buses.sql.
    echo Alternatively, you can open and run sql\buses.sql
    echo directly in MySQL Workbench.
    pause
    exit /b 1
)

echo.
echo ========================================================
echo SUCCESS: Database 'where_my_bus' initialized successfully!
echo ========================================================
pause
