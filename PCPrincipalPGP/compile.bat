@echo off
setlocal enabledelayedexpansion

echo ============================================
echo   PC Principal PGP - Auto Build Script
echo ============================================
echo.

:: Check if already running in MSYS2 MINGW64
if defined MINGW_PREFIX (
    echo [MSYS2 Detected] Running build directly...
    goto :BUILD
)

:: ===================================================================
:: NOT IN MSYS2 - Find and launch MSYS2 MINGW64 automatically
:: ===================================================================
echo [Windows Mode] Searching for MSYS2 installation...

:: Common MSYS2 installation paths
set "MSYS2_PATHS=C:\msys64;C:\msys2;D:\msys64;D:\msys2;%ProgramFiles%\msys64;%ProgramFiles(x86)%\msys64;%LocalAppData%\msys64"

set "MSYS2_FOUND="
for %%P in (%MSYS2_PATHS%) do (
    if exist "%%P\mingw64.exe" (
        set "MSYS2_FOUND=%%P"
        echo Found MSYS2 at: %%P
        goto :FOUND_MSYS2
    )
    if exist "%%P\usr\bin\mintty.exe" (
        set "MSYS2_FOUND=%%P"
        echo Found MSYS2 at: %%P
        goto :FOUND_MSYS2
    )
)

:FOUND_MSYS2
if not defined MSYS2_FOUND (
    echo.
    echo ============================================
    echo   ERROR: MSYS2 NOT FOUND!
    echo ============================================
    echo.
    echo MSYS2 is required to build this application.
    echo.
    echo Please install MSYS2:
    echo 1. Download from: https://www.msys2.org/
    echo 2. Run the installer (msys2-x86_64-latest.exe)
    echo 3. Follow the installation instructions
    echo 4. Run this script again
    echo.
    echo After installing MSYS2, you may need to restart.
    echo.
    pause
    exit /b 1
)

echo.
echo Launching MSYS2 MINGW64 environment...
echo.

:: Get the current directory (where this script is)
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:\=/%"
set "SCRIPT_DIR=%SCRIPT_DIR::=%"
if not "%SCRIPT_DIR:~0,1%"=="/" set "SCRIPT_DIR=/%SCRIPT_DIR%"

:: Create a temporary bash script to run the build
set "TEMP_SCRIPT=%TEMP%\pcprincipal_build_%~n0.sh"
(
    echo #!/bin/bash
    echo cd "%SCRIPT_DIR%"
    echo export MINGW_INSTALLS=mingw64
    echo export PATH="/mingw64/bin:$PATH"
    echo export PKG_CONFIG_PATH="/mingw64/lib/pkgconfig:$PKG_CONFIG_PATH"
    echo.
    echo echo "========================================"
    echo echo "  Building in MSYS2 MINGW64..."
    echo echo "========================================"
    echo.
    echo # Run the actual build
    echo exec "./build_internal.sh"
) > "%TEMP_SCRIPT%"

:: Launch MSYS2 MINGW64 with our build script
if exist "%MSYS2_FOUND%\mingw64.exe" (
    start /wait "" "%MSYS2_FOUND%\mingw64.exe" "%TEMP_SCRIPT%"
) else if exist "%MSYS2_FOUND%\msys2.exe" (
    start /wait "" "%MSYS2_FOUND%\msys2.exe" -mingw64 -defterm -no-start -c "bash '%TEMP_SCRIPT%'"
) else if exist "%MSYS2_FOUND%\usr\bin\mintty.exe" (
    start /wait "" "%MSYS2_FOUND%\usr\bin\mintty.exe" -w full -e /bin/bash -lc "bash '%TEMP_SCRIPT%'"
) else (
    echo ERROR: Could not find MSYS2 launcher
    pause
    exit /b 1
)

:: Clean up
if exist "%TEMP_SCRIPT%" del "%TEMP_SCRIPT%"

:: Check if build succeeded
if exist "%~dp0PCPrincipalPGP_Portable\PCPrincipalPGP.exe" (
    echo.
    echo ============================================
    echo   BUILD SUCCESSFUL!
    echo ============================================
    echo.
    echo Your portable PGP app is ready at:
    echo   %~dp0PCPrincipalPGP_Portable\
    echo.
    explorer "%~dp0PCPrincipalPGP_Portable"
) else (
    echo.
    echo ============================================
    echo   BUILD MAY HAVE FAILED
    echo ============================================
    echo.
    echo Check the output above for errors.
)

