@echo off
setlocal

echo ==============================================================================
echo       LifeLink - Blood Donor Finder: Cloudflare HTTPS Public Tunnel
echo ==============================================================================
echo.

rem Check if C++ server is already running on port 8080
netstat -ano | findstr ":8080" >nul
if %ERRORLEVEL% NEQ 0 (
    echo [INFO] LifeLink server is not running. Starting C++ backend now...
    start /b "" backend\lifelink_server.exe
    timeout /t 2 /nobreak >nul
) else (
    echo [INFO] LifeLink C++ server is active on port 8080.
)

echo.
echo [INFO] Launching Cloudflare Tunnel...
echo Look for the link ending in .trycloudflare.com below:
echo (Example: https://xxxx-xxxx.trycloudflare.com)
echo.
echo Copy and share that link with your friends and college faculty!
echo Press Ctrl+C anytime to close the tunnel.
echo ==============================================================================
echo.

if exist "cloudflared.exe" (
    cloudflared.exe tunnel --url http://localhost:8080
) else if exist "C:\Users\pc\cloudflared.exe" (
    C:\Users\pc\cloudflared.exe tunnel --url http://localhost:8080
) else (
    echo [FALLBACK] Cloudflared not found, using OpenSSH tunnel:
    ssh -o StrictHostKeyChecking=no -p 443 -R0:localhost:8080 a.pinggy.io
)

pause
