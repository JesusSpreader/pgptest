@echo off
setlocal enabledelayedexpansion

echo ============================================
echo   PC Principal PGP - Quick Build Script
echo ============================================
echo.

:: Check for MSYS2 MINGW64
if not defined MINGW_PREFIX (
    echo ============================================
    echo   NOT RUNNING IN MSYS2!
    echo ============================================
    echo.
    echo Please use compile.bat instead - it will:
    echo 1. Find your MSYS2 installation automatically
    echo 2. Launch the build in the correct environment
    echo.
    echo Or manually:
    echo 1. Open 'MSYS2 MinGW 64-bit' terminal
    echo 2. Navigate to this directory
    echo 3. Run: ./build_internal.sh
    echo.
    pause
    exit /b 1
)

echo [MSYS2 Detected] Starting build...
echo.

:: Run the internal build script
bash "%~dp0build_internal.sh"