echo.
pause
exit /b 0

:: ===================================================================
:: BUILD SECTION - Runs inside MSYS2
:: ===================================================================
:BUILD
echo.
echo ============================================
echo   Building PC Principal PGP
echo ============================================
echo.

:: Set up directories
set "SRC_DIR=%~dp0"
set "BUILD_DIR=%SRC_DIR%build"
set "DIST_DIR=%SRC_DIR%PCPrincipalPGP_Portable"

echo Source: %SRC_DIR%
echo Build:  %BUILD_DIR%
echo Output: %DIST_DIR%
echo.

:: Install dependencies
echo [1/6] Installing dependencies...
pacman -Sy --noconfirm 2>nul

:: Check and install required packages
pacman -Q mingw-w64-x86_64-qt6-base 2>nul || (
    echo Installing Qt6...
    pacman -S --noconfirm --needed mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-tools
)

pacman -Q mingw-w64-x86_64-gpgme 2>nul || (
    echo Installing GPGME...
    pacman -S --noconfirm --needed mingw-w64-x86_64-gpgme
)

pacman -Q mingw-w64-x86_64-libsodium 2>nul || (
    echo Installing libsodium...
    pacman -S --noconfirm --needed mingw-w64-x86_64-libsodium
)

pacman -Q mingw-w64-x86_64-cmake 2>nul || (
    echo Installing CMake...
    pacman -S --noconfirm --needed mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
)

pacman -Q mingw-w64-x86_64-gcc 2>nul || (
    echo Installing GCC...
    pacman -S --noconfirm --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config
)

echo Dependencies ready!
echo.

:: Clean previous build
echo [2/6] Cleaning previous build...
if exist "%BUILD_DIR%" (
    rm -rf "%BUILD_DIR%"
)
if exist "%DIST_DIR%" (
    rm -rf "%DIST_DIR%"
)
mkdir -p "%BUILD_DIR%"
mkdir -p "%DIST_DIR%"

:: Build
echo.
echo [3/6] Configuring with CMake...
cd "%BUILD_DIR%"

cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$MINGW_PREFIX" \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    "%SRC_DIR%"

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    pause
    exit /b 1
)

echo.
echo [4/6] Building... (this may take a few minutes)
cmake --build . --parallel

if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    echo.
    pause
    exit /b 1
)

echo.
echo [5/6] Copying executable...
cp -f "PCPrincipalPGP.exe" "%DIST_DIR%/"

:: Deploy Qt
echo.
echo [6/6] Deploying dependencies...
windeployqt "%DIST_DIR%/PCPrincipalPGP.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw 2>/dev/null || true

:: Copy required DLLs
echo Copying libraries...
cp -f "$MINGW_PREFIX/bin/libgpgme-11.dll" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/libassuan-0.dll" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/libgpg-error-0.dll" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/libsodium-23.dll" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/libgcrypt-20.dll" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/libntlm-0.dll" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/gpg.exe" "%DIST_DIR%/" 2>/dev/null || true
cp -f "$MINGW_PREFIX/bin/gpgconf.exe" "%DIST_DIR%/" 2>/dev/null || true

:: Copy icon
cp -f "%SRC_DIR%resources/icon.png" "%DIST_DIR%/" 2>/dev/null || true

:: Create directories
mkdir -p "%DIST_DIR%/keys/public"
mkdir -p "%DIST_DIR%/keys/private"
mkdir -p "%DIST_DIR%/profiles"
mkdir -p "%DIST_DIR%/gpg"
mkdir -p "%DIST_DIR%/temp"

cd "%SRC_DIR%"

echo.
echo ============================================
echo   BUILD SUCCESSFUL!
echo ============================================
echo.
echo Your portable PGP application is ready at:
echo   %DIST_DIR%
echo.
echo To use:
echo 1. Copy 'PCPrincipalPGP_Portable' to USB or keep it here
echo 2. Run PCPrincipalPGP.exe
echo 3. Follow the setup wizard
echo.
echo Features:
echo - Generate/import/export PGP keys
echo - Encrypt/decrypt messages and files
echo - Sign and verify signatures
echo - Manage contact profiles
echo - Post-quantum encryption support
echo - 100%% portable - no installation needed
echo.

:: Open the output folder
if exist "%DIST_DIR%" (
    explorer "%DIST_DIR%" 2>/dev/null || true
)

pause
