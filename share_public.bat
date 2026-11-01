@echo off
echo ==============================================================================
echo       LifeLink - Blood Donor Finder: Instant Public Sharing
echo ==============================================================================
echo.
echo Make sure your LifeLink C++ Server is running on port 8080 first!
echo.
echo Connecting to free HTTPS tunnel via built-in Windows OpenSSH...
echo You will see a public URL below (e.g. https://...a.pinggy.link).
echo Share that link with your friends and faculty to access your site!
echo.
echo Press Ctrl+C anytime to stop sharing.
echo ==============================================================================
echo.

ssh -o StrictHostKeyChecking=no -p 443 -R0:localhost:8080 a.pinggy.io

pause
