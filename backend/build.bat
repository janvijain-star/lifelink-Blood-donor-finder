@echo off
setlocal enabledelayedexpansion

echo =====================================================================
echo    LifeLink - Blood Donor Finder: C++ Backend Build Script
echo =====================================================================

rem Prioritize modern 64-bit MinGW/w64devkit toolchains if installed
if exist "C:\Users\pc\w64devkit\bin\g++.exe" (
    echo [INFO] Using GCC from C:\Users\pc\w64devkit\bin
    set "PATH=C:\Users\pc\w64devkit\bin;%PATH%"
) else if exist "C:\w64devkit\bin\g++.exe" (
    echo [INFO] Using GCC from C:\w64devkit\bin
    set "PATH=C:\w64devkit\bin;%PATH%"
)

where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] g++ compiler not found in PATH or standard directories!
    echo Please install GCC 11+ with C++17 support.
    exit /b 1
)

echo [1/3] Checking SQLite3 object file...
if not exist "sqlite3.o" (
    echo [INFO] Compiling sqlite3.c...
    gcc -O2 -c sqlite3.c -o sqlite3.o -Iinclude
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] Failed to compile sqlite3.c!
        exit /b 1
    )
) else (
    echo [INFO] sqlite3.o already exists, skipping.
)

echo [2/3] Compiling C++ Crow Server and Database Manager...
g++ -std=c++17 -O2 -Iinclude db_manager.cpp server.cpp sqlite3.o -o lifelink_server.exe -lws2_32 -lmswsock

if %ERRORLEVEL% EQU 0 (
    echo =====================================================================
    echo [SUCCESS] Backend built successfully!
    echo Binary created: backend\lifelink_server.exe
    echo Run "lifelink_server.exe" or use the root "run.bat" to launch the website.
    echo =====================================================================
) else (
    echo =====================================================================
    echo [ERROR] Compilation failed! Please inspect the error messages above.
    echo =====================================================================
    exit /b 1
)

endlocal
