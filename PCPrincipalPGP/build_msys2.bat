@echo off
:: Full-featured build script with options
:: This is the comprehensive version with all options

echo ============================================
echo   PC Principal PGP - Full Build Script
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
    pause
    exit /b 1
)

echo [MSYS2 Detected] Starting full build...
echo.

:: Set up paths
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "INSTALL_DIR=%PROJECT_DIR%dist"

echo Project: %PROJECT_DIR%
echo Build:   %BUILD_DIR%
echo Output:  %INSTALL_DIR%
echo.

:: Update package database
echo Updating package database...
pacman -Sy --noconfirm

:: Install all required packages
echo.
echo Installing required packages...
pacman -S --noconfirm --needed ^
    mingw-w64-x86_64-qt6-base ^
    mingw-w64-x86_64-qt6-tools ^
    mingw-w64-x86_64-gpgme ^
    mingw-w64-x86_64-libsodium ^
    mingw-w64-x86_64-cmake ^
    mingw-w64-x86_64-ninja ^
    mingw-w64-x86_64-gcc ^
    mingw-w64-x86_64-pkg-config

echo.
echo All packages installed!
echo.

:: Clean and create build directory
echo Preparing build directory...
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
)
mkdir "%BUILD_DIR%"

:: Configure with CMake
echo.
echo Configuring with CMake...
cd "%BUILD_DIR%"

cmake -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DCMAKE_PREFIX_PATH="%MINGW_PREFIX%" ^
    -DCMAKE_C_COMPILER=gcc ^
    -DCMAKE_CXX_COMPILER=g++ ^
    "%PROJECT_DIR%"

if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    cd "%PROJECT_DIR%"
    pause
    exit /b 1
)
echo CMake configuration successful!
echo.

:: Build
echo Building project...
cmake --build . --parallel

if errorlevel 1 (
    echo ERROR: Build failed!
    cd "%PROJECT_DIR%"
    pause
    exit /b 1
)
echo Build successful!
echo.

:: Install
echo Installing to distribution directory...
if exist "%INSTALL_DIR%" (
    rmdir /s /q "%INSTALL_DIR%"
)
mkdir "%INSTALL_DIR%"

cmake --install .

:: Copy executable
copy /y "%BUILD_DIR%\PCPrincipalPGP.exe" "%INSTALL_DIR%\" 2>nul

:: Deploy Qt dependencies
echo.
echo Deploying Qt dependencies...
windeployqt "%INSTALL_DIR%\PCPrincipalPGP.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw --no-compiler-runtime 2>nul

:: Copy additional dependencies
echo Copying additional libraries...
set "DLLS=libgpgme-11.dll libassuan-0.dll libgpg-error-0.dll libsodium-23.dll libgcrypt-20.dll libntlm-0.dll"

for %%D in (%DLLS%) do (
    if exist "%MINGW_PREFIX%\bin\%%D" (
        copy /y "%MINGW_PREFIX%\bin\%%D" "%INSTALL_DIR%\" >nul 2>&1
        echo   Copied: %%D
    )
)

:: Copy GPG binaries
if exist "%MINGW_PREFIX%\bin\gpg.exe" (
    copy /y "%MINGW_PREFIX%\bin\gpg.exe" "%INSTALL_DIR%\" >nul 2>&1
    copy /y "%MINGW_PREFIX%\bin\gpgconf.exe" "%INSTALL_DIR%\" >nul 2>&1
    echo   Copied: GPG binaries
)

:: Copy icon
copy /y "%PROJECT_DIR%resources\icon.png" "%INSTALL_DIR%\" >nul 2>&1

:: Create directories
mkdir "%INSTALL_DIR%\keys\public" 2>nul
mkdir "%INSTALL_DIR%\keys\private" 2>nul
mkdir "%INSTALL_DIR%\profiles" 2>nul
mkdir "%INSTALL_DIR%\gpg" 2>nul
mkdir "%INSTALL_DIR%\temp" 2>nul

cd "%PROJECT_DIR%"

echo.
echo ============================================
echo    BUILD COMPLETE!
echo ============================================
echo.
echo Your portable PGP application is ready at:
echo   %INSTALL_DIR%
echo.
pause
