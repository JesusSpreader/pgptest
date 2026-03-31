@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   PC Principal PGP - Build Script
echo ========================================
echo.

:: Find MSYS2 installation
set "MSYS2_PATH="

:: Check common MSYS2 locations
if exist "C:\msys64\mingw64.exe" (
    set "MSYS2_PATH=C:\msys64"
) else if exist "D:\msys64\mingw64.exe" (
    set "MSYS2_PATH=D:\msys64"
) else if exist "E:\msys64\mingw64.exe" (
    set "MSYS2_PATH=E:\msys64"
) else if exist "%USERPROFILE%\msys64\mingw64.exe" (
    set "MSYS2_PATH=%USERPROFILE%\msys64"
) else (
    :: Try to find in PATH
    for %%i in (mingw32-make.exe) do (
        set "MAKE_PATH=%%~$PATH:i"
        if not "!MAKE_PATH!"=="" (
            for %%j in ("!MAKE_PATH!") do set "MSYS2_PATH=%%~dpj\..\.."
        )
    )
)

if "!MSYS2_PATH!"=="" (
    echo ERROR: MSYS2 not found!
    echo Please install MSYS2 from https://www.msys2.org/
    echo.
    pause
    exit /b 1
)

echo Found MSYS2 at: !MSYS2_PATH!
echo.

:: Check if we're already in MSYS2 environment
if defined MINGW_PREFIX (
    echo Already in MSYS2 environment, running build directly...
    bash build_internal.sh
    if !errorlevel! neq 0 (
        echo.
        echo Build failed!
        pause
        exit /b 1
    )
) else (
    echo Starting MSYS2 MinGW 64-bit environment...
    echo.
    
    :: Run the build script in MSYS2
    "!MSYS2_PATH!\msys2.exe" -mingw64 -defterm -no-start -c "cd '%CD%' && bash build_internal.sh"
    
    if !errorlevel! neq 0 (
        echo.
        echo Build failed!
        pause
        exit /b 1
    )
)

echo.
echo Build completed successfully!
echo.

:: Open output folder
if exist "PCPrincipalPGP_Portable" (
    echo Opening output folder...
    start explorer "PCPrincipalPGP_Portable"
)

echo.
pause
