@echo off
setlocal enabledelayedexpansion

echo ==============================================================================
echo       LifeLink - Blood Donor Finder: Push to GitHub Helper
echo ==============================================================================
echo.

rem Determine git executable path
set "GIT_CMD=git"
where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    if exist "C:\Users\pc\mingit\cmd\git.exe" (
        set "GIT_CMD=C:\Users\pc\mingit\cmd\git.exe"
    ) else (
        echo [ERROR] Git is not installed or found on this machine!
        pause
        exit /b 1
    )
)

!GIT_CMD! branch -M main

rem Check if remote origin already exists
!GIT_CMD! remote get-url origin >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "delims=" %%u in ('!GIT_CMD! remote get-url origin') do set "EXISTING_URL=%%u"
    echo Current GitHub repository URL: !EXISTING_URL!
    echo.
    set /p UPDATE_CHOICE="Push latest updates to this repository? (Y/N, default Y): "
    if /i "!UPDATE_CHOICE!"=="" set "UPDATE_CHOICE=Y"
    if /i "!UPDATE_CHOICE!"=="Y" (
        echo.
        echo [INFO] Uploading latest updates to GitHub main branch...
        !GIT_CMD! push -u origin main
        goto :END
    )
)

echo 1. Open your browser and go to: https://github.com/new
echo 2. Type repository name: lifelink-blood-finder
echo 3. Keep it Public and click "Create repository"
echo 4. Copy the HTTPS link (e.g. https://github.com/YOUR_USERNAME/lifelink-blood-finder.git)
echo.
set /p REPO_URL="Paste your GitHub repository link here and press Enter: "

if "%REPO_URL%"=="" (
    echo [ERROR] No URL entered!
    pause
    exit /b 1
)

!GIT_CMD! remote remove origin 2>nul
!GIT_CMD! remote add origin %REPO_URL%

echo.
echo [INFO] Uploading complete LifeLink C++ project to GitHub...
!GIT_CMD! push -u origin main

:END
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==============================================================================
    echo [SUCCESS] Your project is now up to date on GitHub!
    echo You can now connect it to Render.com for 24/7 permanent deployment.
    echo ==============================================================================
) else (
    echo.
    echo [INFO] If GitHub prompted you to sign in, please complete the sign-in in your browser and try again.
)

pause
