@echo off
setlocal
echo ==============================================================================
echo       LifeLink - Blood Donor Finder: Push to GitHub Helper
echo ==============================================================================
echo.
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

"C:\Users\pc\mingit\cmd\git.exe" branch -M main
"C:\Users\pc\mingit\cmd\git.exe" remote remove origin 2>nul
"C:\Users\pc\mingit\cmd\git.exe" remote add origin %REPO_URL%

echo.
echo [INFO] Uploading complete LifeLink C++ project to GitHub...
"C:\Users\pc\mingit\cmd\git.exe" push -u origin main

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==============================================================================
    echo [SUCCESS] Your project is now on GitHub!
    echo You can now connect it to Render.com for 24/7 permanent deployment.
    echo ==============================================================================
) else (
    echo.
    echo [INFO] If GitHub prompted you to sign in, please complete the sign-in in your browser and try again.
)

pause
