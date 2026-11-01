@echo off
setlocal enabledelayedexpansion

echo ==============================================================================
echo       LifeLink – Blood Donor Finder (C++ Web Application)
echo       B.Tech C++ Internship Project
echo ==============================================================================

rem Add compiler to PATH if present
if exist "C:\Users\pc\w64devkit\bin\g++.exe" (
    set "PATH=C:\Users\pc\w64devkit\bin;%PATH%"
) else if exist "C:\w64devkit\bin\g++.exe" (
    set "PATH=C:\w64devkit\bin;%PATH%"
)

rem Check if backend binary exists, compile if needed
if not exist "backend\lifelink_server.exe" (
    echo [INFO] lifelink_server.exe not found. Building project now...
    cd backend
    call build.bat
    cd ..
    if not exist "backend\lifelink_server.exe" (
        echo [FATAL] Build failed! Check compiler output.
        pause
        exit /b 1
    )
)

echo [INFO] Starting LifeLink C++ Server on http://localhost:8080 ...
echo [INFO] The website will automatically open in your default browser.
echo [INFO] Press Ctrl+C in this terminal window to stop the server anytime.
echo.

rem Launch browser after a brief 1.5s delay
start /b "" cmd /c "timeout /t 2 /nobreak >nul && start http://localhost:8080"

rem Run the C++ server directly
backend\lifelink_server.exe

endlocal
